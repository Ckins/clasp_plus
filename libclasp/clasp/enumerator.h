// 
// Copyright (c) 2006-2013, Benjamin Kaufmann
// 
// This file is part of Clasp. See http://www.cs.uni-potsdam.de/clasp/ 
// 
// Clasp is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// 
// Clasp is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with Clasp; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
//
#ifndef CLASP_ENUMERATOR_H_INCLUDED
#define CLASP_ENUMERATOR_H_INCLUDED

#ifdef _MSC_VER
#pragma once
#endif
#include <clasp/literal.h>
#include <clasp/constraint.h>
#include <clasp/minimize_constraint.h>
#include <clasp/util/misc_types.h>

namespace Clasp { 
class Solver;
class SharedContext;
class Enumerator;
class EnumerationConstraint;

//! Type for storing a model.
struct Model {
	enum Type { model_sat = 0, model_cons = 1, max_value = 1 };
	//! True if this model stores current (cautious/brave) consequences.
	bool     consequences()    const { return (type & model_cons) != 0; }
	//! For sat models, value of v in model. Otherwise, undefined.
	ValueRep value(Var v)      const { assert(values && v < values->size()); return (*values)[v]; }
	//! True if p is true in model or part of current consequences.
	bool     isTrue(Literal p) const { return (value(p.var()) & trueValue(p)) != 0; }
	uint64            num;    // running number of this model
	const Enumerator* ctx;    // ctx in which model was found
	const ValueVec*   values; // variable assignment or consequences
	const SumVec*     costs;  // associated costs (or 0)
	uint32            sId :16;// id of solver that found the model
	uint32            type:14;// type of model
	uint32            opt : 1;// whether the model is optimal w.r.t costs (0: unknown)
	uint32            sym : 1;// whether symmetric models are possible
};

/**
 * \defgroup enumerator Enumerators and related classes
 */
//@{

//! Options for configuring enumeration.
struct EnumOptions {  
	typedef MinimizeMode OptMode;
	enum EnumType { enum_auto = 0, enum_bt  = 1, enum_record  = 2, enum_dom_record = 3, enum_consequences = 4, enum_brave = 5, enum_cautious = 6, enum_user = 8 };
	EnumOptions() : numModels(-1), enumMode(enum_auto), optMode(MinimizeMode_t::optimize), project(0), maxSat(false) {}
	static Enumerator* createModelEnumerator(const EnumOptions& opts);
	static Enumerator* createConsEnumerator(const EnumOptions& opts);
	static Enumerator* nullEnumerator();
	static Enumerator* createEnumerator(const EnumOptions& opts);
	bool     consequences() const { return (enumMode & enum_consequences) != 0; }
	bool     models()       const { return (enumMode < enum_consequences); }
	bool     optimize()     const { return ((optMode  & MinimizeMode_t::optimize) != 0); }
	int      numModels; /*!< Number of models to compute. */
	EnumType enumMode;  /*!< Enumeration type to use.     */
	OptMode  optMode;   /*!< Optimization mode to use.    */
	uint32   project;   /*!< Options for projection.      */
	SumVec   optBound;  /*!< Initial bound for optimize statements. */
	bool     maxSat;    /*!< Treat DIMACS input as MaxSat */
};


//! Interface for supporting enumeration of models.
/*!
 * Enumerators are global w.r.t a solve algorithm. That is,
 * even if the solve algorithm itself uses a parallel search, there
 * shall be only one enumerator and it is the user's responsibility
 * to protect the enumerator with appropriate locking.
 *
 * Concrete enumerators must implement a function for preparing a problem for enumeration
 * and an EnumerationConstraint to be added to each solver attached to the problem. 
 * It is then the job of the actual solve algorithm to commit models to the enumerator 
 * and to integrate new information into attached solvers via appropriate calls to 
 * Enumerator::update().
 */
class Enumerator {
public:
	typedef EnumerationConstraint*       ConPtr;
	typedef const EnumerationConstraint* ConPtrConst;
	typedef const SharedMinimizeData*    Minimizer;
	typedef EnumOptions::OptMode         OptMode;
	class   ThreadQueue;
	explicit Enumerator();
	virtual ~Enumerator();

	void   reset();

	//! Prepares the problem for enumeration.
	/*!
	 * The function shall be called once before search is started and before SharedContext::endInit()
	 * was called. It freezes enumeration-related variables and adds a suitable enumeration constraint 
	 * to the master solver.
	 *
	 * \pre The problem is not yet frozen, i.e. SharedContext::endInit() was not yet called.
	 * \param problem The problem on which this enumerator should work.
	 * \param min     Optional minimization constraint to be applied during enumeration.
	 * \param limit   Optional hint on max number of models to compute.
	 *
	 * \note In the incremental setting, init() must be called once for each incremental step.
	 * \note The enumerator takes ownership of the minimize constraint (if any). That is,
	 *       it does not increase its reference count.
	 */
	int    init(SharedContext& problem, SharedMinimizeData* min = 0, int limit = 0);
	
	//! Prepares the given solver for enumeration under the given path.
	/*!
	 * The function shall be called once before solving is started. It pushes the
	 * given path to the solver by calling Solver::pushRoot() and prepares s for 
	 * enumeration/optimization.
	 * \return true if path was added. 
	 */
	bool   start(Solver& s, const LitVec& path = LitVec(), bool disjointPath = false)  const;
	
	//! Updates the given solver with enumeration-related information.
	/*!
	 * The function is used to integrate enumeration-related information,
	 * like minimization bounds or previously committed models, into the search space of s.
	 * It shall be called after each commit.
	 *
	 * \param s The solver to update.
	 * \note The function is concurrency-safe, i.e. can be called
	 *       concurrently by different solvers.
	 */
	bool   update(Solver& s)  const;
	
	/*!
	 * \name Commit functions
	 * Functions for committing enumeration-related information to the enumerator.
	 * \note The functions in this group are *not* concurrency-safe, i.e. in a parallel search
	 *       at most one solver shall call a commit function at any one time.
	 */
	//@{

	//! Commits the model stored in the given solver.
	/*!
	 * If the model is valid and unique, the function returns true and the 
	 * model can be accessed via a call to Enumerator::lastModel(). 
	 * Otherwise, the function returns false. 
	 * In either case, Enumerator::update(s) shall be called
	 * in order to continue search for further models. 
	 *
	 * \pre The solver's assignment is total.
	 */
	bool   commitModel(Solver& s);
	//! Expands the next symmetric model if any.
	bool   commitSymmetric(Solver& s);
	//! Commits an unsatisfiable path stored in the given solver.
	/*!
	 * The return value determines how search should proceed.
	 * If true is returned, the enumerator has relaxed an enumeration constraint
	 * and search may continue after a call to Enumerator::update(s).
	 * Otherwise, the search shall be stopped.
	 */
	bool   commitUnsat(Solver& s);
	//! Commits the given clause to this enumerator.
	bool   commitClause(const LitVec& clause) const;
	//! Marks current enumeration phase as completed.
	/*!
	 * If the enumerator was initialized with a minimization constraint and
	 * optimization mode MinimizeMode_t::enumOpt, the optimal bound is committed,
	 * the enumerator is prepared for enumerating optimal models, and the function
	 * returns false. Otherwise, the function returns true and search shall be stopped.
	 */
	bool   commitComplete();
	//! Commits the state stored in the given solver.
	/*!
	 * Calls commitModel() or commitUnsat() depending on the state of s.
	 * The function returns value_true, to signal that s stores a valid and
	 * unique model, value_false to signal that search shall be stopped, and
	 * value_free otherwise. 
	 * \see commitModel()
	 * \see commitUnsat()
	 */
	uint8  commit(Solver& s);
	//@}

	//! Removes from s the path that was passed to start() and any active (minimization) bound.
	void   end(Solver& s) const;
	//! Returns the number of models enumerated so far.
	uint64 enumerated()   const { return model_.num; }
	//! Returns the last model enumerated.
	/*!
	 * \note If enumerated() is equal to 0, the returned object is in an indeterminate state.
	 */
	const Model& lastModel()        const { return model_; }
	//! Returns whether optimization is active.
	bool         optimize()         const { return mini_ && mini_->mode() != MinimizeMode_t::enumerate && model_.opt == 0; }
	//! Returns whether computed models are still tentative.
	bool         tentative()        const { return mini_ && mini_->mode() == MinimizeMode_t::enumOpt && model_.opt == 0; }
	//! Returns the active minimization constraint if any.
	Minimizer    minimizer()        const { return mini_; }
	//! Returns the type of models this enumerator computes.
	virtual int  modelType()        const { return Model::model_sat; }
	//! Returns whether or not this enumerator supports full restarts once a model was found.
	virtual bool supportsRestarts() const { return true; }
	//! Returns whether or not this enumerator supports parallel search.
	virtual bool supportsParallel() const { return true; }
	//! Returns whether or not this enumerator supports splitting-based search.
	virtual bool supportsSplitting(const SharedContext& problem) const;
	//! Returns whether this enumerator requires exhaustive search to produce a definite answer.
	virtual bool exhaustive()       const { return mini_ && mini_->mode() != MinimizeMode_t::enumerate; }
	//! Sets whether the search path stored in s is disjoint from all others.
	void         setDisjoint(Solver& s, bool b) const;
	//! Sets whether symmetric should be ignored.
	void         setIgnoreSymmetric(bool b);
	ConPtr       constraint(const Solver& s) const;
protected:
	//! Shall prepare the enumerator and freeze any enumeration-related variable.
	/*!
	 * \return A prototypical enumeration constraint to be used in a solver.
	 */
	virtual ConPtr doInit(SharedContext& ctx, SharedMinimizeData* min, int numModels) = 0;
	virtual void   doReset();
private:
	class SharedQueue;
	Enumerator(const Enumerator&);
	Enumerator& operator=(const Enumerator&);
	SharedMinimizeData* mini_;
	SharedQueue*        queue_;
	SumVec              costs_;
	Model               model_;
};

//! A solver-local (i.e. thread-local) constraint to support enumeration.
/*!
 * An enumeration constraint is used to extract/store enumeration-related information
 * from models.
 */
class EnumerationConstraint : public Constraint {
public:
	typedef EnumerationConstraint*   ConPtr;
	typedef MinimizeConstraint*      MinPtr;
	typedef Enumerator::ThreadQueue* QueuePtr;
	//! Returns true if search-path is disjoint from all others.
	bool     disjointPath()const { return (flags_ & flag_path_disjoint) != 0u; }
	ValueRep state()       const { return static_cast<ValueRep>(flags_ & 3u); }
	//! Returns true if optimization is active.
	bool     optimize()    const;
	MinPtr   minimizer()   const { return mini_; }
	// Methods used by enumerator
	void init(Solver& s, SharedMinimizeData* min, QueuePtr q);
	bool start(Solver& s, const LitVec& path, bool disjoint);
	void end(Solver& s);
	bool update(Solver& s);
	void setDisjoint(bool x);
	bool integrateBound(Solver& s);
	bool integrateNogoods(Solver& s);
	bool commitModel(Enumerator& ctx, Solver& s);
	bool commitUnsat(Enumerator& ctx, Solver& s);
	void setMinimizer(MinPtr min) { mini_ = min; }
	void add(Constraint* c);
	void modelHeuristic(Solver& s);
protected:
	EnumerationConstraint();
	virtual ~EnumerationConstraint();
	// base interface
	virtual void        destroy(Solver* s, bool detach);
	virtual PropResult  propagate(Solver&, Literal, uint32&) { return PropResult(true, true); }
	virtual void        reason(Solver&, Literal, LitVec&)    {}
	virtual bool        simplify(Solver& s, bool reinit);
	virtual bool        valid(Solver& s);
	virtual Constraint* cloneAttach(Solver& s);
	// own interface
	virtual ConPtr      clone() = 0;
	virtual bool        doUpdate(Solver& s) = 0;
	virtual void        doCommitModel(Enumerator&, Solver&) {}
private:
	enum Flag { flag_path_disjoint = 4u, flag_model_heuristic = 8u };
	enum { clear_state_mask = ~uint32(value_true|value_false) };
	typedef PodVector<Constraint*>::type ConstraintDB;
	typedef SingleOwnerPtr<Enumerator::ThreadQueue> QPtr;
	MinimizeConstraint* mini_;
	QPtr                queue_;
	ConstraintDB        nogoods_;
	LitVec              next_;
	uint32              flags_ : 4;
	uint32              root_  : 28;
};
//@}
}

#endif

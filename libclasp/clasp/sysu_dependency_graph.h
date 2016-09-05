//
// Created by Junhong on 8/26/16.
//

#ifndef CLASP_SYSU_DEPENDENCY_GRAPH_H
#define CLASP_SYSU_DEPENDENCY_GRAPH_H

/*!
 * \file
 * namespace sysu
 * Contains the definition of the class program and related classes.
 *
 * tarjian algorithm
 *
 * call-consistent whole graph check algo
 *
 * T-algo and unfounded algo and w-expand are all implement internally
 */
#include <clasp/logic_program.h>
#include <set>
#include <iostream>

namespace Sysu {

    typedef Clasp::Var Var;
    typedef Clasp::Literal Literal;  // Literal.watched(): has been removed from graph
    typedef Clasp::VarVec VarVec;
    typedef Clasp::LitVec LitVec;
    typedef std::set<Var> VarSet;
    typedef std::set<Literal> LitSet;
    typedef VarSet SCC;
    typedef std::pair<VarSet, VarSet> VarSetPair;
    typedef Clasp::PodVector<SCC*>::type SCCVec;
    typedef std::pair<Var, Var> SimpleEdge;
    typedef std::pair<Literal, Literal> Edge;
    typedef std::pair<Literal, LitVec> MultiEdge;
    typedef Clasp::PodVector<MultiEdge>::type GraphType;
    enum EDGE_TYPE { NEG_EDGE, POS_EDGE };
    enum RULE_SATISFACTION { RULE_FAIL=-1, RULE_UNKNOWN=0, RULE_SATISFIED=1};
    typedef std::pair<SimpleEdge, EDGE_TYPE> SignedEdge;
    typedef std::map<SimpleEdge, EDGE_TYPE> DetailedGraphType;
    const Var FAILURE_MARK = 0;

    class Rule {
    public:
        LitVec heads;  // Vector<Var>
        LitVec body;  // Vector<Literal>
        VarSet vars;
        Rule(const Clasp::Asp::Rule& r);
        bool is_constraint();
    };
    typedef Clasp::PodVector<Rule>::type RuleVec;

    class DependencyGraph {
    public:
        const DetailedGraphType* signed_edges_ptr;

        // constructor parts
        ~DependencyGraph();
        void add_edge(const Rule& rule);
        void graph_reduce(const VarSet& P, const VarSet& N);
        void gl_reduce(const VarSet& P);
        void resume(GraphType& g);

        // construction Answer Set key algos
        VarSet gather_facts(const VarSet& P);
        VarSet T_once_plus(const VarSet& P, const VarSet& N);
        VarSet greatest_unfounded_set(const VarSet& P);
        VarSetPair W_once(const VarSet& P, const VarSet& N);
        VarSetPair W_inf(const VarSet& P, const VarSet& N);
        VarSetPair W_expand(const VarSet& P, const VarSet& N);

        // Update internal SCCs and Return a copy of it.
        void find_SCCs();

        // return true if every SCC in SCCs is call-consistent
        bool whole_call_consistent();

        // auxiliary methods
        void mark_failure(VarSet& s);
        void mark_failure(VarSetPair& s1_s2);
        bool failed(const VarSet& s);
        bool failed(const VarSetPair& s1_s2);

        void print_graph(const GraphType& g);
        void print_SCCs();

    private:
        GraphType graph_;
        GraphType graph_aux;
        SCCVec SCCs;
        VarSet vertices;
        unsigned long vertices_num;
        //targan
        void tarjan(const Literal& v);
        bool find_var(const LitVec& list, const Literal& item);
        int* DFN;
        int* LOW;
        int tarjan_index;
        LitVec tarjan_stack;
        // methods
        void copy_graph();
        bool same(const VarSet& Ax, const VarSet& B);
        bool same(const VarSetPair& A1_B1, const VarSetPair& A2_B2);
        bool has_outgoing_edge(SCC* scc);
        VarSetPair call_consistent(SCC* scc);
        void call_consistent_dfs(SCC* scc, const Var& v, VarSet& J, VarSet& K, int mark);
        void print_SCC(SCC* scc);
    };
}

#endif //CLASP_SYSU_DEPENDENCY_GRAPH_H

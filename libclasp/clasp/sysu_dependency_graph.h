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
    typedef LitSet SCC;
    typedef std::pair<VarSet, VarSet> VarSetPair;
    typedef Clasp::PodVector<SCC>::type SCCVec;
    enum EDGE_TYPE { NEG_EDGE, POS_EDGE };
    typedef std::pair<Var, Var> SimpleEdge;
    typedef std::pair<Literal, Literal> Edge;
    typedef std::pair<Literal, LitVec> MultiEdge;
    typedef Clasp::PodVector<MultiEdge>::type GraphType;
    typedef std::pair<SimpleEdge, EDGE_TYPE> SignedEdge;
    typedef std::map<SimpleEdge, EDGE_TYPE > DetailedGraphType;

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
        void add_edge(const Rule& rule);
        void reduce(const VarSet& P, const VarSet& N);
        void resume();

        // construction Answer Set key algos

        /*
         * Input : (P, N)
         * Ouput : (P', N')
         */
        VarSetPair T_once(const VarSet& P, const VarSet& N);
        VarSetPair T_inf(const VarSet& P, const VarSet& N);
        VarSetPair T_expand(const VarSet& P, const VarSet& N);

        // unknown todo
        void W_once();

        /*
         * Input : (P, N)
         * Output : (P*, N*)
         */
        void W_expand();

        // Update internal SCCs and Return a copy of it.
        void find_SCCs();

        // return true if every SCC in SCCs is call-consistent
        bool whole_call_consistent();

        // auxiliary functions
        void print_graph();
        void print_SCCs();
    private:
        GraphType graph_;
        SCCVec SCCs;
        VarSet vertices;
        int vertices_num;
        //targan
        void tarjan(const Literal& v);
        bool find_var(const LitVec& list, const Literal& item);
        int* DFN;
        int* LOW;
        int tarjan_index;
        LitVec tarjan_stack;
        // methods
        bool has_outgoing_edge(const SCC& scc);
        std::pair<bool, VarSetPair> call_consistent(const SCC& scc);
        void call_consistent_dfs(const SCC& scc, const Var& v, VarSet& J, VarSet& K, int mark);
        void print_SCC(const SCC& scc);
    };
}

#endif //CLASP_SYSU_DEPENDENCY_GRAPH_H

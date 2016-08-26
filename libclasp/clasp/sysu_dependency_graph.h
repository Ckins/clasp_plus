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
    typedef std::pair<LitSet, LitSet> LitSetPair;
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
        void reduce(const LitSet& P, const LitSet& N);
        void resume();

        // construction Answer Set key algos

        /*
         * Input : (P, N)
         * Ouput : (P', N')
         */
        void T_once();
        void T_inf();

        // unknown todo
        void W_once();

        /*
         * Input : (P, N)
         * Output : (P*, N*)
         */
        void W_expand();

        /*
         * Update internal SCCs and Return a copy of it.
         */
        SCCVec find_SCCs();

        /*
         * check whether we should continue
         * if the dg under (P, N) is not call-consistent,
         * (P, N) can NOT be expanded to an AS.
         */
        SCCVec check_SCCs();

        bool whole_call_consistent();

        // auxiliary functions
        void print_graph();
        void print_SCCs();
    private:
        GraphType graph_;
        SCCVec SCCs;
        VarSet vertices;
        //targan
        void tarjan(const Literal& v);
        int* DFN;
        int* LOW;
        int tarjan_index;
        LitVec tarjan_stack;
        // methods
        std::pair<bool, LitSetPair> call_consistent(const SCC& scc);
        void call_consistent_dfs(const SCC& scc, const Literal& v, LitSet& J, LitSet& K, int mark);
        void print_SCC(const SCC& scc);
    };
}

#endif //CLASP_SYSU_DEPENDENCY_GRAPH_H

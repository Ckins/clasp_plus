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
#include <stack>
#include <iostream>

namespace Sysu {

    typedef Clasp::Var Var;
    typedef Clasp::Literal Literal;
    typedef Clasp::VarVec VarVec;
    typedef Clasp::LitVec LitVec;
    typedef std::set<Var> VarSet;
    typedef std::set<Literal> LitSet;
    typedef LitSet SCC;
    typedef std::pair<Var, Var> Edge;
    typedef std::pair<Var, LitVec> MultiEdge;
    typedef Clasp::PodVector<MultiEdge>::type GraphType;
    enum EDGE_TYPE { NEG_EDGE=0, POS_EDGE };
    typedef std::pair<Edge, EDGE_TYPE> SignedEdge;
    typedef Clasp::PodVector<SignedEdge>::type DetailedGraphType;

    class Rule {
    public:
        VarVec heads;  // Vector<Var>
        LitVec body;  // Vector<Literal>
        VarSet atomVars;
        Rule(const Clasp::Asp::Rule& r);
        bool is_constraint();
    };
    typedef Clasp::PodVector<Rule>::type RuleVec;

    class DependencyGraph {
    public:

        // constructor parts
        DependencyGraph(const RuleVec& rules);
        void reduce (const LitSet& P, const LitSet& N);

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
         * check whether we should continue
         * if the dg under (P, N) is not call-consistent,
         * (P, N) can NOT be expanded to an AS.
         */
        Clasp::PodVector<SCC> checkSCC();
        std::pair<bool, std::pair<LitSet, LitSet> > call_consistent(SCC scc);
        bool whole_call_consistent();

        // auxiliary functions
        void print_all_edges();

    private:
        GraphType depGraph;
        Clasp::PodVector<SCC>::type SCCs;
        //targan
        void tarjan();
        bool *visited;
        bool *involved;
        int *DFN;
        int *LOW;
        int Index;
        std::stack<int> path;
        // methods
        void dfs(SCC scc, int v, LitSet J, LitSet K, int mark);
    };
}

#endif //CLASP_SYSU_DEPENDENCY_GRAPH_H

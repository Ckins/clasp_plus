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
    typedef std::set<Literal> AtomSet;
    typedef AtomSet SCC;
    typedef std::pair<Var, Var> Edge;
    typedef std::pair<Edge, bool> EdgeType;
    typedef std::pair<Var, AtomSet> MultiEdge;
    typedef std::map<Var, AtomSet> depGraphType;

    class Rule {
    public:
        Clasp::VarVec heads;  // Vector<Var>
        AtomSet body;  // Vector<Literal>
        Rule(const Clasp::Asp::Rule& r);
    };
    typedef Clasp::PodVector<Rule>::type RuleList;

    class DependencyGraph {
    public:

        // constructor parts
        DependencyGraph(const RuleList& rules);
        void reduce (const AtomSet& P, const AtomSet& N);

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
        std::pair<bool, std::pair<AtomSet, AtomSet> > call_consistent(SCC scc);
        bool whole_call_consistent();

        // auxiliary functions
        void print_all_edges();

    private:
        std::map<Var, AtomSet> depGraph;
        std::map<Edge, bool> edges;  // 0 - neg edge, 1 - pos edge
        Clasp::PodVector<SCC> SCCs;
        //targan
        void tarjan();
        bool *visited;
        bool *involved;
        int *DFN;
        int *LOW;
        int Index;
        std::stack<int> path;
        // methods
        void dfs(SCC scc, int v, AtomSet J, AtomSet K, int mark);
    };
}

#endif //CLASP_SYSU_DEPENDENCY_GRAPH_H

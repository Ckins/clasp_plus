//
// Created by kinsang on 16-8-25.
//

#ifndef CLASP_PROGRAM_H
#define CLASP_PROGRAM_H

/*!
 * \file
 * namespace sysu
 * Contains the definition of the class program and related classes.
 */
#include <clasp/logic_program.h>
#include <set>

namespace Sysu {
    class Rule {
    public:
        Clasp::VarVec heads;
        Clasp::WeightLitVec body;
    };

    // typedef Clasp::Asp::Rule Rule;
    typedef Clasp::Literal Literal;
    typedef Clasp::PodVector<Rule*>::type RuleList;
    typedef std::set<Literal> AtomSet;
    typedef AtomSet SCC;

    class DependencyGraph {
    private:
        std::map<Literal, AtomSet> dpg;
        std::vector<SCC> SCCs;
        void tarjan();
        void dfs(SCC scc, int v, AtomSet J, AtomSet K, int mark);
    public:
        DependencyGraph();
        DependencyGraph(const RuleList& rl);
        std::pair<AtomSet, AtomSet> call_consistent(SCC scc);
    };

    class Prg {
    public:
        Prg();
        DependencyGraph dependencyGraph;
        const Clasp::SymbolTable* symbolTablePtr;
        RuleList rules;
        AtomSet atomSet;

        void update(AtomSet P, AtomSet N);
        void print_all_rules();
    };
}

#endif //CLASP_PROGRAM_H

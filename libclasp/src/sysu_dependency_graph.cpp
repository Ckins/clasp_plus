//
// Created by Junhong on 8/26/16.
//
#include <clasp/sysu_dependency_graph.h>

namespace Sysu {

    Rule::Rule(const Clasp::Asp::Rule &r) {
        heads = r.heads;  // [Literal.var, Literal.var, ...]
        body = r.body;  // [<Literal, weight>, <Literal, weight>, ...]
    }

    DependencyGraph::DependencyGraph(const RuleList &rules) {
        for (RuleList::const_iterator r_it = rules.begin(); r_it != rules.end(); ++r_it) {
            for (Clasp::VarVec::const_iterator h_it = r_it->heads.begin(); h_it != r_it->heads.end(); ++h_it) {
                Var headVar = *h_it;
                AtomSet bodySet;
                for (Clasp::WeightLitVec::const_iterator b_it = r_it->body.begin(); b_it != r_it->body.end(); ++b_it) {
                    Literal bodyLiteral = b_it->first;
                    bodySet.insert(bodyLiteral);
                    if (bodyLiteral.sign()) {
                        edges.insert(EdgeType(Edge(headVar, bodyLiteral.var()), 0));
                    } else {
                        edges.insert(EdgeType(Edge(headVar, bodyLiteral.var()), 1));
                    }
                }
                depGraph.insert(MultiEdge(headVar, bodySet));
            }
        }
    }
    void DependencyGraph::print_all_edges() {
        for (depGraphType::const_iterator r_it = depGraph.begin(); r_it != depGraph.end(); ++r_it) {
            std::cout << "Rule: " << r_it->first << " :- ";
            int first_term = 1;
            for (AtomSet::const_iterator b_it = r_it->second.begin(); b_it != r_it->second.end(); ++b_it) {
                if (first_term) first_term = 0;
                else std::cout << ", ";
                if (b_it->sign()) std::cout << "not ";
                std::cout << (b_it->var());
            }
            std::cout << std::endl;
        }
    }
    void DependencyGraph::tarjan() {

    }
    void DependencyGraph::dfs(SCC scc, int v, AtomSet J, AtomSet K, int mark) {

    }
    std::pair<bool, std::pair<AtomSet, AtomSet> > DependencyGraph::call_consistent(SCC scc) {

    };
}
//
// Created by Junhong on 8/26/16.
//

#include <clasp/sysu_dependency_graph.h>

namespace Sysu {

    Rule::Rule(const Clasp::Asp::Rule &r) {
        heads = r.heads;  // [Literal.var, Literal.var, ...]
        for (VarVec::const_iterator it = r.heads.begin(); it != r.heads.end(); ++it) {
            atomVars.insert(*it);
        }
        for (Clasp::WeightLitVec::const_iterator it = r.body.begin(); it != r.body.end(); ++it) {
            atomVars.insert(it->first.var());
            body.push_back(it->first);  // [Literal, Literal, ...]
        }
    }
    bool Rule::is_constraint() {
        return heads.size() == 1 && heads[0] == 1;
    }

    DependencyGraph::DependencyGraph(const RuleVec &rules) {
        for (RuleVec::const_iterator r_it = rules.begin(); r_it != rules.end(); ++r_it) {
            for (VarVec::const_iterator h_it = r_it->heads.begin(); h_it != r_it->heads.end(); ++h_it) {
                Var headVar = *h_it;
                depGraph.push_back(MultiEdge(headVar, r_it->body));
            }
        }
    }
    void DependencyGraph::print_all_edges() {
        for (GraphType::const_iterator r_it = depGraph.begin(); r_it != depGraph.end(); ++r_it) {
            std::cout << r_it->first << " -> ";
            int first_term = 1;
            for (LitVec::const_iterator b_it = r_it->second.begin(); b_it != r_it->second.end(); ++b_it) {
                if (first_term) first_term = 0;
                else std::cout << ", ";
                if (b_it->sign()) std::cout << "[-]";
                else std::cout << "[+]";
                std::cout << (b_it->var());
            }
            std::cout << std::endl;
        }
    }
    void DependencyGraph::tarjan() {

    }
    void DependencyGraph::dfs(SCC scc, int v, LitSet J, LitSet K, int mark) {

    }
    std::pair<bool, std::pair<LitSet, LitSet> > DependencyGraph::call_consistent(SCC scc) {

    };

    bool DependencyGraph::whole_call_consistent() {

    }
}
//
// Created by Junhong on 8/26/16.
//

#include <clasp/sysu_dependency_graph.h>

namespace Sysu {

    Rule::Rule(const Clasp::Asp::Rule &r) {
        for (Clasp::VarVec::const_iterator it = r.heads.begin(); it != r.heads.end(); ++it) {
            vars.insert(*it);
            heads.push_back(Clasp::posLit(*it));
        }
        for (Clasp::WeightLitVec::const_iterator it = r.body.begin(); it != r.body.end(); ++it) {
            vars.insert(it->first.var());
            body.push_back(it->first);
        }
    }
    bool Rule::is_constraint() {
        return heads.size() == 1 && heads[0].var() == 1;
    }

    void DependencyGraph::add_edge(const Rule &rule) {
        for (Clasp::LitVec::const_iterator h_it = rule.heads.begin(); h_it != rule.heads.end(); ++h_it) {
            graph_.push_back(MultiEdge(*h_it, rule.body));
        }
        vertices.insert(rule.vars.begin(), rule.vars.end());
    }
    void DependencyGraph::resume() {
        for (GraphType::const_iterator r_it = graph_.begin(); r_it != graph_.end(); ++r_it) {
        }
    }
    SCCVec DependencyGraph::checkSCCs() {
    }
    SCCVec DependencyGraph::getSCCs() {
        SCCs.clear();
        Literal v, w;  // v -> w;
        for (GraphType::const_iterator edge_it = graph_.begin(); edge_it != graph_.end(); ++edge_it) {
            v = edge_it->first;
            if (!v.watched() && DFN.find(v) == DFN.end()) {  // still in graph
                tarjan(v);
            }
        }
    }
    void DependencyGraph::tarjan(const Literal& v) {

    }
    void DependencyGraph::dfs(SCC scc, int v, LitSet J, LitSet K, int mark) {

    }
    std::pair<bool, std::pair<LitSet, LitSet> > DependencyGraph::call_consistent(SCC scc) {

    };

    bool DependencyGraph::whole_call_consistent() {

    }
    void DependencyGraph::print() {
        std::cout << "---Dependency Graph---" << std::endl;
        for (GraphType::const_iterator r_it = graph_.begin(); r_it != graph_.end(); ++r_it) {
            std::cout << r_it->first.var() << " -> ";
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
        std::cout << "---End---" << std::endl;
    }
}
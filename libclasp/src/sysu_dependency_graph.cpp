//
// Created by Junhong on 8/26/16.
//

#include <clasp/sysu_dependency_graph.h>

namespace Sysu {

    int min(int a, int b) {
        return a <= b ? a : b;
    }
    bool find_var(const LitVec& list, const Literal& item) {
        for (LitVec::const_iterator it = list.begin(); it != list.end(); ++it)
            if (it->var() == item.var())
                return true;
        return false;
    }

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
    SCCVec DependencyGraph::check_SCCs() {
    }
    SCCVec DependencyGraph::find_SCCs() {
        SCCs.clear();
        tarjan_index = 0;
        tarjan_stack.clear();
        vertices_num = vertices.size() * 2;
        DFN = new int[vertices_num]();
        LOW = new int[vertices_num]();
        Literal v;
        // start
        for (GraphType::const_iterator edge_it = graph_.begin(); edge_it != graph_.end(); ++edge_it) {
            v = edge_it->first;
            if (!v.watched() && DFN[v.var()] <= 0) {  // w is still in graph & not visited by tarjan
                tarjan(v);
            }
        }
        // end
        delete[] DFN;
        delete[] LOW;
        return SCCs;
    }
    void DependencyGraph::tarjan(const Literal& v) {
        DFN[v.var()] = LOW[v.var()] = ++tarjan_index;  // initial visiting mark
        tarjan_stack.push_back(v);
        Literal w;  // v -> w
        for (GraphType::const_iterator edge_it = graph_.begin(); edge_it != graph_.end(); ++edge_it) {
            if (edge_it->first.var() == v.var()) {
                for (LitVec::const_iterator w_it = edge_it->second.begin(); w_it != edge_it->second.end(); ++w_it) {  // v -> w
                    w = *w_it;
                    if (!w.watched()) {  // w is still in graph
                        if (DFN[w.var()] <= 0) {  // not visited by tarjan
                            tarjan(w);
                            LOW[v.var()] = min(LOW[v.var()], LOW[w.var()]);
                        } else if (find_var(tarjan_stack, w)) {  // in stack
                            LOW[v.var()] = min(LOW[v.var()], DFN[w.var()]);
                        }
                    }
                }
            }
        }
        if (LOW[v.var()] == DFN[v.var()]) {
            SCC scc;
            do {
                w = tarjan_stack.back();
                scc.insert(w);
                tarjan_stack.pop_back();
            } while (w.var() != v.var());
            SCCs.push_back(scc);
        }
    }
    void DependencyGraph::dfs(SCC scc, int v, LitSet J, LitSet K, int mark) {

    }
    std::pair<bool, std::pair<LitSet, LitSet> > DependencyGraph::call_consistent(SCC scc) {

    };

    bool DependencyGraph::whole_call_consistent() {

    }
    void DependencyGraph::print_SCC(const SCC& scc) {
        std::cout << "Strong Connected Component: ";
        for (LitSet::const_iterator it = scc.begin(); it != scc.end(); ++it) {
            std::cout << it->var() << " ";
        }
        std::cout << std::endl;
    }
    void DependencyGraph::print_SCCs() {
        find_SCCs();
        for (SCCVec::const_iterator it = SCCs.begin(); it != SCCs.end(); ++it) {
            print_SCC(*it);
        }
    }
    void DependencyGraph::print_graph() {
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
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
    void DependencyGraph::reduce(const LitSet& P, const LitSet& N) {
        // TODO: 16-08-27
        for (GraphType::iterator edge_it = graph_.begin(); edge_it != graph_.end(); ++edge_it) {
            if (N.find(edge_it->first) != N.end()) {  // delete head[edge_it->first]
                edge_it->first.watch();
            } else {
                for (LitVec::iterator w_it = edge_it->second.begin(); w_it != edge_it->second.end(); ++w_it) {

                }
            }
        }
    }
    void DependencyGraph::resume() {
        for (GraphType::iterator edge_it = graph_.begin(); edge_it != graph_.end(); ++edge_it) {
            edge_it->first.clearWatch();  // clear head
            for (LitVec::iterator w_it = edge_it->second.begin(); w_it != edge_it->second.end(); ++w_it) {
                w_it->clearWatch();
            }
        }
    }
    SCCVec DependencyGraph::check_SCCs() {
    }
    SCCVec DependencyGraph::find_SCCs() {
        SCCs.clear();
        tarjan_index = 0;
        tarjan_stack.clear();
        DFN = new int[vertices.size()+10]();
        LOW = new int[vertices.size()+10]();
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
    std::pair<bool, LitSetPair> DependencyGraph::call_consistent(const SCC& scc) {
        LitSet J, K;
        Literal v = *scc.begin();
        DetailedGraphType::const_iterator it;
        call_consistent_dfs(scc, v, J, K, false);
        std::cout << "---call_consistent Call---" << std::endl;
        print_SCC(scc);
        std::cout << "In J: ";
        for (LitSet::const_iterator it = J.begin(); it != J.end(); ++it) {
            std::cout << it->var() << " ";
        }
        std::cout << std::endl << "In K: ";
        for (LitSet::const_iterator it = K.begin(); it != K.end(); ++it) {
            std::cout << it->var() << " ";
        }
        std::cout << std::endl;
        std::cout << "---call_consistent End---" << std::endl;
        bool p_in_J, q_in_J, pq_in_J, pq_in_K;  // !p_in_J implies p_in_K
        for (LitSet::const_iterator p_it = scc.begin(); p_it != scc.end(); ++p_it) {
            for (LitSet::const_iterator q_it = scc.begin(); q_it != scc.end(); ++q_it) { // p->q
                it = signed_edges_ptr->find(SimpleEdge(p_it->var(), q_it->var()));
                if (it != signed_edges_ptr->end()) {  // find p->q in graph
                    std::cout << p_it->var() << " -> " << q_it->var() << ": " << (it->second == POS_EDGE ? "POS" : "NEG");
                    p_in_J = J.find(*p_it) != J.end();
                    q_in_J = J.find(*q_it) != J.end();
                    pq_in_J = p_in_J && q_in_J;
                    pq_in_K = !p_in_J && !q_in_J;
                    std::cout << ", " << p_it->var() << " in " << (p_in_J ? "J" : "K") << ", " << q_it->var() << " in " << (q_in_J ? "J" : "K") << std::endl;
                    if ((it->second == POS_EDGE && (!pq_in_J && !pq_in_K))
                        || (it->second == NEG_EDGE && (pq_in_J || pq_in_K))) {
                        std::cout << "Call Consistent Fail: " << p_it->var() << " -> " << q_it->var() << std::endl;
                        return std::pair<bool, LitSetPair>(false, LitSetPair(J, K));
                    }
                }
            }
        }
        std::cout << "TRUE" << std::endl;
        return std::pair<bool, LitSetPair>(true, LitSetPair(J, K));
    };
    void DependencyGraph::call_consistent_dfs(const SCC& scc, const Literal& v, LitSet& J, LitSet& K, int mark) {
        if (mark) {
            J.insert(v);
        } else {
            K.insert(v);
        }
        DetailedGraphType::const_iterator it;
        for (LitSet::const_iterator w_it = scc.begin(); w_it != scc.end(); ++w_it) { // v -> w
            it = signed_edges_ptr->find(SimpleEdge(v.var(), w_it->var()));
            if (it != signed_edges_ptr->end() && J.find(*w_it) == J.end() && K.find(*w_it) == K.end()) {
                call_consistent_dfs(scc, *w_it, J, K, it->second == POS_EDGE ? mark : !mark);
            }
        }
    }
    bool DependencyGraph::whole_call_consistent() {
        find_SCCs();
        std::cout << "Size: " << SCCs.size() << std::endl;
        for (SCCVec::const_iterator scc_it = SCCs.begin(); scc_it != SCCs.end(); ++scc_it) {
            if (!call_consistent(*scc_it).first) {
                return false;
            }
        }
        return true;
    }
    void DependencyGraph::print_SCC(const SCC& scc) {
        std::cout << "Strongly Connected Component: ";
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
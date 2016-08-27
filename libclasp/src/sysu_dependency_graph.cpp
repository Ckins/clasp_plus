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
        vertices_num += 1;
    }
    void DependencyGraph::resume() {
        for (GraphType::iterator edge_it = graph_.begin(); edge_it != graph_.end(); ++edge_it) {
            edge_it->first.clearWatch();  // clear head
            for (LitVec::iterator w_it = edge_it->second.begin(); w_it != edge_it->second.end(); ++w_it) {
                w_it->clearWatch();
            }
        }
    }
    void DependencyGraph::reduce(const VarSet& P, const VarSet& N) {
        // use Log(n) for assignment check
        std::map<Var, bool> assign;
        std::map<Var, bool>::const_iterator a_it;
        for(VarSet::const_iterator it = P.begin(); it != P.end(); ++it) {
            assign[*it] = true;
        }
        for(VarSet::const_iterator it = N.begin(); it != N.end(); ++it) {
            assign[*it] = false;
        }
        // reduce
        for (GraphType::iterator edge_it = graph_.begin(); edge_it != graph_.end(); ++edge_it) {  // v: edge_it->first
            // check rule body
            for (LitVec::iterator w_it = edge_it->second.begin(); w_it != edge_it->second.end(); ++w_it) {  // *w: w_it
                a_it = assign.find(w_it->var());
                if (a_it != assign.end()) {  // assigned
                    if (a_it->second) {  // w is true, Literal(w) uncertain
                        if (w_it->sign()) {  // (not w) - Literal is false
                            edge_it->first.watch();  // remove edge by removing head
                            break;  // no need to check rest
                        } else {  // w - Literal is true
                            w_it->watch();  // remove body literal
                        }
                    } else {
                        if (w_it->sign()) {  // (not w) - Literal is true
                            w_it->watch();  // remove body literal
                        } else {  // w - Literal is false
                            edge_it->first.watch();  // remove edge by removing head
                            break;  // no need to check rest
                        }
                    }
                }
            }
        }
        // find SCCs in reduced graph;
        find_SCCs();
    }
    VarSetPair DependencyGraph::T_once(const VarSet& P, const VarSet& N) {
        // use Log(n) for assignment check
        std::map<Var, bool> assign;
        std::map<Var, bool>::const_iterator a_it;
        for(VarSet::const_iterator it = P.begin(); it != P.end(); ++it) {
            assign[*it] = true;
        }
        for(VarSet::const_iterator it = N.begin(); it != N.end(); ++it) {
            assign[*it] = false;
        }
        // check rule statisfaction
        VarSet P1, N1;
        int rule_satisfication;  // -1 - rule is false, 0 - rule unknown, 1 - rule is true
        for (GraphType::iterator edge_it = graph_.begin(); edge_it != graph_.end(); ++edge_it) {  // v: edge_it->first
            rule_satisfication = 1;
            // check rule body
            for (LitVec::iterator w_it = edge_it->second.begin(); w_it != edge_it->second.end(); ++w_it) {  // w: *w_it
                a_it = assign.find(w_it->var());
                if (a_it != assign.end()) {  // assigned
                    if (a_it->second) {  // w is true, Literal(w) uncertain
                        if (w_it->sign()) {  // (not w) - Literal is false
                            rule_satisfication = -1;
                            break;
                        } else {  // w - Literal is true
                            // check next w
                        }
                    } else {
                        if (w_it->sign()) {  // (not w) - Literal is true
                            // check next w
                        } else {  // w - Literal is false
                            rule_satisfication = -1;
                            break;
                        }
                    }
                } else {
                    rule_satisfication = 0;
                }
            }
            if (rule_satisfication == 1) { P1.insert(edge_it->first.var()); }
            else if (rule_satisfication == -1) { N1.insert(edge_it->first.var()); }
        }
        return VarSetPair(P1, N1);
    }
    VarSetPair DependencyGraph::T_inf(const VarSet& P, const VarSet& N) {
        int iterations = vertices_num;  // vertices.size() - P.size() - N.size();
        VarSetPair P_N = VarSetPair(P, N), P1_N1;
        bool reach_fixpoint;
        VarSet::const_iterator it, it1;
        while (iterations--) {
            P1_N1 = T_once(P_N.first, P_N.second);
            reach_fixpoint = true;
            if (P_N.first.size() == P1_N1.first.size() && P_N.second.size() == P1_N1.second.size()) {
                for (it = P_N.first.begin(), it1 = P1_N1.first.begin(); it != P_N.first.end() && it1 != P1_N1.first.end(); ++it, ++it1) {
                    if (*it != *it1) {
                        reach_fixpoint = false;
                    }
                }
                for (it = P_N.second.begin(), it1 = P1_N1.second.begin(); it != P_N.second.end() && it1 != P1_N1.second.end(); ++it, ++it1) {
                    if (*it != *it1) {
                        reach_fixpoint = false;
                    }
                }
            } else {
                reach_fixpoint = false;
            }
            if (reach_fixpoint) {
                return P1_N1;
            } else {
                P_N = P1_N1;
            }
        }
        // TODO(2016-08-27): if reached maximum iteration, return last P1, N1?
        return P1_N1;
    }
    VarSetPair DependencyGraph::T_expand(const VarSet& P, const VarSet& N) {
        VarSetPair P_N_star = T_inf(P, N);
        std::pair<bool, VarSetPair> call_consistent_result;
        VarSetPair* J_K_ptr;  // avoid copy ^
        // start T-expand
        reduce(P_N_star.first, P_N_star.second);
        bool has_possible_consistent_SCC = true;
        while (has_possible_consistent_SCC) {
            has_possible_consistent_SCC = false;
            for (SCCVec::const_iterator scc_it = SCCs.begin(); scc_it != SCCs.end(); ++scc_it) {
                if (!has_outgoing_edge(*scc_it)) {  // with no outgoing edge
                    call_consistent_result = call_consistent(*scc_it);
                    if (call_consistent_result.first) {
                        J_K_ptr = &call_consistent_result.second;
                        P_N_star.first.insert(J_K_ptr->first.begin(), J_K_ptr->first.end());  // P + J
                        P_N_star.second.insert(J_K_ptr->second.begin(), J_K_ptr->second.end());  // N + K
                        P_N_star = T_inf(P_N_star.first, P_N_star.second);
                        reduce(P_N_star.first, P_N_star.second);
                        has_possible_consistent_SCC = true;
                    }
                }
            }
        }
        return P_N_star;
    }
    bool DependencyGraph::has_outgoing_edge(const SCC &scc) {
        for (SCC::const_iterator v_it = scc.begin(); v_it != scc.end(); ++v_it) {
            for (GraphType::iterator edge_it = graph_.begin(); edge_it != graph_.end(); ++edge_it) {  // v: edge_it->first
                if (v_it->var() == edge_it->first.var()  && !edge_it->first.watched()) {
                    return true;
                }
            }
        }
        return false;
    }
    void DependencyGraph::find_SCCs() {
        SCCs.clear();
        tarjan_index = 0;
        tarjan_stack.clear();
        DFN = new int[vertices_num+10]();
        LOW = new int[vertices_num+10]();
        Literal v;
        // start tarjan
        for (GraphType::const_iterator edge_it = graph_.begin(); edge_it != graph_.end(); ++edge_it) {
            v = edge_it->first;
            if (!v.watched() && DFN[v.var()] <= 0) {  // w is still in graph & not visited by tarjan
                tarjan(v);
            }
        }
        // end tarjan
        delete[] DFN;
        delete[] LOW;
    }
    bool DependencyGraph::find_var(const LitVec& list, const Literal& item) {
        for (LitVec::const_iterator it = list.begin(); it != list.end(); ++it)
            if (it->var() == item.var())
                return true;
        return false;
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
                            LOW[v.var()] = LOW[w.var()] < LOW[v.var()] ? LOW[w.var()] : LOW[v.var()];
                        } else if (find_var(tarjan_stack, w)) {  // in stack
                            LOW[v.var()] = DFN[w.var()] < LOW[v.var()] ? DFN[w.var()] : LOW[v.var()];
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
    std::pair<bool, VarSetPair> DependencyGraph::call_consistent(const SCC& scc) {
        VarSet J, K;
        Var v = scc.begin()->var();
        call_consistent_dfs(scc, v, J, K, false);
        std::cout << "---call_consistent Start---" << std::endl;
        print_SCC(scc);
        std::cout << "In J: ";
        for (VarSet::const_iterator it = J.begin(); it != J.end(); ++it) {
            std::cout << *it << " ";
        }
        std::cout << std::endl << "In K: ";
        for (VarSet::const_iterator it = K.begin(); it != K.end(); ++it) {
            std::cout << *it << " ";
        }
        std::cout << std::endl;
        DetailedGraphType::const_iterator signed_edge_it;
        bool p_in_J, q_in_J, pq_in_J, pq_in_K;  // !p_in_J implies p_in_K
        for (LitSet::const_iterator p_it = scc.begin(); p_it != scc.end(); ++p_it) {
            for (LitSet::const_iterator q_it = scc.begin(); q_it != scc.end(); ++q_it) { // p->q
                signed_edge_it = signed_edges_ptr->find(SimpleEdge(p_it->var(), q_it->var()));
                if (signed_edge_it != signed_edges_ptr->end()) {  // find p->q in graph
                    std::cout << p_it->var() << " -> " << q_it->var() << ": " << (signed_edge_it->second == POS_EDGE ? "POS" : "NEG");
                    p_in_J = J.find(p_it->var()) != J.end();
                    q_in_J = J.find(q_it->var()) != J.end();
                    pq_in_J = p_in_J && q_in_J;
                    pq_in_K = !p_in_J && !q_in_J;
                    std::cout << ", " << p_it->var() << " in " << (p_in_J ? "J" : "K") << ", " << q_it->var() << " in " << (q_in_J ? "J" : "K") << std::endl;
                    if ((signed_edge_it->second == POS_EDGE && (!pq_in_J && !pq_in_K))
                        || (signed_edge_it->second == NEG_EDGE && (pq_in_J || pq_in_K))) {
                        std::cout << "Call Consistent Fail: " << p_it->var() << " -> " << q_it->var() << std::endl;
                        return std::pair<bool, VarSetPair>(false, VarSetPair(J, K));
                    }
                }
            }
        }
        return std::pair<bool, VarSetPair>(true, VarSetPair(J, K));
    };
    void DependencyGraph::call_consistent_dfs(const SCC& scc, const Var& v, VarSet& J, VarSet& K, int mark) {
        if (mark) {
            J.insert(v);
        } else {
            K.insert(v);
        }
        DetailedGraphType::const_iterator it;
        for (LitSet::const_iterator w_it = scc.begin(); w_it != scc.end(); ++w_it) {  // v -> w
            it = signed_edges_ptr->find(SimpleEdge(v, w_it->var()));
            if (it != signed_edges_ptr->end() && J.find(w_it->var()) == J.end() && K.find(w_it->var()) == K.end()) {
                call_consistent_dfs(scc, w_it->var(), J, K, it->second == POS_EDGE ? mark : !mark);
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
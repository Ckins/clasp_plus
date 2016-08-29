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
        vertices_num = vertices.size();
    }
    void DependencyGraph::resume() {
        for (GraphType::iterator edge_it = graph_.begin(); edge_it != graph_.end(); ++edge_it) {
            edge_it->first.clearWatch();  // clear head
            for (LitVec::iterator w_it = edge_it->second.begin(); w_it != edge_it->second.end(); ++w_it) {
                w_it->clearWatch();  // clear body
            }
        }
    }
    void DependencyGraph::graph_reduce(const VarSet& P, const VarSet& N) {
        resume();
        // use Log(n) for assignment check
        std::map<Var, bool> assign;
        std::map<Var, bool>::const_iterator a_it;
        for(VarSet::const_iterator it = P.begin(); it != P.end(); ++it) {
            assign[*it] = true;
        }
        for(VarSet::const_iterator it = N.begin(); it != N.end(); ++it) {
            assign[*it] = false;
        }
        // graph reduce
        for (GraphType::iterator edge_it = graph_.begin(); edge_it != graph_.end(); ++edge_it) {  // head: edge_it->first
            for (LitVec::iterator body_it = edge_it->second.begin(); body_it != edge_it->second.end(); ++body_it) {  // body: *w_it
                a_it = assign.find(body_it->var());
                if (a_it != assign.end()) {             // body literal is assigned
                    if (a_it->second) {                 // w is true, Literal(w) uncertain
                        if (body_it->sign()) {          // negLit, Literal is false
                            edge_it->first.watch();     // remove edge by removing head
                            break;                      // no need to check rest
                        } else {                        // posLit, Literal is true
                            body_it->watch();           // remove body literal
                        }
                    } else {                            // w is false, Literal(w) uncertain
                        if (body_it->sign()) {          // negLit, Literal is true
                            body_it->watch();           // remove body literal
                        } else {                        // posLit, Literal is false
                            edge_it->first.watch();     // remove edge by removing head
                            break;                      // no need to check rest
                        }
                    }
                }
            }
        }
        // find SCCs in reduced graph;
        std::cout << "---Graph Reduced---" << std::endl;
        find_SCCs();
        print_graph();
        print_SCCs();
    }
    void DependencyGraph::gl_reduce(const VarSet &P) {
        resume();
        // use Log(n) for assignment check
        VarSet::const_iterator a_it;
        // GL-reduce
        for (GraphType::iterator edge_it = graph_.begin(); edge_it != graph_.end(); ++edge_it) {  // head: edge_it->first
            for (LitVec::iterator body_it = edge_it->second.begin(); body_it != edge_it->second.end(); ++body_it) {  // body: *w_it
                if (body_it->sign()) {                  // negLit
                    a_it = P.find(body_it->var());
                    if (a_it != P.end()) {              // assigned true, negLit is false
                        edge_it->first.watch();         // remove edge by removing head
                        break;                          // no need to check rest
                    } else {                            // assigned false or unknown, negLit is true or unknown
                        body_it->watch();               // remove this negLit
                    }
                }
            }
        }
    }
    VarSet DependencyGraph::T_once_plus(const VarSet& P, const VarSet& N) {
        // use Log(n) for assignment check
        std::map<Var, bool> assign;
        std::map<Var, bool>::const_iterator a_it;
        for(VarSet::const_iterator it = P.begin(); it != P.end(); ++it) {
            assign[*it] = true;
        }
        for(VarSet::const_iterator it = N.begin(); it != N.end(); ++it) {
            assign[*it] = false;
        }
        // check rule satisfaction
        VarSet T_plus;
        RULE_SATISFACTION rule_judgement;
        for (GraphType::iterator edge_it = graph_.begin(); edge_it != graph_.end(); ++edge_it) {  // head(v): edge_it->first
            if (!edge_it->first.watched()) {                        // rule is still in graph
                rule_judgement = RULE_SATISFIED;
                for (LitVec::iterator w_it = edge_it->second.begin(); w_it != edge_it->second.end(); ++w_it) {  // body(w): *w_it
                    if (!w_it->watched()) {                         // body literal is still in graph
                        a_it = assign.find(w_it->var());
                        if (a_it != assign.end()) {                 // body literal is assigned
                            if (a_it->second) {                     // w is true, Literal(w) uncertain
                                if (w_it->sign()) {                 // negLit, Literal is false
                                    rule_judgement = RULE_FAIL;     // rule fail
                                    break;                          // no need to check rest
                                }
                            } else {                                // w is false, Literal(w) uncertain
                                if (!w_it->sign()) {                // posLit, Literal is false
                                    rule_judgement = RULE_FAIL;     // rule fail
                                    break;                          // no need to check rest
                                }
                            }
                        } else {                                    // body literal isn't assigned
                            rule_judgement = RULE_UNKNOWN;          // rule unknown
                            break;                                  // no need to check rest
                        }
                    }
                }
                if (rule_judgement == RULE_SATISFIED) { T_plus.insert(edge_it->first.var()); }
            }
        }
        return T_plus;
    }
    VarSet DependencyGraph::greatest_unfounded_set(const VarSet &P, const VarSet &N) {
        VarSet phi_known, phi_deduced = P, gus;  // phi is known or duduced from fact, gus is greatest unfounded set
        while (!reach_fixpoint(phi_known, phi_deduced)) {
            phi_known = phi_deduced;
            gl_reduce(phi_known);
            phi_deduced = T_once_plus(P, N);
            phi_deduced.insert(phi_known.begin(), phi_known.end());
        }
        std::set_difference(vertices.begin(), vertices.end(), phi_deduced.begin(), phi_deduced.end(), std::inserter(gus, gus.begin()));
        return gus;
    }
    VarSetPair DependencyGraph::W_once(const VarSet& P, const VarSet& N) {
        VarSet T_plus, U;
        T_plus = T_once_plus(P, N);
        U = greatest_unfounded_set(P, N);
        return VarSetPair(T_plus, U);
    }
    bool DependencyGraph::reach_fixpoint(const VarSet& A, const VarSet& B) {
        if (A.size() == B.size()) {
            for (VarSet::const_iterator it1 = A.begin(), it2 = B.begin(); it1 != A.end() && it2 != B.end(); ++it1, ++it2) {
                if (*it1 != *it2) {
                    return false;
                }
            }
        } else {
            return false;
        }
        return true;
    }
    bool DependencyGraph::reach_fixpoint(const VarSetPair& A1_B1, const VarSetPair& A2_B2) {
        return reach_fixpoint(A1_B1.first, A2_B2.first) && reach_fixpoint(A1_B1.second, A2_B2.second);
    }
    void DependencyGraph::mark_fixpoint_failure(VarSet& s) {
        s.insert(FIXPOINT_FAILURE_MARK);
    }
    void DependencyGraph::mark_fixpoint_failure(VarSetPair& s1_s2) {
        s1_s2.first.insert(FIXPOINT_FAILURE_MARK);
    }
    bool DependencyGraph::fixpoint_fail(const VarSet& s) {
        return s.find(FIXPOINT_FAILURE_MARK) != s.end();
    }
    bool DependencyGraph::fixpoint_fail(const VarSetPair& s1_s2) {
        return s1_s2.first.find(FIXPOINT_FAILURE_MARK) != s1_s2.first.end();
    }
    VarSetPair DependencyGraph::W_inf(const VarSet& P, const VarSet& N) {
        VarSetPair P_N = VarSetPair(P, N), P1_N1;
        unsigned long iterations = vertices_num;
        while (iterations--) {
            P1_N1 = W_once(P_N.first, P_N.second);
            if (reach_fixpoint(P_N, P1_N1)) {
                return P1_N1;
            } else {
                P_N = P1_N1;
            }
        }
        std::cout << "W-infinite Fail!" << std::endl;
        mark_fixpoint_failure(P1_N1);
        return P1_N1;
    }
    VarSetPair DependencyGraph::W_expand(const VarSet& P, const VarSet& N) {
        VarSetPair P_N_star = W_inf(P, N), J_K;
        // start T-expand
        graph_reduce(P_N_star.first, P_N_star.second);
        bool has_possible_consistent_SCC = true;
        while (has_possible_consistent_SCC) {
            has_possible_consistent_SCC = false;
            for (SCCVec::const_iterator scc_it = SCCs.begin(); scc_it != SCCs.end(); ++scc_it) {
                if (!has_outgoing_edge(*scc_it)) {  // with no outgoing edge
                    J_K = call_consistent(*scc_it);
                    if (!fixpoint_fail(J_K)) {
                        P_N_star.first.insert(J_K.first.begin(), J_K.first.end());  // P + J
                        P_N_star.second.insert(J_K.second.begin(), J_K.second.end());  // N + K
                        P_N_star = W_inf(P_N_star.first, P_N_star.second);
                        graph_reduce(P_N_star.first, P_N_star.second);
                        has_possible_consistent_SCC = true;
                        break;
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
//            std::cout << "Check: " << v.var() << ", DFN: " << DFN[v.var()] << std::endl;
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
//        std::cout << "Visit: " << v.var() << ", DFN: " << DFN[v.var()] << ", LOW: " << LOW[v.var()] << std::endl;
        tarjan_stack.push_back(v);
        Literal w;  // v -> w
        for (GraphType::const_iterator edge_it = graph_.begin(); edge_it != graph_.end(); ++edge_it) {
            if (edge_it->first.var() == v.var()) {
                for (LitVec::const_iterator w_it = edge_it->second.begin(); w_it != edge_it->second.end(); ++w_it) {  // v -> w
                    w = *w_it;
//                    std::cout << "\t-> " << w.var() << std::endl;
                    if (!w.watched()) {  // w is still in graph
//                        std::cout << "\tIn Graph: " << w.var() << ", Index: " << w.index() << std::endl;
//                        std::cout << "\tIn Stack: " << std::endl;
//                        for (LitVec::const_iterator it = tarjan_stack.begin(); it != tarjan_stack.end(); ++it) {
//                            std::cout << "\t\tElement: " << it->var() << ", Index: " << it->index() << std::endl;
//                        }
                        if (DFN[w.var()] <= 0) {  // not visited by tarjan
                            tarjan(w);
                            LOW[v.var()] = LOW[w.var()] < LOW[v.var()] ? LOW[w.var()] : LOW[v.var()];
//                            std::cout << "\tUpdate: " << v.var() << ", DFN: " << DFN[v.var()] << ", LOW: " << LOW[v.var()] << std::endl;
                        } else if (find_var(tarjan_stack, w)) {  // in stack
                            LOW[v.var()] = DFN[w.var()] < LOW[v.var()] ? DFN[w.var()] : LOW[v.var()];
//                            std::cout << "\tUpdate: " << v.var() << ", DFN: " << DFN[v.var()] << ", LOW: " << LOW[v.var()] << std::endl;
                        }
                    }
                }
            }
        }
//        std::cout << "Visit Finish. " << std::endl;
        if (LOW[v.var()] == DFN[v.var()]) {
            SCC scc;
            do {
                w = tarjan_stack.back();
                scc.insert(w);
                tarjan_stack.pop_back();
            } while (w.var() != v.var());
            SCCs.push_back(scc);
//            std::cout << "Strong Connected Component: ";
//            for (LitSet::const_iterator it = scc.begin(); it != scc.end(); ++it) {
//                std::cout << it->var() << " ";
//            }
//            std::cout << std::endl;
        }
    }
    VarSetPair DependencyGraph::call_consistent(const SCC& scc) {
        VarSet J, K;
        Var v = scc.begin()->var();
        call_consistent_dfs(scc, v, J, K, false);
        // ---print call_consistent_dfs result---
        std::cout << "---call-consistent---" << std::endl;
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
        // ---end print---
        DetailedGraphType::const_iterator signed_edge_it;
        bool p_in_J, q_in_J, pq_in_J, pq_in_K;  // !p_in_J implies p_in_K
        for (LitSet::const_iterator p_it = scc.begin(); p_it != scc.end(); ++p_it) {
            for (LitSet::const_iterator q_it = scc.begin(); q_it != scc.end(); ++q_it) { // p->q
                signed_edge_it = signed_edges_ptr->find(SimpleEdge(p_it->var(), q_it->var()));
                if (signed_edge_it != signed_edges_ptr->end()) {  // find p->q in graph
//                    std::cout << p_it->var() << " -> " << q_it->var() << ": " << (signed_edge_it->second == POS_EDGE ? "POS" : "NEG");
                    p_in_J = J.find(p_it->var()) != J.end();
                    q_in_J = J.find(q_it->var()) != J.end();
                    pq_in_J = p_in_J && q_in_J;
                    pq_in_K = !p_in_J && !q_in_J;
//                    std::cout << ", " << p_it->var() << " in " << (p_in_J ? "J" : "K") << ", " << q_it->var() << " in " << (q_in_J ? "J" : "K") << std::endl;
                    if ((signed_edge_it->second == POS_EDGE && (!pq_in_J && !pq_in_K))
                        || (signed_edge_it->second == NEG_EDGE && (pq_in_J || pq_in_K))) {
                        std::cout << "Call Consistent Fail: " << p_it->var() << " -> " << q_it->var() << std::endl;
                        mark_fixpoint_failure(J);
                        return VarSetPair(J, K);
                    }
                }
            }
        }
        return VarSetPair(J, K);
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
        for (SCCVec::const_iterator scc_it = SCCs.begin(); scc_it != SCCs.end(); ++scc_it) {
            if (!fixpoint_fail(call_consistent(*scc_it))) {
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
            if (!r_it->first.watched()) {
                std::cout << r_it->first.var() << " -> ";
                int first_term = 1;
                for (LitVec::const_iterator b_it = r_it->second.begin(); b_it != r_it->second.end(); ++b_it) {
                    if (!b_it->watched()) {
                        if (first_term) first_term = 0;
                        else std::cout << ", ";
                        if (b_it->sign()) std::cout << "[-]";
                        else std::cout << "[+]";
                        std::cout << (b_it->var());
                    }
                }
                std::cout << std::endl;
            }
        }
        std::cout << "---End---" << std::endl;
    }
}
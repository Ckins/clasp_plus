//
// Created by Junhong on 8/26/16.
//

#include <clasp/sysu_dependency_graph.h>

namespace Sysu {

    Rule::Rule() {}
    Rule::Rule(const Clasp::Asp::Rule &r) {
        for (Clasp::VarVec::const_iterator it = r.heads.begin(); it != r.heads.end(); ++it) {
            add_head(*it);
        }
        for (Clasp::WeightLitVec::const_iterator it = r.body.begin(); it != r.body.end(); ++it) {
            add_body(it->first);
        }
    }
    void Rule::add_head(const Var h) {
        vars.insert(h);
        heads.push_back(Clasp::posLit(h));
    }
    void Rule::add_body(const Literal b) {
        vars.insert(b.var());
        body.push_back(b);
    }
    bool Rule::is_constraint() {
        return heads.size() == 1 && heads[0].var() == 1;
    }

    void DependencyGraph::add_edge(const Rule &rule) {
        for (Clasp::LitVec::const_iterator h_it = rule.heads.begin(); h_it != rule.heads.end(); ++h_it) {
            graph.push_back(MultiEdge(*h_it, rule.body));
        }
        vertices.insert(rule.vars.begin(), rule.vars.end());
        vertices_num = vertices.size();
        // detailed graph
        for (LitVec::const_iterator h_it = rule.heads.begin(); h_it != rule.heads.end(); ++h_it) {
            for (LitVec::const_iterator b_it = rule.body.begin(); b_it != rule.body.end(); ++b_it) {
                if (b_it->sign()) {
                    detailed_edges.insert(DetailedEdge(SimpleEdge(h_it->var(), b_it->var()), NEG_EDGE));
                } else {
                    detailed_edges.insert(DetailedEdge(SimpleEdge(h_it->var(), b_it->var()), POS_EDGE));
                }
            }
        }
    }
    void DependencyGraph::resume() {
        for (GraphType::iterator edge_it = graph.begin(); edge_it != graph.end(); ++edge_it) {
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
        std::map<Var, bool>::const_iterator assign_it;
        for(VarSet::const_iterator it = P.begin(); it != P.end(); ++it) {
            assign[*it] = true;
        }
        for(VarSet::const_iterator it = N.begin(); it != N.end(); ++it) {
            assign[*it] = false;
        }
        // graph reduce
        for (GraphType::iterator edge_it = graph.begin(); edge_it != graph.end(); ++edge_it) {  // head: edge_it->first
            if (assign.find(edge_it->first.var()) != assign.end()) {
                edge_it->first.watch();                 // head in P or N, remove it
                continue;
            }
            for (LitVec::iterator body_it = edge_it->second.begin(); body_it != edge_it->second.end(); ++body_it) {  // body: *w_it
                assign_it = assign.find(body_it->var());
                if (assign_it != assign.end()) {
                    body_it->watch();                   // body in P or N, remove it
                    if (assign_it->second) {            // assigned true, Literal uncertain
                        if (body_it->sign()) {          // negLit, Literal is false
                            edge_it->first.watch();     // edge fail
                            break;
                        }
                    } else {                            // assigned false, Literal(w) uncertain
                        if (!body_it->sign()) {         // posLit, Literal is false
                            edge_it->first.watch();     // edge fail
                            break;
                        }
                    }
                }
            }
        }
        find_SCCs();
    }
    void DependencyGraph::gl_reduce(const VarSet &P) {
        resume();
        for (GraphType::iterator edge_it = graph.begin(); edge_it != graph.end(); ++edge_it) {  // head: edge_it->first
            for (LitVec::iterator body_it = edge_it->second.begin(); body_it != edge_it->second.end(); ++body_it) {  // body: *w_it
                if (body_it->sign()) {                          // negLit
                    if (P.find(body_it->var()) != P.end()) {    // assigned true, negLit is false
                        edge_it->first.watch();                 // remove edge by removing head
                        break;                                  // no need to check rest
                    } else {                                    // assigned false or unknown, negLit is true or unknown
                        body_it->watch();                       // remove this negLit
                    }
                }
            }
        }
    }
    VarSet DependencyGraph::deduce(const VarSet& P) {
        VarSet facts;
        RULE_SATISFACTION rule_judgement;
        // remove body and collect head
        for (GraphType::iterator edge_it = graph.begin(); edge_it != graph.end(); ++edge_it) {  // head(v): edge_it->first
            if (!edge_it->first.watched()) {                        // rule is still in graph
                rule_judgement = RULE_SATISFIED;
                for (LitVec::iterator w_it = edge_it->second.begin(); w_it != edge_it->second.end(); ++w_it) {  // body(w): *w_it
                    if (!w_it->watched()) {                         // body literal is still in graph
                        if (P.find(w_it->var()) != P.end()) {       // body literal is not assigned true
                            w_it->watch();                          // remove body
                        } else {
                            rule_judgement = RULE_UNKNOWN;          // rule unknown
                        }
                    }
                }
                if (rule_judgement == RULE_SATISFIED) {
                    facts.insert(edge_it->first.var());
                }
            }
        }
        // add already known facts
        facts.insert(P.begin(), P.end());
        return facts;
    }
    VarSet DependencyGraph::T_once_plus(const VarSet& P, const VarSet& N) {
        // check rule satisfaction
        VarSet T_plus;
        RULE_SATISFACTION rule_judgement;
        for (GraphType::iterator edge_it = graph.begin(); edge_it != graph.end(); ++edge_it) {  // head(v): edge_it->first
            rule_judgement = RULE_SATISFIED;
            for (LitVec::iterator w_it = edge_it->second.begin(); w_it != edge_it->second.end(); ++w_it) {  // body(w): *w_it
                if (P.find(w_it->var()) != P.end()) {           // assigned true
                    if (w_it->sign()) {                         // negLit, Literal is false
                        rule_judgement = RULE_FAIL;             // rule fail
                        break;                                  // no need to check rest
                    }
                } else if (N.find(w_it->var()) != N.end()) {    // assigned false
                    if (!w_it->sign()) {                        // posLit, Literal is false
                        rule_judgement = RULE_FAIL;             // rule fail
                        break;                                  // no need to check rest
                    }
                } else {                                        // not assigned
                    rule_judgement = RULE_UNKNOWN;              // rule unknown
                    break;                                      // no need to check rest
                }
            }
            if (rule_judgement == RULE_SATISFIED) {
                T_plus.insert(edge_it->first.var());
            }
        }
        return T_plus;
    }
    VarSet DependencyGraph::greatest_unfounded_set(const VarSet &P) {
        gl_reduce(P);

        VarSet facts, increased_facts, gus;  // gus is greatest unfounded set
        do {
            facts = increased_facts;
            increased_facts = deduce(facts);
        } while (!same(facts, increased_facts));

        // atoms - lfs = greatest unfounded set(gus)
        OrderedVarSet ordered_facts(increased_facts.begin(), increased_facts.end());
        std::set_difference(vertices.begin(), vertices.end(), ordered_facts.begin(), ordered_facts.end(), std::inserter(gus, gus.begin()));

        return gus;
    }
    VarSetPair DependencyGraph::W_once(const VarSet& P, const VarSet& N, bool enhance) {
        VarSet T_plus, U, tmp1;
        T_plus = T_once_plus(P, N);
        U = greatest_unfounded_set(P);
        if (enhance) {
            T_plus.insert(P.begin(), P.end());  // insert large set too slow, therefore use unordered_set
            U.insert(N.begin(), N.end());
        }
        return VarSetPair(T_plus, U);
    }
    VarSetPair DependencyGraph::W_inf(const VarSet& P, const VarSet& N, bool enhance=false) {
        VarSetPair P_N = VarSetPair(P, N), P1_N1 = W_once(P_N.first, P_N.second, enhance);
        int iterations = (int)vertices_num;  // !additional one to check last same set
        while (!same(P_N, P1_N1) && iterations--) {
            P_N = P1_N1;
            P1_N1 = W_once(P_N.first, P_N.second, enhance);
        }
        if (iterations < 0 || (!enhance && P1_N1.first.empty() && P1_N1.second.empty() && !(P.empty() && N.empty()))) {
            mark_failure(P1_N1);
        }
        return P1_N1;
    }
    VarSetPair DependencyGraph::W_expand(const VarSet& P, const VarSet& N) {
        VarSetPair P_N_star, J_K;

//        std::cout << "W_inf" << std::endl;
        P_N_star = W_inf(P, N);
//        std::cout << "W_inf End" << std::endl;
        if (failed(P_N_star)) {
            return P_N_star;
        }

//        std::cout << "Reduce" << std::endl;
        graph_reduce(P_N_star.first, P_N_star.second);
//        std::cout << "Reduce End" << std::endl;

        std::cout << "Call-consistent Classification" << std::endl;
        bool has_possible_consistent_SCC = true;
        while (has_possible_consistent_SCC) {
            has_possible_consistent_SCC = false;
            for (SCCVec::const_iterator scc_it = SCCs.begin(); scc_it != SCCs.end(); ++scc_it) {
                if (!has_outgoing_edge(*scc_it)) {
//                    std::cout << "Get J&K" << std::endl;
                    J_K = call_consistent(*scc_it);
//                    std::cout << "Get J&K End" << std::endl;
                    if (!failed(J_K)) {  // a call-consistent scc without outgoing edge
                        P_N_star.first.insert(J_K.first.begin(), J_K.first.end());  // P + J
                        P_N_star.second.insert(J_K.second.begin(), J_K.second.end());  // N + K
//                        std::cout << "W_inf" << std::endl;
                        P_N_star = W_inf(P_N_star.first, P_N_star.second);
//                        std::cout << "W_inf End" << std::endl;
                        if (failed(P_N_star)) {
                            return P_N_star;
                        }
//                        std::cout << "Reduce" << std::endl;
                        graph_reduce(P_N_star.first, P_N_star.second);
//                        std::cout << "Reduce End" << std::endl;
                        has_possible_consistent_SCC = true;
                    } else {
                        return J_K;
                    }
                }
            }
        }
//        std::cout << "Call-consistent Classification End" << std::endl;

        return P_N_star;
    }
    bool DependencyGraph::has_outgoing_edge(SCC* scc) {
        for (SCC::const_iterator v_it = scc->begin(); v_it != scc->end(); ++v_it) {
            for (GraphType::iterator edge_it = graph.begin(); edge_it != graph.end(); ++edge_it) {  // head: edge_it->first
                if (*v_it == edge_it->first.var()  && !edge_it->first.watched()) {
                    for (LitVec::iterator body_it = edge_it->second.begin(); body_it != edge_it->second.end(); ++body_it) {
                        if (!body_it->watched() && scc->find(body_it->var()) == scc->end()) {  // in graph and not in scc
                            return true;
                        }
                    }
                }
            }
        }
        return false;
    }
    void DependencyGraph::find_SCCs() {
        clear_SCCs();
        tarjan_index = 0;
        tarjan_stack.clear();
        DFN = new int[vertices_num+10]();
        LOW = new int[vertices_num+10]();
        Literal v;
        // start tarjan
        for (GraphType::const_iterator edge_it = graph.begin(); edge_it != graph.end(); ++edge_it) {
            v = edge_it->first;
            if (!v.watched() && DFN[v.var()] <= 0) {  // w is still in graph & not visited by tarjan
                tarjan(v);
            }
        }
        // end tarjan
        delete[] DFN;
        delete[] LOW;
        std::cout << "SCC: " << SCCs.size() << std::endl;
    }
    bool DependencyGraph::find_var(const LitVec& vec, const Literal& item) {
        for (LitVec::const_iterator it = vec.begin(); it != vec.end(); ++it)
            if (it->var() == item.var())
                return true;
        return false;
    }
    void DependencyGraph::tarjan(const Literal& v) {
        DFN[v.var()] = LOW[v.var()] = ++tarjan_index;  // initial visiting mark
        tarjan_stack.push_back(v);
        Literal w;  // v -> w
        for (GraphType::const_iterator edge_it = graph.begin(); edge_it != graph.end(); ++edge_it) {
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
            SCC *scc = new SCC();
            do {
                w = tarjan_stack.back();
                scc->insert(w.var());
                tarjan_stack.pop_back();
            } while (w.var() != v.var());
            SCCs.push_back(scc);
        }
    }
    VarSetPair DependencyGraph::call_consistent(SCC* scc) {
        VarSet J, K;
        Var v = *(scc->begin());
        call_consistent_dfs(scc, v, J, K, false);
        DetailedGraphType::const_iterator signed_edge_it;
        bool p_in_J, q_in_J, pq_in_J, pq_in_K;  // !p_in_J implies p_in_K
        for (VarSet::const_iterator p_it = scc->begin(); p_it != scc->end(); ++p_it) {
            for (VarSet::const_iterator q_it = scc->begin(); q_it != scc->end(); ++q_it) { // p->q
                signed_edge_it = detailed_edges.find(SimpleEdge(*p_it, *q_it));
                if (signed_edge_it != detailed_edges.end()) {  // find p->q in graph
                    p_in_J = J.find(*p_it) != J.end();
                    q_in_J = J.find(*q_it) != J.end();
                    pq_in_J = p_in_J && q_in_J;
                    pq_in_K = !p_in_J && !q_in_J;
                    if ((signed_edge_it->second == POS_EDGE && !(pq_in_J || pq_in_K))
                        || (signed_edge_it->second == NEG_EDGE && (pq_in_J || pq_in_K))) {
                        mark_failure(J);
                        return VarSetPair(J, K);
                    }
                }
            }
        }
        return VarSetPair(J, K);
    };
    void DependencyGraph::call_consistent_dfs(SCC* scc, const Var& v, VarSet& J, VarSet& K, int mark) {
        if (mark) {
            J.insert(v);
        } else {
            K.insert(v);
        }
        DetailedGraphType::const_iterator it;
        for (VarSet::const_iterator w_it = scc->begin(); w_it != scc->end(); ++w_it) {  // v -> w
            it = detailed_edges.find(SimpleEdge(v, *w_it));
            if (it != detailed_edges.end() && J.find(*w_it) == J.end() && K.find(*w_it) == K.end()) {
                call_consistent_dfs(scc, *w_it, J, K, it->second == POS_EDGE ? mark : !mark);
            }
        }
    }
    bool DependencyGraph::whole_call_consistent() {
        for (SCCVec::const_iterator scc_it = SCCs.begin(); scc_it != SCCs.end(); ++scc_it) {
            if (failed(call_consistent(*scc_it))) {
                return false;
            }
        }
        return true;
    }
    void DependencyGraph::clear_SCCs() {
        for (SCCVec::iterator it = SCCs.begin(); it != SCCs.end(); ++it) {
            delete (*it);
        }
        SCCs.clear();
    }
    void DependencyGraph::print_SCC(SCC* scc) {
        std::cout << "SCC: ";
        for (VarSet::const_iterator it = scc->begin(); it != scc->end(); ++it) {
            std::cout << *it << " ";
        }
        std::cout << std::endl;
    }
    void DependencyGraph::print_SCCs() {
        std::cout << "---SCCs---" << std::endl;
        for (SCCVec::const_iterator it = SCCs.begin(); it != SCCs.end(); ++it) {
            print_SCC(*it);
        }
        std::cout << "---SCCs End---" << std::endl;
    }
    void DependencyGraph::print_graph() {
        std::cout << "---Dependency Graph---" << std::endl;
        for (GraphType::const_iterator r_it = graph.begin(); r_it != graph.end(); ++r_it) {
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
        std::cout << "---Dependency Graph End---" << std::endl;
    }
    bool DependencyGraph::same(const VarSet& A, const VarSet& B) {
        return A == B;
    }
    bool DependencyGraph::same(const VarSetPair& A1_B1, const VarSetPair& A2_B2) {
        return same(A1_B1.first, A2_B2.first) && same(A1_B1.second, A2_B2.second);
    }
    void DependencyGraph::mark_failure(VarSet& s) {
        s.insert(FAILURE_MARK);
    }
    void DependencyGraph::mark_failure(VarSetPair& s1_s2) {
        mark_failure(s1_s2.first);
    }
    bool DependencyGraph::failed(const VarSet& s) {
        return s.find(FAILURE_MARK) != s.end();  // find FIXPOINT_FAILURE_MARK
    }
    bool DependencyGraph::failed(const VarSetPair& s1_s2) {
        return failed(s1_s2.first);
    }
}
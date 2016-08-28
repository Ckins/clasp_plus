//
// Created by kinsang on 16-8-25.
//

#include <clasp/sysu_program.h>

namespace Sysu {

    Prg::Prg() { }

    Prg* Prg::getPrg() {
        static Prg prg;
        return &prg;
    }
    void Prg::add_rule(const Clasp::Asp::Rule& r) {
        Rule rule = Rule(r);
        // Rules Category
        if (rule.is_constraint()) {
            constraints.push_back(rule);
        } else {
            rules.push_back(rule);
            dependencyGraph.add_edge(rule);
        }
        // Edge Sign Table
        for (LitVec::const_iterator h_it = rule.heads.begin(); h_it != rule.heads.end(); ++h_it) {
            for (LitVec::const_iterator b_it = rule.body.begin(); b_it != rule.body.end(); ++b_it) {
                if (b_it->sign()) {
                    signed_edges.insert(SignedEdge(SimpleEdge(h_it->var(), b_it->var()), NEG_EDGE));
                } else {
                    signed_edges.insert(SignedEdge(SimpleEdge(h_it->var(), b_it->var()), POS_EDGE));
                }
            }
        }
        // Expose signed edges table to dependency graph
        dependencyGraph.signed_edges_ptr = &signed_edges;
    }

    void Prg::do_solve(const VarSet &P, const VarSet &N) {
        break_constraint(P, N);

        // every reduce the graph find scc automatically
        dependencyGraph.graph_reduce(P, N);

        if (!dependencyGraph.whole_call_consistent()) {
            return;
        }

        VarSetPair P_N_1 = dependencyGraph.T_once(P, N);
        // if fixed point

        if (same_set(P_N_1, VarSetPair(P, N))) {
            finalize(P_N_1);
        }

        // (p', N') = W.inf(P', N')
        VarSetPair P_N_star = dependencyGraph.T_inf(P_N_1.first, P_N_1.second);

        // if (P', N') is null return;
        if (empty_set(P_N_star)) return;

        // dg.reduce(p', n') and findScc()
        // if dg.whole_call-consistent, w_expand()

        dependencyGraph.graph_reduce(P_N_star.first, P_N_star.second);
        if (dependencyGraph.whole_call_consistent()) {
            finalize(P_N_star);
        }
    }

    void Prg::finalize(const VarSetPair &P_N) {
        VarSetPair result = dependencyGraph.T_expand(P_N.first, P_N.second);
        report_answer(result.first);
    }

    void Prg::report_answer(const VarSet &P) {
        // todo traversal and print the answer
        std::cout << "We construct an answer set!" << std::endl;
        // std::cout << P.size() << std::endl;
        VarSet::iterator it = P.begin();

        for (; it != P.end(); it++) std::cout << *it << " ";
        std::cout << std::endl;
        /*for (Clasp::SymbolTable::const_iterator it = symbolTablePtr->begin(); it != symbolTablePtr->end(); ++it) {

            std::cout << (*it).second.lit.var() << std::endl;
            /*if (P.find(it->second.lit.var()) != P.end()) {
                std::cout << "true:" << it->second.name.c_str() << std::endl;
            }
        }*/
    }

    bool Prg::same_set(const VarSetPair &P_N, const VarSetPair &P_N_2) {
        //
        if (P_N.first.size() != P_N.first.size()
                || P_N_2.second.size() != P_N_2.second.size()) return false;

        // check P
        VarSet::const_iterator it = P_N.first.begin();
        for (; it != P_N.first.end(); it++) {
            if (P_N_2.first.find(*it) == P_N_2.first.end()) return false;
        }

        // check N
        VarSet::const_iterator n_it = P_N.second.begin();
        for (; n_it != P_N.second.end(); n_it++) {
            if (P_N_2.second.find(*n_it) == P_N_2.second.end()) return false;
        }
        // both P N are the same.
        return true;
    }

    bool Prg::empty_set(const VarSetPair &P_N) {
        return (P_N.first.size() == 0 && P_N.second.size() == 0);
    }

    bool Prg::break_constraint(const VarSet &P, const VarSet &N) {

        // todo check constraint
        return true;
    }

    void Prg::print_rules(const RuleVec& l) {
        int first_term;
        for (RuleVec::const_iterator r_it = l.begin(); r_it != l.end(); ++r_it) {
            first_term = 1;
            for (LitVec::const_iterator it = r_it->heads.begin(); it != r_it->heads.end(); ++it) {
                if (first_term) first_term = 0;
                else std::cout << " | ";
                std::cout << it->var();
            }
            std::cout << " :- ";
            first_term = 1;
            for (LitVec::const_iterator it = r_it->body.begin(); it != r_it->body.end(); ++it) {
                if (first_term) first_term = 0;
                else std::cout << ", ";
                if (it->sign()) std::cout << "not ";
                std::cout << it->var();
            }
            std::cout << "." << std::endl;
        }
    }

    void Prg::print() {
        std::cout << "---Normal Rules & Facts---" << std::endl;
        print_rules(rules);
        std::cout << "---Constraints---" << std::endl;
        print_rules(constraints);
        std::cout << "---End---" << std::endl;
    }
}
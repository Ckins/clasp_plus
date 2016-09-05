//
// Created by kinsang on 16-8-25.
//

#include <clasp/sysu_program.h>

namespace Sysu {

    Prg::Prg() {}

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
        // edge-sign Table
        for (LitVec::const_iterator h_it = rule.heads.begin(); h_it != rule.heads.end(); ++h_it) {
            for (LitVec::const_iterator b_it = rule.body.begin(); b_it != rule.body.end(); ++b_it) {
                if (b_it->sign()) {
                    signed_edges.insert(SignedEdge(SimpleEdge(h_it->var(), b_it->var()), NEG_EDGE));
                } else {
                    signed_edges.insert(SignedEdge(SimpleEdge(h_it->var(), b_it->var()), POS_EDGE));
                }
            }
        }
        // sxpose edge-sign table to dependency graph
        dependencyGraph.signed_edges_ptr = &signed_edges;
    }

    void Prg::do_solve(const VarSet &P, const VarSet &N) {

        // if the partial assignment can not break constraints, quit
        if (break_constraint(P, N)) {
            return;
        }

        // every reduce the graph find scc automatically
        dependencyGraph.graph_reduce(P, N);
        if (!dependencyGraph.whole_call_consistent()) {
            return;
        }

        // (p×, N×) = W.inf(P, N)
        // iterative steps to get the fixed point under W
        VarSetPair P_N_star = dependencyGraph.W_expand(P, N);
        if (!dependencyGraph.failed(P_N_star) && !break_constraint(P_N_star.first, P_N_star.second)) {
            report_answer(P_N_star.first);
        }

        // if (P', N') is null return;
        // if (empty_set(P_N_star)) return;

        // if (P, N) is already a fixed pointed

        // todo check whether the p_n_star already has

        // if the return fixed point is call-consistent, it could be expanded to AS
    }

    void Prg::report_answer(const VarSet &P) {
        std::cout << "\n===Answer Set===" << std::endl;
        for (VarSet::iterator it = P.begin(); it != P.end(); ++it) {
            std::cout << *it;
            for (Clasp::SymbolTable::const_iterator s_it = symbolTablePtr->begin();
                 s_it != symbolTablePtr->end(); ++s_it) {
                if (s_it->first == *it) {
                    std::cout << "(" << s_it->second.name.c_str() << ") ";
                }
            }
        }
        std::cout << "\n===Answer Set End===" << std::endl;
    }

    bool Prg::break_constraint(const VarSet &P, const VarSet &N) {
        RULE_SATISFACTION constraint_judgement;
        for (RuleVec::const_iterator c_it = constraints.begin(); c_it != constraints.end(); ++c_it) {  // constraint
            constraint_judgement = RULE_SATISFIED;
            for (LitVec::const_iterator b_it = c_it->body.begin(); b_it != c_it->body.end(); ++b_it) {  // constraint body
                if (P.find(b_it->var()) != P.end()) {           // true value
                    if (b_it->sign()) {                         // negLit, literal is false
                        constraint_judgement = RULE_FAIL;       // constraint fail
                        break;
                    }
                } else if (N.find(b_it->var()) != N.end()) {    // false value
                    if (!b_it->sign()) {                        // posLit, literal is false
                        constraint_judgement = RULE_FAIL;       // constraint fail
                        break;
                    }
                } else {
                    constraint_judgement = RULE_UNKNOWN;
                    break;
                }
            }
            if (constraint_judgement == RULE_SATISFIED) {
                std::cout << "Break Constraint: "; print_rule(*c_it);
                return true;
            }
        }
        return false;
    }

    void Prg::print_rule(const Rule& r) {
        int first_term = 1;
        for (LitVec::const_iterator it = r.heads.begin(); it != r.heads.end(); ++it) {
            if (first_term) first_term = 0;
            else std::cout << " | ";
            std::cout << it->var();
        }
        std::cout << " :- ";
        first_term = 1;
        for (LitVec::const_iterator it = r.body.begin(); it != r.body.end(); ++it) {
            if (first_term) first_term = 0;
            else std::cout << ", ";
            if (it->sign()) std::cout << "not ";
            std::cout << it->var();
        }
        std::cout << "." << std::endl;
    }

    void Prg::print_rules(const RuleVec& l) {
        for (RuleVec::const_iterator r_it = l.begin(); r_it != l.end(); ++r_it) {
            print_rule(*r_it);
        }
    }

    void Prg::print_program() {
        std::cout << "\n---Normal Rules & Facts---" << std::endl;
        print_rules(rules);
        std::cout << "---Constraints---" << std::endl;
        print_rules(constraints);
        std::cout << "---End---\n" << std::endl;
    }
}

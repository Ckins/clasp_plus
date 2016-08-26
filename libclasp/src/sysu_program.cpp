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
        atomSet.insert(rule.atomVars.begin(), rule.atomVars.end());
        if (rule.is_constraint()) {
            constraintList.push_back(rule);
        } else {
            ruleList.push_back(rule);
        }
        // Edges Type
        for (VarVec::const_iterator h_it = rule.heads.begin(); h_it != rule.heads.end(); ++h_it) {
            for (LitVec::const_iterator b_it = rule.body.begin(); b_it != rule.body.end(); ++b_it) {
                if (b_it->sign()) {
                    signed_edges.push_back(SignedEdge(Edge(*h_it, b_it->var()), NEG_EDGE));
                } else {
                    signed_edges.push_back(SignedEdge(Edge(*h_it, b_it->var()), POS_EDGE));
                }
            }
        }
    }

    void Prg::do_solve(const LitSet &P, const LitSet &N) {

        DependencyGraph dg(ruleList);
        // dg.reduce(P, N);

        if (dg.whole_call_consistent()) {

        }

    }
    void Prg::print_rules(const RuleVec& rules) {
        for (RuleVec::const_iterator r_it = rules.begin(); r_it != rules.end(); ++r_it) {
            int first_term = 1;
            for (VarVec::const_iterator it = r_it->heads.begin(); it != r_it->heads.end(); ++it) {
                if (first_term) first_term = 0;
                else std::cout << "; ";
                std::cout << (*it);
            }
            std::cout << " :- ";
            first_term = 1;
            for (LitVec::const_iterator it = r_it->body.begin(); it != r_it->body.end(); ++it) {
                if (first_term) first_term = 0;
                else std::cout << ", ";
                if (it->sign()) std::cout << "not ";
                std::cout << (it->var());
            }
            std::cout << "." << std::endl;
        }
    }

    void Prg::print_all_rules() {
        DependencyGraph dg = DependencyGraph(ruleList);
        std::cout << "---Rules---" << std::endl;
        print_rules(ruleList);
        std::cout << "---Constraints---" << std::endl;
        print_rules(constraintList);
        std::cout << "---Edges---" << std::endl;
        dg.print_all_edges();
        std::cout << "---End---\n" << std::endl;
    }
}
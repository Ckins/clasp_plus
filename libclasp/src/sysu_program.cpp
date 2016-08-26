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
        atoms.insert(rule.vars.begin(), rule.vars.end());
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
                    signed_edges.push_back(SignedEdge(Edge(*h_it, *b_it), NEG_EDGE));
                } else {
                    signed_edges.push_back(SignedEdge(Edge(*h_it, *b_it), POS_EDGE));
                }
            }
        }
    }

    void Prg::do_solve(const LitSet &P, const LitSet &N) {

        // DependencyGraph dg(ruleList);
        // dg.reduce(P, N);

//        if (dg.whole_call_consistent()) {
//
//        }

    }

    void Prg::print_rules(const RuleVec& l) {
        for (RuleVec::const_iterator r_it = l.begin(); r_it != l.end(); ++r_it) {
            int first_term = 1;
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
        dependencyGraph.print();
        std::cout << "---Normal Rules & Facts---" << std::endl;
        print_rules(rules);
        std::cout << "---Constraints---" << std::endl;
        print_rules(constraints);
        std::cout << "---End---\n" << std::endl;
    }
}
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
        rules.push_back(Rule(r));
    }

    void Prg::do_solve(const AtomSet &P, const AtomSet &N) { }

    void Prg::print_all_rules() {
        std::cout << "---Rules---" << std::endl;
        for (RuleList::const_iterator r_it = rules.begin(); r_it != rules.end(); ++r_it) {
            std::cout << "Rule: ";
            int first_term = 1;
            for (Clasp::VarVec::const_iterator it = r_it->heads.begin(); it != r_it->heads.end(); ++it) {
                if (first_term) first_term = 0;
                else std::cout << "; ";
                std::cout << (*it);
            }
            std::cout << " :- ";
            first_term = 1;
            for (Clasp::WeightLitVec::const_iterator it = r_it->body.begin(); it != r_it->body.end(); ++it) {
                if (first_term) first_term = 0;
                else std::cout << ", ";
                if (it->first.sign()) std::cout << "not ";
                std::cout << (it->first.var());
            }
            std::cout << std::endl;
        }
        std::cout << "---End---" << std::endl;
    }
}
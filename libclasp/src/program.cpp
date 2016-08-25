//
// Created by kinsang on 16-8-25.
//

#include <clasp/program.h>
#include <iostream>

namespace Sysu {
    Prg::Prg() {
    }
    void Prg::print_all_rules() {
        for (RuleList::const_iterator rit = rules.begin(); rit != rules.end(); ++rit) {
            std::cout << "Rule: ";
            int first_term = 1;
            Rule* r = *rit;
            for (Clasp::VarVec::const_iterator it = r->heads.begin(); it != r->heads.end(); ++it) {
                if (first_term) first_term = 0;
                else std::cout << " ; ";
                std::cout << (*it);
            }
            std::cout << " :- ";
            first_term = 1;
            for (Clasp::WeightLitVec::const_iterator it = r->body.begin(); it != r->body.end(); ++it) {
                if (first_term) first_term = 0;
                else std::cout << " , ";
                if (it->first.sign()) std::cout << "not ";
                std::cout << (it->first.var());
            }
            std::cout << std::endl;
        }
    }
    DependencyGraph::DependencyGraph() {

    }
}
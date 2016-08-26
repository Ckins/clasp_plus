//
// Created by kinsang on 16-8-25.
//

#include <clasp/program.h>
#include <iostream>

namespace Sysu {
    // Rule
    Rule::Rule(const Clasp::Asp::Rule &r)  {
        heads = r.heads;  // [Literal.var, Literal.var, ...]
        body = r.body;  // [<Literal, weight>, <Literal, weight>, ...]
    }

    // DependencyGraph
    DependencyGraph::DependencyGraph() {}
    DependencyGraph::DependencyGraph(const RuleList& rules) {
        Rule* rulePtr;
        Clasp::Var headVar;
        AtomSet bodySet;
        Literal bodyLiteral;
        for (RuleList::const_iterator r_it = rules.begin(); r_it != rules.end(); ++r_it) {
            rulePtr = *r_it;
            for (Clasp::VarVec::const_iterator h_it = rulePtr->heads.begin(); h_it != rulePtr->heads.end(); ++h_it) {
                headVar = *h_it;
                bodySet.clear();
                for (Clasp::WeightLitVec::const_iterator b_it = rulePtr->body.begin(); b_it != rulePtr->body.end(); ++b_it) {
                    bodyLiteral = (*b_it).first;
                    bodySet.insert(bodyLiteral);
                    if (bodyLiteral.sign()) {
                        edgeType.insert(std::pair<Edge, bool>(Edge(headVar, bodyLiteral.var()), 0));
                    } else {
                        edgeType.insert(std::pair<Edge, bool>(Edge(headVar, bodyLiteral.var()), 1));
                    }
                }
                depGraph.insert(std::pair<Var, AtomSet>(headVar, bodySet));
            }
        }
    }
    void DependencyGraph::tarjan() {

    }

    // Prg
    Prg* Prg::getPrg() {
        static Prg prg;
        return &prg;
    }
    Prg::Prg() {
    }
    Prg::~Prg() {
        for (RuleList::const_iterator r_it = rules.begin(); r_it != rules.end(); ++r_it) {
            delete (*r_it);
        }
    }
    void Prg::add_rule(Rule* rule) {
        rules.push_back(rule);
    }
    void Prg::print_all_rules() {
        dependencyGraph = DependencyGraph(rules);
        for (RuleList::const_iterator r_it = rules.begin(); r_it != rules.end(); ++r_it) {
            std::cout << "Rule: ";
            int first_term = 1;
            Rule* rulePtr = *r_it;
            for (Clasp::VarVec::const_iterator it = rulePtr->heads.begin(); it != rulePtr->heads.end(); ++it) {
                if (first_term) first_term = 0;
                else std::cout << "; ";
                std::cout << (*it);
            }
            std::cout << " :- ";
            first_term = 1;
            for (Clasp::WeightLitVec::const_iterator it = rulePtr->body.begin(); it != rulePtr->body.end(); ++it) {
                if (first_term) first_term = 0;
                else std::cout << ", ";
                if (it->first.sign()) std::cout << "not ";
                std::cout << (it->first.var());
            }
            std::cout << std::endl;
        }
    }
}
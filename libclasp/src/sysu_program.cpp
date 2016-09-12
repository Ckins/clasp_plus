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
    }

    void Prg::do_solve(const VarSet &P, const VarSet &N) {

        // preprocessing - use enhanced W to get expanded (P, N) and make sure it doesn't break constraint
        if (verbose) { std::cout << "Preprocessing..." << std::endl; }
        VarSetPair P_N = dependencyGraph.W_inf(P, N, true);
        if (dependencyGraph.failed(P_N) || break_constraint(P_N)) {
            return;
        }

        // optimization - judge whether reduced dependency graph is whole call-consistent at first
        if (verbose) { std::cout << "Optimizing..." << std::endl; }
        dependencyGraph.graph_reduce(P_N.first, P_N.second);
        if (!dependencyGraph.whole_call_consistent()) {
            if (verbose) { std::cout << "Program not Whole Call-Consistent." << std::endl; }
            return;
        }

        // solving - (P*, N*) = W.inf(P, N), iterative steps to get the fixed point under W
        if (verbose) { std::cout << "Solving..." << std::endl; }
        VarSetPair P_N_star = dependencyGraph.W_expand(P_N.first, P_N.second);
        if (!dependencyGraph.failed(P_N_star)) {
            if (!break_constraint(P_N_star)) {
                report_answer(P_N_star.first);
            }
        } else {
            if (verbose) { std::cout << "Program Fail." << std::endl; }
        }

        // if (P', N') is null return;
        // if (empty_set(P_N_star)) return;

        // if (P, N) is already a fixed pointed

        // todo check whether the p_n_star already has

        // if the return fixed point is call-consistent, it could be expanded to AS
    }

    void Prg::report_answer(const VarSet &P) {
        std::string path("../");
        path += name;
        path += ".result";
        FILE *awswer_set_stream = fopen( path.c_str(), "w" );
        if (verbose) {
            std::cout << "\n===Answer Set===" << std::endl;
            for (VarSet::const_iterator it = P.begin(); it != P.end(); ++it) {
                for (Clasp::SymbolTable::const_iterator s_it = symbolTablePtr->begin();
                     s_it != symbolTablePtr->end(); ++s_it) {
                    if (s_it->first == *it) {
                        std::cout << s_it->second.name.c_str() << " ";
                        fprintf( awswer_set_stream, "%s ", s_it->second.name.c_str());
                    }
                }
            }
            std::cout << "\n===Answer Set End===" << std::endl;
        } else {
            for (VarSet::const_iterator it = P.begin(); it != P.end(); ++it) {
                for (Clasp::SymbolTable::const_iterator s_it = symbolTablePtr->begin();
                     s_it != symbolTablePtr->end(); ++s_it) {
                    if (s_it->first == *it) {
                        fprintf( awswer_set_stream, "%s ", s_it->second.name.c_str());
                    }
                }
            }
        }
        fclose( awswer_set_stream );
        exit(88);
    }

    bool Prg::break_constraint(const VarSetPair &P_N) {
        RULE_SATISFACTION constraint_judgement;
        for (RuleVec::const_iterator c_it = constraints.begin(); c_it != constraints.end(); ++c_it) {  // constraint
            constraint_judgement = RULE_SATISFIED;
            for (LitVec::const_iterator b_it = c_it->body.begin(); b_it != c_it->body.end(); ++b_it) {  // constraint body
                if (P_N.first.find(b_it->var()) != P_N.first.end()) {           // true value
                    if (b_it->sign()) {                         // negLit, literal is false
                        constraint_judgement = RULE_FAIL;       // constraint fail
                        break;
                    }
                } else if (P_N.second.find(b_it->var()) != P_N.second.end()) {    // false value
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
                if (verbose) { std::cout << "Break Constraint: "; print_rule(*c_it); }
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

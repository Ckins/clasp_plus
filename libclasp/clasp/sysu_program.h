//
// Created by kinsang on 16-8-25.
//

#ifndef CLASP_PROGRAM_H
#define CLASP_PROGRAM_H

/*!
 * \file
 * namespace sysu
 * Contains the definition of the class program and related classes.
 */
#include <clasp/sysu_dependency_graph.h>

namespace Sysu {
    class DependencyGraph;

    /*
     * This is a Singleton class similar to ShareContext in Clasp
     * It could be accessed in the parsing process
     * and partial assignment process during DPLL
     * Complete information of the original program is stored here
     * and key algorithms are implemented on the classes above
     *
     */
    class Prg {
    public:
        std::string name;
        // member
        const Clasp::SymbolTable *symbolTablePtr;
        DependencyGraph dependencyGraph;
        RuleVec rules;
        RuleVec constraints;

        /*
         * Input : Partial assignment, P, N
         * Output : Print the answer set or return to clasp
        */

        void do_solve(const VarSet& P, const VarSet& N);

        /*
         * check whether the assignment violate constraints
         */
        bool break_constraint(const VarSetPair &P_N);


        /*
         * print the answer set
         */

        void report_answer(const VarSet &P);


        // auxiliary function
        static Prg *getPrg();
        // add/check information
        void add_rule(const Clasp::Asp::Rule& r);
        void print_program();
    private:
        Prg();
        void print_rule(const Rule& rule);
        void print_rules(const RuleVec& rules);
    };
}

#endif //CLASP_PROGRAM_H

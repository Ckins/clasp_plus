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
        // member
        DependencyGraph dependencyGraph;
        const Clasp::SymbolTable *symbolTablePtr;
        RuleList rules;
        AtomSet atomSet;

       /* void doSolve(P, N) {
            if (!is_call_consistent(P, N)) {
                return;
            }

            <p2, n2> = well_founded(P, N);

            // it must be able to expand the answer set!
            if (pair<P,N> == pair<P2, N2>) {
                print w-expand(p2, n2)
                        exit(10);
            }



            else {
                // p2, n2 may be able to be larger
                do {
                    p2, n2 =well_founded(P2, N2);
                } while (p2,n2 change);

                if (p2, n2 is call-consistent) {

                   print w-expand(p2, n2);
                }
            }
        }


        // key interface
        bool is_call_consistent(AtomSet &P, AtomSet &N) {

            return dependencyGraph->is_call_consistent(P, N);

        } */


        /* It reduces the ruleList under (P, N)
        * produce a temporary rulelist corresponding to a graph
        * Construct the memebr "dependencyGraph"
         *
         * Input : original RuleList, P, N
         * Output : the pointer pointing at the gragh with the reduced rules
        */
        // void reduce_with_assignment();

        void do_solve(const AtomSet& P, const AtomSet& N);

        // auxiliary function
        static Prg *getPrg();
        // add/check information
        void add_rule(const Clasp::Asp::Rule& r);
        void print_all_rules();
    private:
        Prg();
    };
}

#endif //CLASP_PROGRAM_H

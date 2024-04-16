#ifndef EXPLICITSTATEDFA_H
#define EXPLICITSTATEDFA_H

extern "C" {
#include <mona/bdd.h>
#include <mona/dfa.h>
#include <mona/mem.h>
}

#include "lydia/logic/ltlf/base.hpp"
#include "lydia/dfa/mona_dfa.hpp"
#include "VarMgr.h"

namespace Syft {

/*
 * Wrapper to Lydia DFA.
 */
    class ExplicitStateDfa : public whitemech::lydia::mona_dfa {
    public:

        ExplicitStateDfa(DFA* dfa, int nb_variables)
        : whitemech::lydia::mona_dfa(dfa, nb_variables) {
        }

        ExplicitStateDfa(DFA* dfa, const std::vector<std::string>& names)
                : whitemech::lydia::mona_dfa(dfa, names) {
        }

        ExplicitStateDfa(const ExplicitStateDfa& other)
        : whitemech::lydia::mona_dfa(dfaCopy(other.dfa_), other.names) {

        }

        ExplicitStateDfa& operator=(ExplicitStateDfa other)
        {
//            std::cout << "copy assignment of ExplicitStateDfa\n";
            this->names = other.names;
            this->dfa_ = dfaCopy(other.get_dfa());
            return *this;
        }
        // ExplicitStateDfa& operator=(const ExplicitStateDfa& other);

        ~ExplicitStateDfa(){
            
        }

        // rewrite this function, since nb_vairiables_ in mona_dfa is private
        int get_nb_variables() {
            return this->names.size();
        }

        /**
         * \brief Print the DFA.
         */
        void dfa_print();


        /**
         * \brief Construct DFA from a given formula
         *
         *
         * \param formula An LTLf formula.
         * \return The corresponding explicit-state DFA.
         */
        static ExplicitStateDfa dfa_of_formula(const whitemech::lydia::LTLfFormula& formula);

        /**
         * \brief Take the product AND of a vector of DFAs.
         *
         * \param dfa_vector The DFAs to be processed.
         * \return The product DFA.
         */
        static ExplicitStateDfa dfa_product_and(const std::vector<ExplicitStateDfa>& dfa_vector);

        /**
         * \brief Take the product AND of a vector of DFAs.
         *
         * \param dfa_vector The DFAs to be processed.
         * \return The product DFA.
         */
        static ExplicitStateDfa dfa_product_or(const std::vector<ExplicitStateDfa>& dfa_vector);

        /**
         * \brief Minimize a given DFA.
         *
         * \param d The DFA to be minimized.
         * \return The minimal DFA.
         */
        static ExplicitStateDfa dfa_minimize(const ExplicitStateDfa& d);


        /**
         * \brief Construct DFA from a given formula
         *
         *
         * \param formula An LTLf formula.
         * \return The corresponding explicit-state DFA.
         */
        static ExplicitStateDfa dfa_of_formula(const std::string& formula);

        /**
      * \brief Prune a DFA with given set of states.
      *
      * Basically remove a set of states from the DFA, and return a minimized one.
      *
      * \param d The DFA to be pruned.
      * \param states The set of states to be removed.
      * \return The pruned DFA.
      */
        static ExplicitStateDfa prune_dfa_with_states(ExplicitStateDfa& d, std::vector<size_t> states);

        /**
         * \brief Prune a DFA with given transitions.
         *
         * Basically remove a set of transitions from the DFA, and return a minimized one.
         *
         * \param d The DFA to be pruned.
         * \param transitions The set of transitions to be removed.
         * \return The pruned DFA.
         */
        static ExplicitStateDfa prune_dfa_with_transitions(ExplicitStateDfa& d, std::unordered_map<size_t, CUDD::BDD> transitions, std::shared_ptr<VarMgr> var_mgr);

        /**
         * \brief Complement a DFA.
         *
         * Basically exchange the accepting states and the non-accepting states.
         *
         * \param d The DFA to be complemented.
         * \return The complement DFA.
         */
        static ExplicitStateDfa dfa_complement(ExplicitStateDfa& d);

    private:
        static std::vector<std::string> traverse_bdd(CUDD::BDD dd, std::shared_ptr<VarMgr> var_mgr, std::vector<std::string>& names, std::string guard_str);
    };

}
#endif //EXPLICITSTATEDFA_H

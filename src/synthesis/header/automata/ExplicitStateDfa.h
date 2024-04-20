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

/**
 * \brief A DFA with explicit states and symbolic transitions.
 *
 * The LTLf-to-DFA construction utilizes Lydia.
 * Giuseppe De Giacomo, Marco Favorito: Compositional Approach to Translate LTLf/LDLf into Deterministic Finite Automata. ICAPS 2021: 122-130
 */
    class ExplicitStateDfa : public whitemech::lydia::mona_dfa {
    public:

        /**
         * \brief Create an explicit-state DFA from an Lydia DFA structure and the number of variables.
         *
         * \param dfa A Lydia DFA structure.
         * \param nb_variables The number of variables in the Lydia DFA.
         */
        ExplicitStateDfa(DFA *dfa, int nb_variables)
                : whitemech::lydia::mona_dfa(dfa, nb_variables) {
        }

        /**
         * \brief Create an explicit-state DFA from an Lydia DFA structure and the names of variables.
         *
         * \param dfa A Lydia DFA structure.
         * \param names The names of the variables in the Lydia DFA.
         */
        ExplicitStateDfa(DFA *dfa, const std::vector<std::string> &names)
                : whitemech::lydia::mona_dfa(dfa, names) {
        }

        /**
         * \brief Create an explicit-state DFA from an existing one.
         */
        ExplicitStateDfa(const ExplicitStateDfa &other)
                : whitemech::lydia::mona_dfa(dfaCopy(other.dfa_), other.names) {

        }

        /**
         * \brief Create an explicit-state DFA from an existing one using the opeartor =.
         */
        ExplicitStateDfa &operator=(ExplicitStateDfa other) {
            this->names = other.names;
            this->dfa_ = dfaCopy(other.get_dfa());
            return *this;
        }


        ~ExplicitStateDfa() {

        }

        /**
         * \brief Get the number of variables.
         */
        int get_nb_variables() {
            return this->names.size();
        }

        /**
         * \brief Print the explicit-state DFA.
         */
        void dfa_print();


        /**
         * \brief Construct an explicit-state DFA from a given formula using Lydia.
         *
         *
         * \param formula An LTLf formula.
         * \return The corresponding explicit-state DFA.
         */
        static ExplicitStateDfa dfa_of_formula(const whitemech::lydia::LTLfFormula &formula);

        /**
         * \brief Take the product AND of a sequence of explicit-state DFAs.
         *
         * \param dfa_vector The DFAs to be processed.
         * \return The product explicit-state DFA, which is also minimized.
         */
        static ExplicitStateDfa dfa_product_and(const std::vector<ExplicitStateDfa> &dfa_vector);

        /**
         * \brief Take the product OR of a sequence of explicit-state DFAs.
         *
         * \param dfa_vector The DFAs to be processed.
         * \return The product explicit-state DFA, which is also minimized.
         */
        static ExplicitStateDfa dfa_product_or(const std::vector<ExplicitStateDfa> &dfa_vector);

        /**
         * \brief Minimize a given explicit-state DFA.
         *
         * \param d The DFA to be minimized.
         * \return The minimal explicit-state DFA.
         */
        static ExplicitStateDfa dfa_minimize(const ExplicitStateDfa &d);


        /**
      * \brief Restrict an explicit-state DFA with a given set of states.
      *
      * Basically restrict a DFA to a set of states from the DFA, and return a minimized one.
      *
      * \param d The DFA to be restricted.
      * \param states The set of states to be kept.
      * \return The restricted explicit-state DFA, which is also minimized.
      */
        static ExplicitStateDfa restrict_dfa_with_states(ExplicitStateDfa &d, std::vector<size_t> states);

        /**
         * \brief Restrict a DFA with a given set of transitions.
         *
         * Basically restrict a DFA to a set of transitions, and return a minimized one.
         *
         * \param d The DFA to be restricted.
         * \param transitions The set of transitions to be kept.
         * \return The restricted explicit-state DFA, which is also minimized.
         */
        static ExplicitStateDfa
        restrict_dfa_with_transitions(ExplicitStateDfa &d, std::unordered_map<size_t, CUDD::BDD> transitions,
                                      std::shared_ptr<VarMgr> var_mgr);

        /**
         * \brief Complement a DFA.
         *
         * Basically exchange the accepting states and the non-accepting states.
         *
         * \param d The DFA to be complemented.
         * \return The complement DFA.
         */
        static ExplicitStateDfa dfa_complement(ExplicitStateDfa &d);

    private:
        static std::vector<std::string>
        traverse_bdd(CUDD::BDD dd, std::shared_ptr<VarMgr> var_mgr, std::vector<std::string> &names,
                     std::string guard_str);
    };

}
#endif //EXPLICITSTATEDFA_H

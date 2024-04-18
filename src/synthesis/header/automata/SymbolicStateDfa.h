#ifndef SYMBOLIC_STATE_DFA_H
#define SYMBOLIC_STATE_DFA_H

#include <memory>
#include <vector>

#include <cuddObj.hh>

#include "ExplicitStateDfaAdd.h"

namespace Syft {

/**
 * \brief A DFA with symbolic states and transitions.
 *
 * Shufang Zhu, Lucas M. Tabajara, Jianwen Li, Geguang Pu, Moshe Y. Vardi: Symbolic LTLf Synthesis. IJCAI 2017: 1362-1369
 */
    class SymbolicStateDfa {
    private:

        std::shared_ptr<VarMgr> var_mgr_;
        std::size_t automaton_id_;

        std::vector<int> initial_state_;
        CUDD::BDD final_states_;
        std::vector<CUDD::BDD> transition_function_;

        SymbolicStateDfa(std::shared_ptr<VarMgr> var_mgr);

        static std::pair<std::size_t, std::size_t> create_state_variables(
                std::shared_ptr<VarMgr> &mgr,
                std::size_t state_count);

        static CUDD::BDD state_to_bdd(const std::shared_ptr<VarMgr> &mgr,
                                      std::size_t automaton_id,
                                      std::size_t state);

        static CUDD::BDD state_set_to_bdd(
                const std::shared_ptr<VarMgr> &mgr,
                std::size_t automaton_id,
                const std::vector<size_t> &states);

        static std::vector<CUDD::BDD> symbolic_transition_function(
                const std::shared_ptr<VarMgr> &mgr,
                std::size_t automaton_id,
                const std::vector<CUDD::ADD> &transition_function);

    public:


        /**
         * \brief Converts an explicit DFA to a symbolic representation.
         *
         * Encodes the state space of the DFA in a logarithmic number of state
         * variables, using BDDs to represent the transition function and the set of
         * final states.
         *
         * \param explicit_dfa The explicit DFA to be converted.
         * \return The symbolic representation of the DFA.
         */
        static SymbolicStateDfa from_explicit(const ExplicitStateDfaAdd &explicit_dfa);

        /**
         * \brief Creates a simple automaton that remembers the value of predicates.
         *
         * \param var_mgr The variable manager to use to create the state variables.
         * \param predicates A vector of BDDs over named variables representing the
         *   predicates to remember.
         * \return A symbolic DFA with one state variable for each predicate. The i-th
         *   state variable records whether \a predicates[i] was true in the last
         *   step. The initial state has all state variables set to 0. All states are
         *   accepting states.
         */
        static SymbolicStateDfa from_predicates(std::shared_ptr<VarMgr> var_mgr,
                                                std::vector<CUDD::BDD> predicates);

        /**
         * \brief Returns the variable manager.
         */
        std::shared_ptr<VarMgr> var_mgr() const;

        /**
         * \brief Returns the automaton ID.
         *
         * This ID can be used to retrieve the associated state variables from the
         * variable manager.
         */
        std::size_t automaton_id() const;

        /**
         * \brief Returns the bitvector representing the initial state of the DFA.
         */
        std::vector<int> initial_state() const;

        /**
         * \brief Returns the BDD representing the initial state of the DFA.
         */
        CUDD::BDD initial_state_bdd() const;

        /**
         * \brief Returns the BDD encoding the set of final states.
         */
        CUDD::BDD final_states() const;

        /**
         * \brief Returns the transition function of the DFA as a vector of BDDs.
         *
         * The BDD in index \a i computes the value of state variable \a i in the
         * next step, given the current values of the state and alphabet variables.
         */
        std::vector<CUDD::BDD> transition_function() const;

        /**
            * \brief Restrict a symbolic DFA with a given set of states.
            *
            * Basically restrict a DFA to a set of states from the DFA.
            *
            * \param valid_states The set of states to be kept.
            * \return The restricted symbolic DFA.
            */
        void restrict_dfa_with_states(const CUDD::BDD &valid_states);

        /**
            * \brief Restrict a symbolic DFA with a set of feasible moves.
            *
            * \param feasible_moves The set of feasible moves to be kept.
            * \return The restricted symbolic DFA.
            */
        void restrict_dfa_with_transitions(const CUDD::BDD &feasible_moves);

        /**
         * \brief Saves the symbolic representation of the DFA in a .dot file.
         *
         * The file includes both the BDDs representing the transition function and
         * the BDD representing the final states.
         *
         * \param filename The name of the file to save the symbolic DFA to.
         */
        void dump_dot(const std::string &filename) const;

        /**
         * \brief Returns a product AND of two symbolic DFAs.
         *
         * \param first_dfa The first DFA.
         * \param second_dfa The second DFA.
         * \return A symbolic DFA of the product AND.
         */
        static SymbolicStateDfa product_AND(const std::vector<SymbolicStateDfa> &dfa_vector);

        /**
         * \brief Returns the binary encoding of a given state index.
         *
         * \param state The state index.
         * \param bit_count The number of bits in the binary encoding DFA.
         * \return The binary encoding.
         */
        static std::vector<int> state_to_binary(std::size_t state,
                                                std::size_t bit_count);

        /**
         * \brief Shrink a set of states to a sink state.
         *
         * \param sink_states The set of states to shrink.
         */
        void new_sink_states(const CUDD::BDD &sink_states);

        /**
    * \brief Returns a product AND of two symbolic DFAs.
    *
    * \param first_dfa The first DFA.
    * \param second_dfa The second DFA.
    * \return A symbolic DFA of the product OR.
    */
        static SymbolicStateDfa product_OR(const std::vector<SymbolicStateDfa> &dfa_vector);

        /**
    * \brief Returns the complement of a symbolic DFA.
    *
    * \param dfa The DFA to be complemented.
    * \return A symbolic DFA of the complement.
    */
        static SymbolicStateDfa complement(const SymbolicStateDfa dfa);
    };

}

#endif // SYMBOLIC_STATE_DFA_H

//
// Created by shuzhu on 21/01/24.
//

#ifndef LYDIASYFT_STABILITYLTLFSYNTHESIZER_H
#define LYDIASYFT_STABILITYLTLFSYNTHESIZER_H

#include "automata/SymbolicStateDfa.h"
#include "Synthesizer.h"

namespace Syft {


/**
 * \brief A single-strategy-synthesizer for an LTLf formula given as a symbolic-state DFA, considering environment simple Stability assumption.
 * The simple Stability assumption is in the form of FG\alpha, where \alpha is a Boolean formula over environment variables.
 *
 * 	Shufang Zhu, Giuseppe De Giacomo, Geguang Pu, Moshe Y. Vardi: LTLƒ Synthesis with Fairness and Stability Assumptions. AAAI 2020: 3088-3095
 */
    class StabilityLtlfSynthesizer {
    private:
        /**
         * \brief Variable manager.
         */
        std::shared_ptr<VarMgr> var_mgr_;
        /**
         * \brief The game arena.
         */
        SymbolicStateDfa spec_;
        /**
         * \brief The player that moves first each turn.
         */
        Player starting_player_;
        /**
         * \brief The player for which we aim to find the winning strategy.
         */
        Player protagonist_player_;
        /**
         * \brief The set of goal states.
         */
        CUDD::BDD goal_states_;
        /**
         * \brief The state space to consider.
         */
        CUDD::BDD state_space_;
        /**
         * \brief The simple Stability assumption represented as a Boolean formula \beta over input variables, denoting FG\beta
         */
        CUDD::BDD assumption_;

    protected:
        CUDD::BDD load_CNF(const std::string &filename) const;

    public:

        /**
         * \brief Construct an StabilityLtlfSynthesizer.
         *
         * \param spec A symbolic-state DFA representing the LTLf formula.
         * \param starting_player The player that moves first each turn.
         * \param protagonist_player The player for which we aim to find the winning strategy.
         * \param goal_states The set of states that the agent must reach to win.
         * \param state_space The state space.
         * \param assumption_filename The file that specifies a Boolean formula \alpha over input variables, denoting the simple Stability assumption FG\alpha.
         */
        StabilityLtlfSynthesizer(SymbolicStateDfa spec, Player starting_player, Player protagonist_player,
                                 CUDD::BDD goal_states, CUDD::BDD state_space,
                                 std::string &assumption_filename);


        /**
         * \brief Solves the synthesis problem of LTLf with simple Stability environment assumption.
         *
         * \return The synthesis result.
         */
        SynthesisResult run() const;

    };

}

#endif //LYDIASYFT_STABILITYLTLFSYNTHESIZER_H

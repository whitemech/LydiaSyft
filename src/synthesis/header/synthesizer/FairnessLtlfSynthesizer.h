//
// Created by shuzhu on 20/01/24.
//

#ifndef LYDIASYFT_FAIRNESSLTLFSYNTHESIZER_H
#define LYDIASYFT_FAIRNESSLTLFSYNTHESIZER_H

#include "automata/SymbolicStateDfa.h"
#include "Synthesizer.h"

namespace Syft {

/**
 * \brief A single-strategy-synthesizer for an LTLf formula given as a symbolic-state DFA, considering environment simple Fairness assumption.
 * The simple Fairness assumption is in the form of GF\alpha, where \alpha is a Boolean formula over environment variables.
 *
 * 	Shufang Zhu, Giuseppe De Giacomo, Geguang Pu, Moshe Y. Vardi:
 *  LTLƒ Synthesis with Fairness and Stability Assumptions. AAAI 2020: 3088-3095
 */
    class FairnessLtlfSynthesizer {
    private:
        SymbolicStateDfa spec_;
        Player starting_player_;
        Player protagonist_player_;
        CUDD::BDD goal_states_;
        CUDD::BDD state_space_;
        std::shared_ptr<VarMgr> var_mgr_;

        CUDD::BDD assumption_;

    protected:
        CUDD::BDD load_CNF(const std::string &filename) const;

    public:

        /**
         * \brief Construct a FairnessLtlfSynthesizer.
         *
         * \param spec A symbolic-state DFA representing the LTLf formula.
         * \param starting_player The player that moves first each turn.
         * \param protagonist_player The player for which we aim to find the winning strategy.
         * \param goal_states The set of states that the agent must reach to win.
         * \param state_space The state space.
         * \param assumption_filename The file that specifies a Boolean formula \alpha over input variables, denoting the simple Fairness assumption GF\alpha.
         */
        FairnessLtlfSynthesizer(const SymbolicStateDfa &spec, Player starting_player, Player protagonist_player,
                                const CUDD::BDD &goal_states, const CUDD::BDD &state_space,
                                const std::string &assumption_filename);

        /**
         * \brief Solves the synthesis problem of LTLf with simple Fairness environment assumption.
         *
         * \return The synthesis result.
         */
        SynthesisResult run() const;

    };

}

#endif //LYDIASYFT_FAIRNESSLTLFSYNTHESIZER_H

//
// Created by shuzhu on 20/01/24.
//

#ifndef LYDIASYFT_FAIRREACHABILITYSYNTHESIZER_H
#define LYDIASYFT_FAIRREACHABILITYSYNTHESIZER_H

#include "game/DfaGameSynthesizer.h"

namespace Syft {

/**
 * \brief A single-strategy-synthesizer for a reachability game given as a symbolic-state DFA, considering environment simple Fairness assumption.
 * The simple Fairness assumption is in the form of GF\alpha, where \alpha is a Boolean formula over environment variables.
 */
    class FairReachabilitySynthesizer : public DfaGameSynthesizer {
    private:

        CUDD::BDD goal_states_;
        CUDD::BDD state_space_;
        
        //fair
        CUDD::BDD assumption_;

    protected:
        CUDD::BDD load_CNF(const std::string &filename) const;

    public:

        /**
         * \brief Construct a single-strategy-synthesizer for the given coBuchi-reachability game.
         *
         * \param spec A symbolic-state DFA representing the coBuchi-reachability game arena.
         * \param starting_player The player that moves first each turn.
         * \param protagonist_player The player for which we aim to find the winning strategy.
         * \param goal_states The set of states that the agent must reach to win.
         * \param state_space The state space.
         * \param assumption_filename The file that specifies a Boolean formula \alpha over input variables, denoting the simple Fairness assumption GF\alpha.
         */
        FairReachabilitySynthesizer(const SymbolicStateDfa &spec, Player starting_player, Player protagonist_player,
                                    const CUDD::BDD &goal_states, const CUDD::BDD &state_space,
                                    const std::string &assumption_filename);

        /**
         * \brief Solves the coBuchi-reachability game.
         *
         * \return The result consists of
         * realizability
         * a set of agent winning states
         * a transducer representing a winning strategy or nullptr if the game is unrealizable.
         */
        virtual SynthesisResult run() const final;

    };

}

#endif //LYDIASYFT_FAIRREACHABILITYSYNTHESIZER_H

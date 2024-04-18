//
// Created by shuzhu on 16/04/24.
//

#ifndef LYDIASYFT_BUCHIREACHABILITY_HPP
#define LYDIASYFT_BUCHIREACHABILITY_HPP

#include "game/DfaGameSynthesizer.h"

namespace Syft {
/**
 * \brief A single-strategy-synthesizer for a Buchi-reachability game given as a symbolic-state DFA.
 *
 * Either Buchi condition holds or reachability condition holds.
 */
    class BuchiReachability : public DfaGameSynthesizer {
    private:
        /**
         * \brief The set of goal states.
         */
        CUDD::BDD goal_states_;
        /**
         * \brief The state space to consider.
         */
        CUDD::BDD state_space_;
        /**
         * \brief The Buchi condition represented as a Boolean formula \beta over input variables, denoting the Buchi condition GF\beta
         */
        CUDD::BDD Buchi_;

    public:

        /**
         * \brief Construct a single-strategy-synthesizer for the given Buchi-reachability game.
         *
         * \param spec A symbolic-state DFA representing the Buchi-reachability game arena.
         * \param starting_player The player that moves first each turn.
         * \param protagonist_player The player for which we aim to find the winning strategy.
         * \param goal_states The reachability condition.
         * \param Buchi The Buchi condition represented as a Boolean formula \beta over input variables, denoting the Buchi condition FG\beta.
         * \param state_space The state space.
         */
        BuchiReachability(const SymbolicStateDfa &spec, Player starting_player, Player protagonist_player,
                          const CUDD::BDD &goal_states, const CUDD::BDD &Buchi, const CUDD::BDD &state_space);


        /**
         * \brief Solves the Buchi-reachability game.
         *
         * \return The result consists of
         * realizability
         * a set of agent winning states
         * a transducer representing a winning strategy or nullptr if the game is unrealizable.
         */
        SynthesisResult run() const final;

    };
}


#endif //LYDIASYFT_BUCHIREACHABILITY_HPP

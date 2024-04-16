//
// Created by shuzhu on 16/04/24.
//

#ifndef LYDIASYFT_COBUCHIREACHABILITY_HPP
#define LYDIASYFT_COBUCHIREACHABILITY_HPP

#include "game/DfaGameSynthesizer.h"

namespace Syft {

/**
 * \brief A single-strategy-synthesizer for a coBuchi-reachability game given as a symbolic-state DFA.
 *
 * Either coBuchi condition holds or reachability condition holds.
 */
    class coBuchiReachability : public DfaGameSynthesizer {
    private:

        CUDD::BDD goal_states_;
        CUDD::BDD state_space_;

        CUDD::BDD coBuchi_;


    public:

        /**
         * \brief Construct a single-strategy-synthesizer for the given coBuchi-reachability game.
         *
         * \param spec A symbolic-state DFA representing the coBuchi-reachability game arena.
         * \param starting_player The player that moves first each turn.
         * \param protagonist_player The player for which we aim to find the winning strategy.
         * \param goal_states The reachability condition.
         * \param coBuchi The coBuchi condition represented as a Boolean formula \beta over input variables, denoting the coBuchi condition FG\beta.
         * \param state_space The state space.
         */
        coBuchiReachability(const SymbolicStateDfa &spec, Player starting_player, Player protagonist_player,
                            const CUDD::BDD &goal_states, const CUDD::BDD &coBuchi, const CUDD::BDD &state_space);


        /**
         * \brief Solves the coBuchi-reachability game.
         *
         * \return The result consists of
         * realizability
         * a set of agent winning states
         * a transducer representing a winning strategy or nullptr if the game is unrealizable.
         */
        SynthesisResult run() const final;

    };

}


#endif //LYDIASYFT_COBUCHIREACHABILITY_HPP

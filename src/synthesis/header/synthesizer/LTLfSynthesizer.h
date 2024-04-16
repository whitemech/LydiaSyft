#ifndef SYFT_REACHABILITYSYNTHESIZER_H
#define SYFT_REACHABILITYSYNTHESIZER_H

#include "game/DfaGameSynthesizer.h"

namespace Syft {

/**
 * \brief A single-strategy-synthesizer for a reachability game given as a symbolic-state DFA.
 */
    class LTLfSynthesizer {
    private:

        SymbolicStateDfa spec_;
        Player starting_player_;
        Player protagonist_player_;
        CUDD::BDD goal_states_;
        CUDD::BDD state_space_;
        std::shared_ptr<VarMgr> var_mgr_;

    public:

        /**
         * \brief Construct a single-strategy-synthesizer for the given reachability game.
         *
         * \param spec A symbolic-state DFA representing the reachability game arena.
         * \param starting_player The player that moves first each turn.
         * \param protagonist_player The player for which we aim to find the winning strategy.
         * \param goal_states The set of states that the agent must reach to win.
         * \param state_space The state space.
         */
        LTLfSynthesizer(SymbolicStateDfa spec, Player starting_player, Player protagonist_player,
                        CUDD::BDD goal_states, CUDD::BDD state_space);

        /**
         * \brief Solves the reachability game.
         *
         * \return The result consists of
         * realizability
         * a set of agent winning states
         * a transducer representing a winning strategy or nullptr if the game is unrealizable.
         */
        SynthesisResult run() const;

    };

}

#endif //SYFT_REACHABILITYSYNTHESIZER_H

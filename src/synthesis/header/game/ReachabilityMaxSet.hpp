//
// Created by shuzhu on 16/04/24.
//

#ifndef LYDIASYFT_REACHABILITYMAXSET_HPP
#define LYDIASYFT_REACHABILITYMAXSET_HPP


#include "game/DfaGameSynthesizer.h"

namespace Syft {
/**
 * \brief A maxset-strategy-synthesizer for a reachability game given as a symbolic-state DFA.
 *
 * Reachability condition holds.
 */
    class ReachabilityMaxSet : public DfaGameSynthesizer {
    private:
        /**
         * \brief The set of goal states.
         */
        CUDD::BDD goal_states_;
        /**
         * \brief The state space to consider.
         */
        CUDD::BDD state_space_;

    public:

        /**
         * \brief Construct a maxset-strategy-synthesizer for the given reachability game.
         *
         * \param spec A symbolic-state DFA representing the reachability game arena.
         * \param starting_player The player that moves first each turn.
         * \param protagonist_player The player for which we aim to find the winning strategy.
         * \param goal_states The reachability condition.
         * \param state_space The state space.
         */
        ReachabilityMaxSet(const SymbolicStateDfa &spec, Player starting_player, Player protagonist_player,
                           const CUDD::BDD &goal_states, const CUDD::BDD &state_space);


        /**
         * \brief Solves the maxset-reachability game.
         *
         * \return The result consists of
         * realizability
         * the non-deferring strategy
         * the deferring strategy.
         */
        MaxSetSynthesisResult run_maxset() const;

        /**
         * \brief Solve standard reachability game.
         *
         * \return The result consists of
         * realizability
         * the non-deferring strategy
         * the deferring strategy.
         */
        SynthesisResult run() const final;

        void dump_dot(MaxSetSynthesisResult maxset, const std::string &def_filename,
                      const std::string &nondef_filename) const;

    };
}


#endif //LYDIASYFT_REACHABILITYMAXSET_HPP

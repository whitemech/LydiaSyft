//
// Created by shuzhu on 20/01/24.
//

#ifndef SYFT_REACHABILITYMAXSETSYNTHESIZER_H
#define SYFT_REACHABILITYMAXSETSYNTHESIZER_H

#include "automata/SymbolicStateDfa.h"
#include "Synthesizer.h"
#include "game/ReachabilityMaxSet.hpp"

namespace Syft {

/**
 * \brief A maxset-synthesizer for a reachability game given as a symbolic-state DFA.
 *
 * Shufang Zhu, Giuseppe De Giacomo: Synthesis of Maximally Permissive Strategies for LTLf Specifications. IJCAI 2022: 2783-2789
 */
    class LTLfMaxSetSynthesizer {
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

    public:

        /**
         * \brief Construct a MaxSet-LtlfSynthesizer.
         *
         * \param spec A symbolic-state DFA representing the LTLf formula.
         * \param starting_player The player that moves first each turn.
         * \param protagonist_player The player for which we aim to find the winning strategy.
         * \param goal_states The set of states that the agent must reach to win.
         * \param state_space The state space.
         */
        LTLfMaxSetSynthesizer(SymbolicStateDfa spec, Player starting_player, Player protagonist_player,
                              CUDD::BDD goal_states, CUDD::BDD state_space);

        /**
         * \brief Solves the MaxSet-LTLf synthesis problem.
         *
         * \return The synthesis result.
         */
        MaxSetSynthesisResult run() const;

        void dump_dot(MaxSetSynthesisResult maxset, const std::string &def_filename,
                      const std::string &nondef_filename) const;

    };

}

#endif //SYFT_REACHABILITYMAXSETSYNTHESIZER_H

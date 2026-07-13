//
// Created by shuzhu on 16/04/24.
//

#ifndef LYDIASYFT_REACHABILITY_MULTIAGENT_HPP
#define LYDIASYFT_REACHABILITY_MULTIAGENT_HPP

#include "game/DfaGameSynthesizer_multiagent.h"
#include "Actor.h"

namespace Syft {
/**
 * \brief A single-strategy-synthesizer for a reachability game given as a symbolic-state DFA.
 *
 * Reachability condition holds.
 */
    class Reachability_multiagent : public DfaGameSynthesizer_multiagent {
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
         * \brief Construct a single-strategy-synthesizer for the given reachability game.
         *
         * \param spec A symbolic-state DFA representing the reachability game arena.
         * \param starting_actor The actor that moves first each turn.
         * \param protagonist_actor The actor for which we aim to find the winning strategy.
         * \param goal_states The reachability condition.
         * \param state_space The state space.
         * \param agent_variables The variables corresponding to each agent, used for quantification in the preimage computation.
         */
        Reachability_multiagent(const SymbolicStateDfa &spec, Actor starting_actor, Actor protagonist_actor,
                     const CUDD::BDD &goal_states, const CUDD::BDD &state_space, size_t num_agents);


        /**
         * \brief Solves the reachability game.
         *
         * \return The result consists of
         * realizability
         * a set of agent winning states
         * a transducer representing a winning strategy or nullptr if the game is unrealizable.
         */
        SynthesisResult run() const final;

    };
}


#endif //LYDIASYFT_REACHABILITY_HPP

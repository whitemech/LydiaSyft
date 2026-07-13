//
// Created by shuzhu on 16/04/24.
//

#ifndef LYDIASYFT_BUCHIREACHABILITY_MULTIAGENT_HPP
#define LYDIASYFT_BUCHIREACHABILITY_MULTIAGENT_HPP

#include "game/DfaGameSynthesizer_multiagent.h"

namespace Syft {
/**
 * \brief A single-strategy-synthesizer for a Buchi-reachability game given as a symbolic-state DFA.
 *
 * Either Buchi condition holds or reachability condition holds.
 */
    class BuchiReachability_multiagent : public DfaGameSynthesizer_multiagent {
    private:
        /**
         * \brief The set of goal states.
         */
        CUDD::BDD Buchi_; //states to visit inifinitely often
        /**
         * \brief The state space to consider.
         */
        CUDD::BDD state_space_;

    public:

        /**
         * \brief Construct a single-strategy-synthesizer for the given Buchi-reachability game.
         *
         * \param spec A symbolic-state DFA representing the Buchi-reachability game arena.
         * \param starting_actor The player that moves first each turn.
         * \param protagonist_actor The player for which we aim to find the winning strategy.
         * \param Buchi The Buchi condition represented as a Boolean formula \beta over input variables, denoting the Buchi condition FG\beta.
         * \param state_space The state space.
         * \param num_agents number of agents
         */
        BuchiReachability_multiagent(const SymbolicStateDfa &spec, Actor starting_actor, Actor protagonist_actor,
                          const CUDD::BDD &Buchi, const CUDD::BDD &state_space, size_t num_agents);


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

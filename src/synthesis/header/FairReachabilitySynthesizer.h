//
// Created by shuzhu on 20/01/24.
//

#ifndef LYDIASYFT_FAIRREACHABILITYSYNTHESIZER_H
#define LYDIASYFT_FAIRREACHABILITYSYNTHESIZER_H

#include "DfaGameSynthesizer.h"

namespace Syft {

/**
 * \brief A single-strategy-synthesizer for a coBuchi-reachability game given as a symbolic-state DFA.
 */
    class FairReachabilitySynthesizer : public DfaGameSynthesizer {
    private:

        CUDD::BDD goal_states_;
        CUDD::BDD state_space_;


        //fair
        CUDD::BDD assumption_;

    public:

        /**
         * \brief Construct a single-strategy-synthesizer for the given coBuchi-reachability game.
         *
         * \param spec A symbolic-state DFA representing the coBuchi-reachability game.
         * \param starting_player The player that moves first each turn.
         * \param goal_states The set of states that the agent must reach to win.
         */
        FairReachabilitySynthesizer(SymbolicStateDfa spec, Player starting_player, Player protagonist_player,
                                CUDD::BDD goal_states, CUDD::BDD state_space,
                                    CUDD::BDD assumption);



        /**
         * \brief Solves the coBuchi-reachability game.
         *
         * \return The result consists of
         * realizability
         * a set of agent winning states
         * a transducer representing a winning strategy or nullptr if the game is unrealizable.
         */
        virtual SynthesisResult run() const final;

        std::unique_ptr<Transducer> AbstractSingleStrategy(SynthesisResult result) const;

    };

}

#endif //LYDIASYFT_FAIRREACHABILITYSYNTHESIZER_H

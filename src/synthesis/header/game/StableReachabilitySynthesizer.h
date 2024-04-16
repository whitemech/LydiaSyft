//
// Created by shuzhu on 21/01/24.
//

#ifndef LYDIASYFT_STABLEREACHABILITYSYNTHESIZER_H
#define LYDIASYFT_STABLEREACHABILITYSYNTHESIZER_H

#include "game/DfaGameSynthesizer.h"

namespace Syft {

/**
 * \brief A single-strategy-synthesizer for a Buchi-reachability game given as a symbolic-state DFA.
 */
    class StableReachabilitySynthesizer : public DfaGameSynthesizer {
    private:

        CUDD::BDD goal_states_;
        CUDD::BDD state_space_;


        //fair
        CUDD::BDD assumption_;

    protected:
        CUDD::BDD load_CNF(const std::string &filename) const;

    public:

        /**
         * \brief Construct a single-strategy-synthesizer for the given Buchi-reachability game.
         *
         * \param spec A symbolic-state DFA representing the Buchi-reachability game.
         * \param starting_player The player that moves first each turn.
         * \param goal_states The set of states that the agent must reach to win.
         */
        StableReachabilitySynthesizer(SymbolicStateDfa spec, Player starting_player, Player protagonist_player,
                                      CUDD::BDD goal_states, CUDD::BDD state_space,
                                      std::string &assumption_filename);


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

#endif //LYDIASYFT_STABLEREACHABILITYSYNTHESIZER_H

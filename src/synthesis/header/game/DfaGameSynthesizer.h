#ifndef DFA_GAME_SYNTHESIZER_H
#define DFA_GAME_SYNTHESIZER_H

#include "Quantification.h"
#include "automata/SymbolicStateDfa.h"
#include "synthesizer/Synthesizer.h"
#include "Transducer.h"

namespace Syft {


/**
 * \brief A synthesizer for a game whose arena is a symbolic-state DFA.
 */
    class DfaGameSynthesizer : public Synthesizer<SymbolicStateDfa> {
    protected:
        /**
         * \brief Variable manager.
         */
        std::shared_ptr<VarMgr> var_mgr_;
        /**
         * \brief The player that moves first each turn.
         */
        Player starting_player_;
        /**
         * \brief The player for which we aim to find the winning strategy.
         */
        Player protagonist_player_;
        /**
         * \brief The initial state of the game arena.
         */
        std::vector<int> initial_vector_;
        /**
         * \brief The transition function of the game arena.
         */
        std::vector<CUDD::BDD> transition_vector_;
        /**
         * \brief Quantification on the variables that the protagonist player does not depend on.
         */
        std::unique_ptr<Quantification> quantify_independent_variables_;
        /**
         * \brief Quantification on non-state variables.
         */
        std::unique_ptr<Quantification> quantify_non_state_variables_;

        /**
         * \brief Compute a set of winning moves.
         *
         * Basically first collect the transitions that move into a winning state, and then quantify all variables that the protagonist player doesn't depend on.
         * \param winning_states A set of winning states.
         * \return The preimage.
         */
        CUDD::BDD preimage(const CUDD::BDD &winning_states) const;

        /**
         * \brief Project a set of winning moves to a set of winning states.
         *
         * Basically quantify all the non-state variables.
         * \param winning_moves A set of winning moves.
         * \return A set of winning states.
         */
        CUDD::BDD project_into_states(const CUDD::BDD &winning_moves) const;

        /**
         * \brief Check whether the initial state is a winning state.
         *
         * \param winning_states A set of winning states.
         * \return True if the initial state is a winning state.
         */
        bool includes_initial_state(const CUDD::BDD &winning_states) const;

    public:

        /**
         * \brief Construct a synthesizer for a given DFA game.
         *
         * The winning condition is unspecified and should be defined by the subclass.
         *
         * \param spec A symbolic-state DFA representing the game's arena.
         * \param starting_player The player that moves first each turn.
         * \param protagonist_player The player for which we aim to find the winning strategy.
         */
        DfaGameSynthesizer(SymbolicStateDfa spec, Player starting_player, Player protagonist_player);


        /**
         * \brief Synthesis for the game.
         *
         * \return The synthesis result, consisting of realizability, the set of winning states and the set of winning moves.
         */
        virtual SynthesisResult run()
        const override = 0;


        /**
         * \brief Abstract a winning strategy for the game.
         *
         * \return A winning strategy represented as a transducer.
         */
        std::unique_ptr<Transducer> AbstractSingleStrategy(const SynthesisResult &result) const;

    private:
        std::unique_ptr<Transducer> abstract_single_strategy(const CUDD::BDD &winning_moves,
                                                             const std::shared_ptr<VarMgr> &var_mgr,
                                                             const std::vector<int> &initial_vector,
                                                             const std::vector<CUDD::BDD> &transition_vector,
                                                             Player starting_player) const;

        std::unordered_map<int, CUDD::BDD>
        synthesize_strategy(const CUDD::BDD &winning_moves, const std::shared_ptr<VarMgr> &var_mgr) const;
    };

}

#endif // DFA_GAME_SYNTHESIZER_H

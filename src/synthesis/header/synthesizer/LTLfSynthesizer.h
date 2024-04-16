#ifndef SYFT_REACHABILITYSYNTHESIZER_H
#define SYFT_REACHABILITYSYNTHESIZER_H

#include "automata/SymbolicStateDfa.h"
#include "Synthesizer.h"

namespace Syft {

/**
 * \brief A single-strategy-synthesizer for an LTLf formula given as a symbolic-state DFA.
 *
 * 	Shufang Zhu, Lucas M. Tabajara, Jianwen Li, Geguang Pu, Moshe Y. Vardi:
 *  Symbolic LTLf Synthesis. IJCAI 2017: 1362-1369
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
         * \brief Construct an LtlfSynthesizer.
         *
         * \param spec A symbolic-state DFA representing the LTLf formula.
         * \param starting_player The player that moves first each turn.
         * \param protagonist_player The player for which we aim to find the winning strategy.
         * \param goal_states The set of states that the agent must reach to win.
         * \param state_space The state space.
         */
        LTLfSynthesizer(SymbolicStateDfa spec, Player starting_player, Player protagonist_player,
                        CUDD::BDD goal_states, CUDD::BDD state_space);

        /**
         * \brief Solves the LTLf synthesis problem.
         *
         * \return The synthesis result.
         */
        SynthesisResult run() const;

    };

}

#endif //SYFT_REACHABILITYSYNTHESIZER_H

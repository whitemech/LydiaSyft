//
// Created by shuzhu on 21/01/24.
//

#ifndef LYDIASYFT_GR1LTLFSYNTHESIZER_H
#define LYDIASYFT_GR1LTLFSYNTHESIZER_H

#include "automata/ExplicitStateDfaAdd.h"
#include "GR1.h"
#include "Stopwatch.h"
#include "automata/SymbolicStateDfa.h"
#include "Synthesizer.h"
#include "game/DfaGameSynthesizer.h"


namespace Syft {
/**
 * \brief A single-strategy-synthesizer for an LTLf formula given as a symbolic-state DFA, considering environment GR(1) assumption.
 * Additionally, one can also add safety conditions to both the environment and the agent, utilizing LTLf formulas.
 *
 * Giuseppe De Giacomo, Antonio Di Stasio, Lucas M. Tabajara, Moshe Y. Vardi, Shufang Zhu: Finite-Trace and Generalized-Reactivity Specifications in Temporal Synthesis. IJCAI 2021: 1852-1858
 */

    class GR1LTLfSynthesizer {
    private:
        /**
         * \brief Variable manager.
         */
        const std::shared_ptr<VarMgr> var_mgr_;
        /**
         * \brief The GR(1) assumption.
         */
        const GR1 gr1_;
        /**
         * \brief The symbolic-state DFA representing the LTLf formula indicating the environment safety condition.
         */
        const SymbolicStateDfa env_safety_;
        /**
         * \brief The symbolic-state DFA representing the LTLf formula indicating the system safety condition.
         */
        const SymbolicStateDfa agn_safety_;
        /**
         * \brief The symbolic-state DFA representing the LTLf formula indicating the system reachability condition.
         */
        const SymbolicStateDfa agn_reach_;
        /**
         * \brief The name of the benchmark file.
         */
        const std::string benchmark_name_;
        /**
         * \brief The root directory of SLUGS
         */
        const std::string slugs_dir_;

    public:
        /**
         * \brief Construct a GR(1)LtlfSynthesizer.
         *
         * \param gr1 The GR(1) environment assumption.
         * \param env_safety A symbolic-state DFA representing the LTLf formula indicating the environment safety condition
         * \param agn_reach  A symbolic-state DFA representing the LTLf formula indicating the system reachability condition
         * \param agn_safety A symbolic-state DFA representing the LTLf formula indicating the system safety condition
         * \param slugs_dir  The root directory of SLUGS
         */
        GR1LTLfSynthesizer(const std::shared_ptr<VarMgr> &var_mgr, const GR1 &gr1,
                           const SymbolicStateDfa &env_safety,
                           const SymbolicStateDfa &agn_reach, const SymbolicStateDfa &agn_safety,
                           const std::string &slugs_dir,
                           const std::string &benchmark_name);

        /**
         * \brief Solves the synthesis problem of LTLf with GR(1 environment assumption.
         *
         * \return The synthesis result.
         */
        SynthesisResult run() const;

    };
}
#endif //LYDIASYFT_GR1LTLFSYNTHESIZER_H

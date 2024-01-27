//
// Created by shuzhu on 21/01/24.
//

#ifndef LYDIASYFT_GR1REACHABILITYSYNTHESIZER_H
#define LYDIASYFT_GR1REACHABILITYSYNTHESIZER_H
#include "ExplicitStateDfa.h"
#include "InputOutputPartition.h"
#include "Player.h"
#include "GR1.h"
#include "Stopwatch.h"
#include "SymbolicStateDfa.h"
#include "Synthesizer.h"
#include "DfaGameSynthesizer.h"


namespace  Syft {

    class GR1ReachabilitySynthesizer {
    private:
        const std::shared_ptr<VarMgr>& var_mgr_;
        const GR1& gr1_;
        const SymbolicStateDfa& env_safety_;
        const SymbolicStateDfa& agn_safety_;
        const SymbolicStateDfa& agn_reach_;
        const std::string& benchmark_name_;

        std::string slugs_dir_;

        /**
         * \brief Prints out INPUT and OUTPUT variables in Slugs format
         * [INPUT]
         * a
         * b:0...10
         *
         * [OUTPUT]
         * c:2...8
         * d
         *
         * \param GR1 game arena
         * \param The file that should be written to
         */
        void print_variables(const SymbolicStateDfa& arena, const std::string& filename) const;

        /**
         * \brief Prints out INITIAL conditions in Slugs format
         * Boolean formulas are in infix notations
         * [ENV_INIT]
         * !a
         * b = 1
         * [SYS_INIT]
         * c | d
         * \param GR1 game arena
         * \param The file that should be written to
         */
        void print_initial_conditions(const CUDD::BDD& arena_initial_state_bdd, const std::string& filename) const;

        /**
         * \brief Prints out TRANSITIONS in Slugs format
         * [ENV_TRANS]
         * !a
         * b | b'
         * [SYS_TRANS]
         * c | d
         *
         * \param GR1 game arena
         * \param var_mgr var manager
         * \param The file that should be written to
         */
        void print_transitions(const SymbolicStateDfa& arena, const CUDD::BDD& safe_states, const std::string& filename) const;

        /**
         * \brief Prints out LIVENESS constraints in Slugs format
         * [ENV_LIVENESS]
         * ! a | (b = 3)
         * [SYS_LIVENESS]
         * d
         * c = 2
         * \param The file that should be written to
         */
        void print_liveness_constraints(const std::string& filename) const;

        /**
         * Get the path to the SLUGS binary.
         *
         * @return the path to the SLUGS binary.
         */
        std::string get_slugs_path() const;

    public:

        /**
         * \brief Construct a synthesizer for the given LTLf/GR(1) game.
         *
         * \param gr1 A GR(1) formula stating the winning condition.
         * \param env_safety A symbolic-state DFA representing the environment safety game arena
         * \param agn_reach  A symbolic-state DFA representing the system reachability game arena
         * \param agn_safety A symbolic-state DFA representing the system safety game arena
         * \param slugs_dir  The root directory of the SLUGS project
         *
         */
        GR1ReachabilitySynthesizer(const std::shared_ptr<VarMgr>& var_mgr, const GR1& gr1, const SymbolicStateDfa& env_safety,
                           const SymbolicStateDfa& agn_reach, const SymbolicStateDfa& agn_safety, const std::string& slugs_dir,
                           const std::string& benchmark_name);

        const std::string exec_slugs(const std::string& slugs, const std::string& slugs_input_file,
                                     const std::string& slugs_res_file, const std::string& slugs_strategy_file) const;

        /**
         * \brief Solves the LTLf/GR(1) game.
         *
         * \return The result consists of
         * realizability
         * a set of agent winning states
         * a transducer representing a winning strategy or nullptr if the game is unrealizable.
         */
        SynthesisResult run() const;

    };
}
#endif //LYDIASYFT_GR1REACHABILITYSYNTHESIZER_H

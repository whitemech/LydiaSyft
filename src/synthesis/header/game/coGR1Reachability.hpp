//
// Created by shuzhu on 18/04/24.
//

#ifndef LYDIASYFT_COGR1REACHABILITY_HPP
#define LYDIASYFT_COGR1REACHABILITY_HPP

#include "GR1.h"
#include "automata/SymbolicStateDfa.h"
#include "Synthesizer.h"

namespace Syft {
/**
 * \brief A single-strategy-synthesizer for a Buchi-reachability game given as a symbolic-state DFA.
 *
 * Either Buchi condition holds or reachability condition holds.
 */
    class coGR1Reachability {
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
         * \brief The game arena. Here we only use the transition function of the game arena.
         */
        const SymbolicStateDfa arena_;
        /**
         * \brief The state space to consider.
         */
        const CUDD::BDD state_space_;
        /**
         * \brief The initial condition of the arena in a BDD representation.
         */
        const CUDD::BDD initial_condition_;
        /**
         * \brief The name of the benchmark file, which we use to name the input file of SLUGS.
         */
        const std::string benchmark_name_;
        /**
         * \brief The root directory of SLUGS
         */
        const std::string slugs_dir_;

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
        void print_variables(const std::string &filename) const;

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
        void print_initial_conditions(const CUDD::BDD &arena_initial_state_bdd, const std::string &filename) const;

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
        void print_transitions(const SymbolicStateDfa &arena, const CUDD::BDD &safe_states,
                               const std::string &filename) const;

        /**
         * \brief Prints out LIVENESS constraints in Slugs format
         * [ENV_LIVENESS]
         * ! a | (b = 3)
         * [SYS_LIVENESS]
         * d
         * c = 2
         * \param The file that should be written to
         */
        void print_liveness_constraints(const std::string &filename) const;

        /**
         * Get the path to the SLUGS binary.
         *
         * @return the path to the SLUGS binary.
         */
        std::string get_slugs_path() const;


    public:

        /**
         * \brief Construct a single-strategy-synthesizer for the given Buchi-reachability game.
         *
         * \param spec A symbolic-state DFA representing the Buchi-reachability game arena.
         * \param starting_player The player that moves first each turn.
         * \param protagonist_player The player for which we aim to find the winning strategy.
         * \param goal_states The reachability condition.
         * \param Buchi The Buchi condition represented as a Boolean formula \beta over input variables, denoting the Buchi condition FG\beta.
         * \param state_space The state space.
         */
        coGR1Reachability(const std::shared_ptr<VarMgr> &var_mgr, const GR1 &gr1,
                          const SymbolicStateDfa &arena,
                          const CUDD::BDD &state_space,
                          const CUDD::BDD &initial_condition,
                          const std::string &slugs_dir,
                          const std::string &benchmark_name);

        const std::string exec_slugs(const std::string &slugs, const std::string &slugs_input_file,
                                     const std::string &slugs_res_file, const std::string &slugs_strategy_file) const;


        /**
         * \brief Solves the Buchi-reachability game.
         *
         * \return The result consists of
         * realizability
         * a set of agent winning states
         * a transducer representing a winning strategy or nullptr if the game is unrealizable.
         */
        SynthesisResult run() const;

    };
}


#endif //LYDIASYFT_COGR1REACHABILITY_HPP

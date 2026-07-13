#ifndef OBLIGATION_LTLF_PLUS_SYNTHESIZER_H
#define OBLIGATION_LTLF_PLUS_SYNTHESIZER_H

#include "automata/SymbolicStateDfa.h"
#include "automata/ExplicitStateDfa.h"
//#include "game/BuchiSolver.hpp"
#include "game/InputOutputPartition.h"
#include "lydia/logic/ltlfplus/base.hpp"
#include "automata/SymbolicStateDfa.h"
#include "Synthesizer.h"
#include "game/InputOutputPartition.h"
#include "lydia/parser/ppltl/driver.hpp"

#include <map>
#include <memory>
#include <string>
#include <utility>

struct MinimisationOptions {
    bool allow_minimisation = true;
    int threshold = 128;  // By default, only minimise small weak automata
    int symbolic_threshold = 128;
};

namespace CUDD {
    // forward-declare minimal wrapper type used in signatures
    class BDD;
}

namespace Syft {

    // Forward declarations for types used by the synthesizer interface.
    // These are declared elsewhere in your codebase.
    class VarMgr;
    struct ELSynthesisResult;
    struct SynthesisResult;

    /**
     * \brief Synthesizer for LTLf+ formulas in the obligation fragment.
     *
     * This synthesizer is optimized for the obligation fragment, which consists
     * only of safety (forall) and guarantee (exists) quantifiers.
     *
     */
    class ObligationLTLfPlusSynthesizer {
    public:
        /**
         * Construct an ObligationLTLfPlusSynthesizer.
         *
         * \param ltlf_plus_formula  parsed LTLf+ spec (contains formula->color/quantification)
         * \param partition          input/output partition (variable names)
         * \param starting_actor    who moves first each turn
         * \param protagonist_actor the actor we synthesise for (Agent/Environment)
         * \param allow_minimisation if true, allows minimisation of intermediate DFAs to save memory
         */
        ObligationLTLfPlusSynthesizer(
            LTLfPlus ltlf_plus_formula,
            InputOutputPartition partition,
            Actor starting_actor,
            Actor protagonist_actor,
            //MinimisationOptions minimisation_options = MinimisationOptions(),
            std::shared_ptr<VarMgr> var_mgr,
            bool use_balanced_boolean_product = true
        );

        /**
         * Run the synthesizer.
         *
         * \return result in ELSynthesisResult format.
         */
        ELSynthesisResult run() const;

        /**
         * \brief If set to true, run() will build the automata arena but skip
         * the game-solving step and return an empty result.
         * Useful for benchmarking automata construction time only.
         */
        void set_skip_synthesis(bool skip) { skip_synthesis_ = skip; }

        /**
         * \brief Set a file path to dump the arena DFA in PyDFA format.
         * Works with both normal synthesis and --skip-synthesis mode.
         */
        void set_dfa_dump_path(const std::string& path) { dfa_dump_path_ = path; }



        //QUESTE TRE FUNZIONI ERANO PRIVATE PRIMA
        /**
         * Parse a boolean color formula and build the arena using a hybrid approach.
         * Starts with explicit MONA DFAs for efficiency, but automatically switches
         * to symbolic representation when the state space exceeds a threshold (256 states).
         * This prevents memory blowup on large products while maintaining MONA efficiency
         * for smaller intermediate results.
         *
         * Returns a SymbolicStateDfa representing the complete product arena.
         */
        SymbolicStateDfa build_arena_from_color_formula_hybrid(
            const std::string& color_formula,
            const std::map<int, ExplicitStateDfa>& color_to_dfa) const;

        // --- core phases ---
        void validate_obligation_fragment() const;

        /**
         * \brief Builds the per-color DFAs, combines them according to the color
         * formula and returns the arena and a map of per-color finals.
         *
         * The returned map has per-color final BDDs under their color key and
         * the combined/evaluated final-states of the whole arena under key -1.
         */
        std::pair<SymbolicStateDfa, std::map<int, CUDD::BDD>>
        convert_to_symbolic_dfa() const;



        private:
        // --- state ---
        std::shared_ptr<VarMgr> var_mgr_;
        LTLfPlus ltlf_plus_formula_;
        Actor starting_actor_;
        Actor protagonist_actor_;
        MinimisationOptions minimisation_options_ = MinimisationOptions();
    bool use_balanced_boolean_product_ = true;
    mutable bool skip_synthesis_ = false;
    std::string dfa_dump_path_;  ///< If non-empty, dump arena DFA in PyDFA format to this file

    
        /**
         * \brief Solve the synthesis problem by running the Büchi solver on the arena.
         *
         * The arena is expected to already encode the correct accepting states
         * (i.e., arena.final_states() matches the boolean color formula).
         *
         * \param arena The product arena built from color DFAs.
         * \param color_to_final_states mapping from color -> final-states BDD (key -1 holds arena finals).
         */
        //ELSynthesisResult solve_with_scc(const SymbolicStateDfa& arena,
        //                                 const std::map<int, CUDD::BDD>& color_to_final_states) const;

    /**
     * 
     * Run the Büchi-based solver on the given arena. This is provided
     * separately from the SCC-based weak-game solver so callers can
     * choose the algorithm at runtime.
     */
    //ELSynthesisResult solve_with_buchi(const SymbolicStateDfa& arena,
    //                   const std::map<int, CUDD::BDD>& color_to_final_states) const;

        // --- helpers exposed because they're implemented in the .cpp ---
        /**
         * Parse a boolean color formula string like "(1 & 2) | 3" and build an explicit
         * DFA product using MONA's dfaProduct where numeric tokens reference ExplicitStateDfa
         * provided in color_to_dfa.
         *
         * This computes the product at the MONA level before conversion to symbolic representation.
         */
       /*  ExplicitStateDfa build_explicit_arena_from_color_formula(
            const std::string& color_formula,
            const std::map<int, ExplicitStateDfa>& color_to_dfa) const;
 */
            /**
         * (Optional) Evaluate a boolean color formula by substituting color integers
         * with given BDDs and computing the resulting BDD. Useful if you prefer to
         * compute the accepting set directly rather than relying on arena.final_states().
         */
        CUDD::BDD evaluate_color_formula_with_bdds(
            const std::string& color_formula,
            const std::map<int, CUDD::BDD>& color_to_bdd) const;
    };

} // namespace Syft

#endif // OBLIGATION_LTLF_PLUS_SYNTHESIZER_H

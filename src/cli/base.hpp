//
// Created by marcofavorito on 23/01/24.
//

#ifndef LYDIASYFT_CLI_BASE_HPP
#define LYDIASYFT_CLI_BASE_HPP

#include <string>
#include <CLI/CLI.hpp>
#include "lydia/types.hpp"
#include "Player.h"
#include "VarMgr.h"
#include "Parser.h"
#include "game/InputOutputPartition.h"
#include "game/Transducer.h"
#include "Stopwatch.h"
#include "Synthesizer.h"
#include "synthesizer/LTLfMaxSetSynthesizer.h"
#include "lydia/parser/ltlf/driver.hpp"


namespace Syft {
    static const std::string DEFAULT_SYFCO_PATH_ = "./syfco";
    static const std::string DEFAULT_SLUGS_PATH_ = "./slugs";

    struct TLSFArgs {
        const Player starting_player;
        const Player protagonist_player;
        const InputOutputPartition partition;
        const whitemech::lydia::ltlf_ptr formula;
    };

    class Printer {
    private:
        const bool print_strategy_;
        const bool print_times_;

        // for output messages
        std::ostream &out_;

    public:
        Printer(bool print_strategy, bool print_times, std::ostream &out) :
                print_strategy_(print_strategy), print_times_(print_times), out_(out) {}

        void
        dump_transducer_if_enabled(const Transducer &transducer, const std::string &output_file = "strategy.dot") const;

        void dump_add_if_enabled(const std::shared_ptr<VarMgr> &var_mgr, const CUDD::ADD &add,
                                 const std::string &output_file) const;

        void
        dump_maxset_if_enabled(const LTLfMaxSetSynthesizer &maxset_synthesizer,
                               const MaxSetSynthesisResult &maxset_strategy,
                               const std::string &def_strategy_output_file = "def_strategy.dot",
                               const std::string &nondef_strategy_output_file = "nondef_strategy.dot") const;

        void print_times_if_enabled(const std::string &message, std::chrono::milliseconds time) const;

        void print_realizable() const { out_ << Syft::REALIZABLE_STR << std::endl; }

        void print_unrealizable() const { out_ << Syft::UNREALIZABLE_STR << std::endl; }

    };


    void add_assumption_file_option(CLI::App *, std::string &);

    void add_gr1_file_option(CLI::App *, std::string &);

    void add_env_safety_file_option(CLI::App *, std::optional<std::string> &);

    void add_agent_safety_file_option(CLI::App *, std::optional<std::string> &);

    void add_spec_file_option(CLI::App *, std::string &);

    void add_syfco_option(CLI::App *, std::string &);

    void add_slugs_option(CLI::App *, std::string &);

    TLSFArgs parse_tlsf(const std::shared_ptr<whitemech::lydia::parsers::ltlf::LTLfDriver> &driver,
                        const std::string &path_to_syfco, const std::string &formula_file);

    whitemech::lydia::ltlf_ptr parse_formula(const std::shared_ptr<whitemech::lydia::parsers::ltlf::LTLfDriver> &driver,
                                             const std::string &formula);

    std::shared_ptr<Syft::VarMgr> build_var_mgr(const InputOutputPartition &partition);

    SymbolicStateDfa
    do_dfa_construction(const whitemech::lydia::LTLfFormula &formula, const std::shared_ptr<Syft::VarMgr> &var_mgr);

    class BaseRunner {
    protected:
        const std::string formula_file_;
        const std::string path_to_syfco_;

        const TLSFArgs args_;
        const std::shared_ptr<Syft::VarMgr> var_mgr_;

        const Printer printer_;

        std::shared_ptr<whitemech::lydia::parsers::ltlf::LTLfDriver> driver_;

        bool handle_preprocessing_result_(const OneStepSynthesisResult &one_step_result,
                                          Stopwatch &total_time_stopwatch) const;

        void handle_synthesis_result_(const DfaGameSynthesizer &synthesizer, const SynthesisResult &result) const;

        void handle_synthesis_result_(const SynthesisResult &result) const;

        SymbolicStateDfa do_dfa_construction_() const;

    public:
        BaseRunner(const std::shared_ptr<whitemech::lydia::parsers::ltlf::LTLfDriver> &driver,
                   const std::string &formula_file, const std::string &path_to_syfco, bool print_strategy,
                   bool print_times) :
                driver_(driver),
                formula_file_(formula_file), path_to_syfco_(path_to_syfco),
                printer_(print_strategy, print_times, std::cout),
                args_(parse_tlsf(driver, path_to_syfco, formula_file)),
                var_mgr_(build_var_mgr(args_.partition)) {}
    };

}

#endif //LYDIASYFT_CLI_BASE_HPP

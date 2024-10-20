//
// Created by marcofavorito on 23/01/24.
//

#include "base.hpp"
#include "lydia/parser/ltlf/driver.hpp"
#include "game/InputOutputPartition.h"
#include "Synthesizer.h"
#include "misc.h"
#include "Parser.h"

namespace Syft {

    void Printer::dump_transducer_if_enabled(const Transducer &transducer, const std::string &output_file) const {
        if (print_strategy_) {
            transducer.dump_dot(output_file);
        }
    }

    void Printer::dump_add_if_enabled(const std::shared_ptr<Syft::VarMgr> &var_mgr, const CUDD::ADD &add,
                                      const std::string &output_file) const {
        if (print_strategy_) {
            var_mgr->dump_dot(add, output_file);
        }
    }

    void Printer::dump_maxset_if_enabled(const LTLfMaxSetSynthesizer &maxset_synthesizer,
                                         const MaxSetSynthesisResult &maxset_strategy,
                                         const std::string &def_strategy_output_file,
                                         const std::string &nondef_strategy_output_file) const {
        if (print_strategy_) {
            maxset_synthesizer.dump_dot(maxset_strategy, def_strategy_output_file, nondef_strategy_output_file);
        }
    }

    void Printer::print_times_if_enabled(const std::string &message, std::chrono::milliseconds time) const {
        if (print_times_) {
            out_ << message << ": " << time.count() << " ms" << std::endl;
        }
    }

    void add_assumption_file_option(CLI::App *app, std::string &assumption_file) {
        app->add_option("-a,--assumption-file", assumption_file, "Assumption file")->required()->check(
                CLI::ExistingFile);
    }

    void add_gr1_file_option(CLI::App *app, std::string &gr1_file) {
        app->add_option("-g,--gr1-file", gr1_file, "GR(1) specification file")->required()->check(CLI::ExistingFile);
    }

    void add_env_safety_file_option(CLI::App *app, std::optional<std::string> &env_safety_file) {
        app->add_option("-e,--env-safety-file", env_safety_file,
                        "Formula file for the environment safety assumptions")->required()->check(CLI::ExistingFile);
    }

    void add_agent_safety_file_option(CLI::App *app, std::optional<std::string> &agent_safety_file) {
        app->add_option("-a,--agent-safety-file", agent_safety_file,
                        "Formula file for the agent safety assumptions")->required()->check(CLI::ExistingFile);
    }

    void add_spec_file_option(CLI::App *app, std::string &spec_file) {
        app->add_option("-f,--spec-file", spec_file, "Specification file")->required()->check(CLI::ExistingFile);
    }

    void add_syfco_option(CLI::App *app, std::optional<std::string> &path_to_syfco) {
        app->add_option("-s,--syfco-path", path_to_syfco, "Path to Syfco binary")
        ->check(CLI::ExistingFile);
    }

    void add_slugs_option(CLI::App *app, std::string &path_to_slugs) {
        app->add_option("-S,--slugs-path", path_to_slugs, "Path to Slugs root directory")
        ->check(CLI::ExistingDirectory);
    }

    bool BaseRunner::handle_preprocessing_result_(const OneStepSynthesisResult &one_step_result,
                                                  Stopwatch &total_time_stopwatch) const {
        bool preprocessing_success = one_step_result.realizability.has_value();
        if (preprocessing_success and one_step_result.realizability.value()) {
            CUDD::BDD move = one_step_result.winning_move;
            auto total_time = total_time_stopwatch.stop();
            printer_.print_realizable();
            printer_.dump_add_if_enabled(var_mgr_, move.Add(), "strategy.dot");
            printer_.print_times_if_enabled("Total time", total_time);
            return true;
        } else if (preprocessing_success and !one_step_result.realizability.value()) {
            auto total_time = total_time_stopwatch.stop();
            printer_.print_unrealizable();
            printer_.print_times_if_enabled("Total time", total_time);
            return true;
        } else {
            // Preprocessing was not successful. Continuing with full DFA construction."
        }
        return false;
    }

    Syft::SymbolicStateDfa BaseRunner::do_dfa_construction_() const {
        Stopwatch aut_time_stopwatch_;
        aut_time_stopwatch_.start();
        auto symbolic_dfa = do_dfa_construction(*args_.formula, var_mgr_);
        auto aut_time = aut_time_stopwatch_.stop();
        printer_.print_times_if_enabled("DFA construction time", aut_time);
        return symbolic_dfa;
    }

    void
    BaseRunner::handle_synthesis_result_(const DfaGameSynthesizer &synthesizer, const SynthesisResult &result) const {
        if (result.realizability) {
            printer_.print_realizable();

//             abstract single strategy
            Syft::Stopwatch abstract_single_strategy_time_stopwatch;
            abstract_single_strategy_time_stopwatch.start();
            auto transducer = synthesizer.AbstractSingleStrategy(result);
            auto abstract_single_strategy_time = abstract_single_strategy_time_stopwatch.stop();
            printer_.print_times_if_enabled("Abstract single strategy time", abstract_single_strategy_time);

            // dump strategy
            printer_.dump_transducer_if_enabled(*result.transducer, "strategy.dot");
        } else {
            printer_.print_unrealizable();
        }
    }


    void
    BaseRunner::handle_synthesis_result_(const SynthesisResult &result) const {
        if (result.realizability) {
            printer_.print_realizable();
            printer_.dump_transducer_if_enabled(*result.transducer, "strategy.dot");
        } else {
            printer_.print_unrealizable();
        }
    }

}

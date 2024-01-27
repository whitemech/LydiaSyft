//
// Created by marcofavorito on 23/01/24.
//

#include "base.hpp"
#include "lydia/parser/ltlf/driver.hpp"
#include "InputOutputPartition.h"
#include "Synthesizer.h"
#include "Preprocessing.h"

namespace Syft {

    void Printer::dump_transducer_if_enabled(const Transducer& transducer, const std::string& output_file) const {
        if (print_strategy_) {
            transducer.dump_dot(output_file);
        }
    }

    void Printer::dump_add_if_enabled(const std::shared_ptr<Syft::VarMgr>& var_mgr, const CUDD::ADD& add, const std::string &output_file) const {
        if (print_strategy_) {
            var_mgr->dump_dot(add, output_file);
        }
    }

    void Printer::dump_maxset_if_enabled(const ReachabilityMaxSetSynthesizer& maxset_synthesizer,  const MaxSet& maxset_strategy, const std::string& def_strategy_output_file, const std::string& nondef_strategy_output_file) const {
        if (print_strategy_) {
            maxset_synthesizer.dump_dot(maxset_strategy, def_strategy_output_file, nondef_strategy_output_file);
        }
    }

    void Printer::print_times_if_enabled(const std::string& message, std::chrono::milliseconds time) const {
        if (print_times_) {
            out_ << message << ": " << time.count() << " ms" << std::endl;
        }
    }

    void add_assumption_file_option(CLI::App* app, std::string& assumption_file) {
        app->add_option("-a,--assumption-file", assumption_file, "Assumption file")->required() -> check(CLI::ExistingFile);
    }

    void add_gr1_file_option(CLI::App* app, std::string& gr1_file) {
        app->add_option("-g,--gr1-file", gr1_file, "GR(1) specification file")->required() -> check(CLI::ExistingFile);
    }

    void add_env_safety_file_option(CLI::App* app, std::optional<std::string>& env_safety_file) {
        app->add_option("-e,--env-safety-file", env_safety_file, "Formula file for the environment safety assumptions")->required() -> check(CLI::ExistingFile);
    }

    void add_agent_safety_file_option(CLI::App* app, std::optional<std::string>& agent_safety_file) {
        app->add_option("-a,--agent-safety-file", agent_safety_file, "Formula file for the agent safety assumptions")->required() -> check(CLI::ExistingFile);
    }

    void add_spec_file_option(CLI::App* app, std::string& spec_file) {
        app->add_option("-f,--spec-file", spec_file, "Specification file")->required() -> check(CLI::ExistingFile);
    }

    void add_syfco_option(CLI::App* app, std::string& path_to_syfco) {
        app->add_option("-s,--syfco-path", path_to_syfco, "Path to Syfco binary") -> default_val(DEFAULT_SYFCO_PATH_) -> check(CLI::ExistingFile);
    }

    void add_slugs_option(CLI::App* app, std::string& path_to_slugs) {
        app->add_option("-S,--slugs-path", path_to_slugs, "Path to Slugs root directory") -> default_val(DEFAULT_SLUGS_PATH_) -> check(CLI::ExistingDirectory);
    }

    TLSFArgs parse_tlsf(const std::shared_ptr<whitemech::lydia::parsers::ltlf::LTLfDriver>& driver, const std::string& path_to_syfco, const std::string& formula_file) {
        Syft::Parser parser;
        // the parser assumes "syfco" is in the current working directory
        parser = Syft::Parser::read_from_file(path_to_syfco, formula_file);
        bool sys_first = parser.get_sys_first();

        // get starting player and protagonist player
        Syft::Player starting_player = sys_first? Syft::Player::Agent : Syft::Player::Environment;
        Syft::Player protagonist_player = Syft::Player::Agent;

        Syft::InputOutputPartition partition =
            Syft::InputOutputPartition::construct_from_input(parser.get_input_variables(), parser.get_output_variables());

        // Parsing the formula
        whitemech::lydia::ltlf_ptr parsed_formula = Syft::parse_formula(driver, parser.get_formula());

        return {starting_player, protagonist_player, partition, parsed_formula};
    }

    whitemech::lydia::ltlf_ptr parse_formula(const std::shared_ptr<whitemech::lydia::parsers::ltlf::LTLfDriver>& driver, const std::string& formula) {
        std::stringstream formula_stream(formula);
        driver->parse(formula_stream);
        whitemech::lydia::ltlf_ptr parsed_formula = driver->get_result();
        // Apply no-empty semantics
        auto context = driver->context;
        auto not_end = context->makeLtlfNotEnd();
        parsed_formula = context->makeLtlfAnd({parsed_formula, not_end});
        return parsed_formula;
    }

    std::shared_ptr<Syft::VarMgr> build_var_mgr(const InputOutputPartition& partition) {
        std::shared_ptr<Syft::VarMgr> var_mgr = std::make_shared<Syft::VarMgr>();
        var_mgr->create_named_variables(partition.input_variables);
        var_mgr->create_named_variables(partition.output_variables);
        return var_mgr;
    }

    SymbolicStateDfa do_dfa_construction(const whitemech::lydia::LTLfFormula &formula, const std::shared_ptr<Syft::VarMgr> &var_mgr) {
        ExplicitStateDfaMona explicit_dfa_mona = ExplicitStateDfaMona::dfa_of_formula(formula);
        ExplicitStateDfa explicit_dfa = ExplicitStateDfa::from_dfa_mona(var_mgr, explicit_dfa_mona);
        SymbolicStateDfa symbolic_dfa = SymbolicStateDfa::from_explicit(explicit_dfa);
        return symbolic_dfa;
    }

    bool BaseRunner::handle_preprocessing_result_(const OneStepSynthesisResult& one_step_result, Stopwatch& total_time_stopwatch) const {
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

    void BaseRunner::handle_synthesis_result_(const DfaGameSynthesizer& synthesizer, const SynthesisResult &result) const {
        if (result.realizability) {
            printer_.print_realizable();

            // abstract single strategy
            Syft::Stopwatch abstract_single_strategy_time_stopwatch;
            abstract_single_strategy_time_stopwatch.start();
            auto transducer = synthesizer.AbstractSingleStrategy(result);
            auto abstract_single_strategy_time = abstract_single_strategy_time_stopwatch.stop();
            printer_.print_times_if_enabled("Abstract single strategy time", abstract_single_strategy_time);

            // dump strategy
            printer_.dump_transducer_if_enabled(*transducer, "strategy.dot");
        } else {
            printer_.print_unrealizable();
        }
    }

}
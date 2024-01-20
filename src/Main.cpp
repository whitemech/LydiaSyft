#include <cstring>
#include <iostream>
#include <memory>

#include "Stopwatch.h"

#include "parser/Parser.h"
#include "ExplicitStateDfaMona.h"
#include "ReachabilitySynthesizer.h"
#include "ReachabilityMaxSetSynthesizer.h"
#include "InputOutputPartition.h"
#include "Preprocessing.h"
#include <lydia/parser/ltlf/driver.hpp>
#include <CLI/CLI.hpp>
#include <istream>


int main(int argc, char ** argv) {

    CLI::App app {
            "LydiaSyft: A compositional synthesizer for Linear Temporal Logic on finite traces (LTLf)"
    };

    std::string formula_file;

    app.add_option("-f,--spec-file", formula_file, "Specification file")->
                    required() -> check(CLI::ExistingFile);

    bool print_strategy = false;
    app.add_flag("-p, --print-strategy", print_strategy, "Print out the synthesized strategy (default: false)");

    bool print_times = false;
    app.add_flag("-t, --print-times", print_times, "Print out running times of each step (default: false)");

    bool maxset = false;
    app.add_flag("-m,--maxset", maxset, "Maxset flag (Default: false)");

    CLI11_PARSE(app, argc, argv);
    Syft::Stopwatch total_time_stopwatch; // stopwatch for end-to-end execution
    total_time_stopwatch.start();

    Syft::Stopwatch aut_time_stopwatch; // stopwatch for abstract single strategy
    aut_time_stopwatch.start();

    Syft::Parser parser;
    // the parser assumes "syfco" is in the current working directory
    parser = Syft::Parser::read_from_file("./syfco", formula_file);
    bool sys_first = parser.get_sys_first();

    Syft::Player starting_player = sys_first? Syft::Player::Agent : Syft::Player::Environment;
    Syft::Player protagonist_player = Syft::Player::Agent;
    bool realizability;

    Syft::InputOutputPartition partition =
            Syft::InputOutputPartition::construct_from_input(parser.get_input_variables(), parser.get_output_variables());
    std::shared_ptr<Syft::VarMgr> var_mgr = std::make_shared<Syft::VarMgr>();
    var_mgr->create_named_variables(partition.input_variables);
    var_mgr->create_named_variables(partition.output_variables);

    // Parsing the formula
    std::shared_ptr<whitemech::lydia::parsers::ltlf::LTLfDriver> driver;
    driver = std::make_shared<whitemech::lydia::parsers::ltlf::LTLfDriver>();
    std::stringstream formula_stream(parser.get_formula());
    driver->parse(formula_stream);
    whitemech::lydia::ltlf_ptr parsed_formula = driver->get_result();
    // Apply no-empty semantics
    auto context = driver->context;
    auto not_end = context->makeLtlfNotEnd();
    parsed_formula = context->makeLtlfAnd({parsed_formula, not_end});

    if (!maxset) {
        // abstract single strategy
        // preprocessing
//        auto one_step_result = preprocessing(*parsed_formula, partition, *var_mgr);
//        bool preprocessing_success = one_step_result.realizability.has_value();
//        if (preprocessing_success and one_step_result.realizability.value()) {
//            std::cout << Syft::REALIZABLE_STR << std::endl;
//
//            CUDD::BDD move = one_step_result.winning_move;
//            auto total_time = total_time_stopwatch.stop();
//            if (print_strategy) {
//                var_mgr->dump_dot(move.Add(), "strategy.dot");
//            }
//
//            if (print_times) {
//                std::cout << "Total time: "
//                          << total_time.count() << " ms" << std::endl;
//            }
//            return 0;
//        } else if (preprocessing_success and !one_step_result.realizability.value()) {
//            std::cout << Syft::UNREALIZABLE_STR << std::endl;
//            auto total_time = total_time_stopwatch.stop();
//
//            if (print_times) {
//                std::cout << "Total time: "
//                          << total_time.count() << " ms" << std::endl;
//            }
//            return 0;
//        } else {
//            // Preprocessing was not successful. Continuing with full DFA construction."
//        }

        Syft::ExplicitStateDfaMona explicit_dfa_mona = Syft::ExplicitStateDfaMona::dfa_of_formula(*parsed_formula);

        Syft::ExplicitStateDfa explicit_dfa = Syft::ExplicitStateDfa::from_dfa_mona(var_mgr, explicit_dfa_mona);

        Syft::SymbolicStateDfa symbolic_dfa = Syft::SymbolicStateDfa::from_explicit(
                std::move(explicit_dfa));

        auto aut_time = aut_time_stopwatch.stop();
        if (print_times) {
            std::cout << "DFA construction time: "
                      << aut_time.count() << " ms" << std::endl;
        }

        var_mgr->partition_variables(partition.input_variables,
                                     partition.output_variables);

        Syft::ReachabilitySynthesizer synthesizer(symbolic_dfa, starting_player,
                                                  protagonist_player, symbolic_dfa.final_states(),
                                                  var_mgr->cudd_mgr()->bddOne());
        Syft::SynthesisResult result = synthesizer.run();

        realizability = result.realizability;
        if (realizability == true) {
            std::cout << Syft::REALIZABLE_STR << std::endl;

            Syft::Stopwatch abstract_single_strategy_time_stopwatch; // stopwatch for abstract single strategy
            abstract_single_strategy_time_stopwatch.start();

            auto transducer = synthesizer.AbstractSingleStrategy(std::move(result));
            if (print_strategy) {
                transducer->dump_dot("strategy.dot");
            }
            auto abstract_single_strategy_time = abstract_single_strategy_time_stopwatch.stop();
            if (print_times) {
                std::cout << "Abstract single strategy time: "
                          << abstract_single_strategy_time.count() << " ms" << std::endl;
            }
        } else {
            std::cout << Syft::UNREALIZABLE_STR << std::endl;
        }
    } else {
        // abstract max strategy
        Syft::ExplicitStateDfaMona explicit_dfa_mona = Syft::ExplicitStateDfaMona::dfa_of_formula(*parsed_formula);
        Syft::ExplicitStateDfa explicit_dfa =  Syft::ExplicitStateDfa::from_dfa_mona(var_mgr, explicit_dfa_mona);

        Syft::SymbolicStateDfa symbolic_dfa = Syft::SymbolicStateDfa::from_explicit(
                std::move(explicit_dfa));

        auto aut_time = aut_time_stopwatch.stop();
        if (print_times) {
            std::cout << "DFA construction time: "
                      << aut_time.count() << " ms" << std::endl;
        }

        Syft::Stopwatch nondef_strategy_time_stopwatch; // stopwatch for strategy_generator construction
        nondef_strategy_time_stopwatch.start();

        var_mgr->partition_variables(partition.input_variables,
                                     partition.output_variables);

        Syft::ReachabilityMaxSetSynthesizer synthesizer(symbolic_dfa, starting_player,
                                                        protagonist_player, symbolic_dfa.final_states(),
                                                        var_mgr->cudd_mgr()->bddOne());
        Syft::SynthesisResult result = synthesizer.run();

        realizability = result.realizability;
        if (realizability == true) {

            auto nondef_strategy_time = nondef_strategy_time_stopwatch.stop();
            if (print_times) {
                std::cout << "Nondeferring strategy generator construction time: "
                          << nondef_strategy_time.count() << " ms" << std::endl;
            }

            Syft::Stopwatch def_strategy_time_stopwatch; // stopwatch for abstract max-deferring strategy
            def_strategy_time_stopwatch.start();

            Syft::MaxSet maxset_strategy = synthesizer.AbstractMaxSet(std::move(result));

            auto def_strategy_time = def_strategy_time_stopwatch.stop();
            if (print_times) {
                std::cout << "Deferring strategy generator construction time: "
                          << def_strategy_time.count() << " ms" << std::endl;
            }
            std::cout << Syft::REALIZABLE_STR << std::endl;
            if (print_strategy) {
                synthesizer.dump_dot(maxset_strategy, "def_strategy.dot", "nondef_strategy.dot");
            }
        }
        else{
        std::cout << Syft::UNREALIZABLE_STR << std::endl;
        }
    }

  auto total_time = total_time_stopwatch.stop();

  if (print_times) {
    std::cout << "Total time: "
              << total_time.count() << " ms" << std::endl;
  }

  return 0;
}


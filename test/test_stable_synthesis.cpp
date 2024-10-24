//
// Created by shuzhu on 21/01/24.
//
#include "catch2/catch_test_macros.hpp"
#include "catch2/generators/catch_generators_all.hpp"

#include "automata/ExplicitStateDfa.h"
#include "game/InputOutputPartition.h"
#include "synthesizer/StabilityLtlfSynthesizer.h"
#include "Preprocessing.h"
#include "Parser.h"
#include "utils.hpp"
#include <lydia/parser/ltlf/driver.hpp>


TEST_CASE("Stable Synthesis of realizable counter_1", "[stable synthesis]") {
    Syft::Parser parser;
    parser = Syft::Parser::read_from_file(Syft::Test::SYFCO_LOCATION,
                                          Syft::Test::FAIRSTABLESYNTHESIS_COUNTER_TEST_TLSF);

    bool sys_first = parser.get_sys_first();

    Syft::Player starting_player = sys_first ? Syft::Player::Agent : Syft::Player::Environment;
    Syft::Player protagonist_player = Syft::Player::Agent;

    Syft::InputOutputPartition partition =
            Syft::InputOutputPartition::construct_from_input(parser.get_input_variables(),
                                                             parser.get_output_variables());
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

    Syft::ExplicitStateDfa explicit_dfa = Syft::ExplicitStateDfa::dfa_of_formula(*parsed_formula);

    Syft::ExplicitStateDfaAdd explicit_dfa_add = Syft::ExplicitStateDfaAdd::from_dfa_mona(var_mgr, explicit_dfa);

    Syft::SymbolicStateDfa symbolic_dfa = Syft::SymbolicStateDfa::from_explicit(
            std::move(explicit_dfa_add));

    var_mgr->partition_variables(partition.input_variables,
                                 partition.output_variables);

    Syft::StabilityLtlfSynthesizer synthesizer(symbolic_dfa, starting_player,
                                               protagonist_player, symbolic_dfa.final_states(),
                                               var_mgr->cudd_mgr()->bddOne(),
                                               Syft::Test::FAIRSTABLESYNTHESIS_TEST_ASSUMPTION);
    Syft::SynthesisResult result = synthesizer.run();

    bool expected = true;
    REQUIRE(result.realizability == expected);
}

TEST_CASE("Stable Synthesis of unrealizable counter_1", "[stable synthesis]") {
    Syft::Parser parser;
    parser = Syft::Parser::read_from_file(Syft::Test::SYFCO_LOCATION,
                                          Syft::Test::FAIRSTABLESYNTHESIS_COUNTER_UNREA_TEST_TLSF);

    bool sys_first = parser.get_sys_first();

    Syft::Player starting_player = sys_first ? Syft::Player::Agent : Syft::Player::Environment;
    Syft::Player protagonist_player = Syft::Player::Agent;

    Syft::InputOutputPartition partition =
            Syft::InputOutputPartition::construct_from_input(parser.get_input_variables(),
                                                             parser.get_output_variables());
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

    Syft::ExplicitStateDfa explicit_dfa = Syft::ExplicitStateDfa::dfa_of_formula(*parsed_formula);

    Syft::ExplicitStateDfaAdd explicit_dfa_add = Syft::ExplicitStateDfaAdd::from_dfa_mona(var_mgr, explicit_dfa);
//    explicit_dfa.dump_dot("explicit_dfa_add.dot");

    Syft::SymbolicStateDfa symbolic_dfa = Syft::SymbolicStateDfa::from_explicit(
            std::move(explicit_dfa_add));

//    symbolic_dfa.dump_dot("symbolic_dfa.dot");
    var_mgr->partition_variables(partition.input_variables,
                                 partition.output_variables);

    Syft::StabilityLtlfSynthesizer synthesizer(symbolic_dfa, starting_player,
                                               protagonist_player, symbolic_dfa.final_states(),
                                               var_mgr->cudd_mgr()->bddOne(),
                                               Syft::Test::FAIRSTABLESYNTHESIS_TEST_ASSUMPTION);
    Syft::SynthesisResult result = synthesizer.run();

    bool expected = false;
    REQUIRE(result.realizability == expected);
}

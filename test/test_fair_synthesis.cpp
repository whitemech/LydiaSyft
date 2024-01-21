//
// Created by shuzhu on 21/01/24.
//
#include "catch2/catch_test_macros.hpp"
#include "catch2/generators/catch_generators_all.hpp"

#include "ExplicitStateDfaMona.h"
#include "InputOutputPartition.h"
#include "FairReachabilitySynthesizer.h"
#include "Preprocessing.h"
#include "Parser.h"
#include "utils.hpp"
#include <lydia/parser/ltlf/driver.hpp>

TEST_CASE("Fair Synthesis test", "[fair synthesis]") {
    Syft::Parser parser;
    parser = Syft::Parser::read_from_file(Syft::Test::SYFCO_LOCATION, Syft::Test::FAIRSYNTHESIS_TEST_TLSF);

    bool sys_first = parser.get_sys_first();

    Syft::Player starting_player = sys_first? Syft::Player::Agent : Syft::Player::Environment;
    Syft::Player protagonist_player = Syft::Player::Agent;

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

    Syft::ExplicitStateDfaMona explicit_dfa_mona = Syft::ExplicitStateDfaMona::dfa_of_formula(*parsed_formula);

    Syft::ExplicitStateDfa explicit_dfa = Syft::ExplicitStateDfa::from_dfa_mona(var_mgr, explicit_dfa_mona);
    explicit_dfa.dump_dot("explicit_dfa.dot");

    Syft::SymbolicStateDfa symbolic_dfa = Syft::SymbolicStateDfa::from_explicit(
            std::move(explicit_dfa));

    symbolic_dfa.dump_dot("symbolic_dfa.dot");
    var_mgr->partition_variables(partition.input_variables,
                                 partition.output_variables);

    Syft::FairReachabilitySynthesizer synthesizer(symbolic_dfa, starting_player,
                                                  protagonist_player, symbolic_dfa.final_states(),
                                                  var_mgr->cudd_mgr()->bddOne(), Syft::Test::FAIRSYNTHESIS_TEST_ASSUMPTION);
    Syft::SynthesisResult result = synthesizer.run();

    bool expected = true;
    REQUIRE(result.realizability == expected);
}

TEST_CASE("Fair Synthesis unrealizability test", "[fair synthesis]") {
    Syft::Parser parser;
    parser = Syft::Parser::read_from_file(Syft::Test::SYFCO_LOCATION, Syft::Test::FAIRSYNTHESIS_UNREA_TEST_TLSF);

    bool sys_first = parser.get_sys_first();

    Syft::Player starting_player = sys_first? Syft::Player::Agent : Syft::Player::Environment;
    Syft::Player protagonist_player = Syft::Player::Agent;

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

    Syft::ExplicitStateDfaMona explicit_dfa_mona = Syft::ExplicitStateDfaMona::dfa_of_formula(*parsed_formula);

    Syft::ExplicitStateDfa explicit_dfa = Syft::ExplicitStateDfa::from_dfa_mona(var_mgr, explicit_dfa_mona);
    explicit_dfa.dump_dot("explicit_dfa.dot");

    Syft::SymbolicStateDfa symbolic_dfa = Syft::SymbolicStateDfa::from_explicit(
            std::move(explicit_dfa));

    symbolic_dfa.dump_dot("symbolic_dfa.dot");
    var_mgr->partition_variables(partition.input_variables,
                                 partition.output_variables);

    Syft::FairReachabilitySynthesizer synthesizer(symbolic_dfa, starting_player,
                                                  protagonist_player, symbolic_dfa.final_states(),
                                                  var_mgr->cudd_mgr()->bddOne(), Syft::Test::FAIRSYNTHESIS_TEST_ASSUMPTION);
    Syft::SynthesisResult result = synthesizer.run();

    bool expected = false;
    REQUIRE(result.realizability == expected);
}

TEST_CASE("Fair Synthesis of realizable counter_1", "[fair synthesis]") {
    Syft::Parser parser;
    parser = Syft::Parser::read_from_file(Syft::Test::SYFCO_LOCATION, Syft::Test::FAIRSYNTHESIS_COUNTER_TEST_TLSF);

    bool sys_first = parser.get_sys_first();

    Syft::Player starting_player = sys_first? Syft::Player::Agent : Syft::Player::Environment;
    Syft::Player protagonist_player = Syft::Player::Agent;

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

    Syft::ExplicitStateDfaMona explicit_dfa_mona = Syft::ExplicitStateDfaMona::dfa_of_formula(*parsed_formula);

    Syft::ExplicitStateDfa explicit_dfa = Syft::ExplicitStateDfa::from_dfa_mona(var_mgr, explicit_dfa_mona);

    Syft::SymbolicStateDfa symbolic_dfa = Syft::SymbolicStateDfa::from_explicit(
            std::move(explicit_dfa));

    var_mgr->partition_variables(partition.input_variables,
                                 partition.output_variables);

    Syft::FairReachabilitySynthesizer synthesizer(symbolic_dfa, starting_player,
                                              protagonist_player, symbolic_dfa.final_states(),
                                              var_mgr->cudd_mgr()->bddOne(), Syft::Test::FAIRSYNTHESIS_TEST_ASSUMPTION);
    Syft::SynthesisResult result = synthesizer.run();

    bool expected = true;
    REQUIRE(result.realizability == expected);
}

TEST_CASE("Fair Synthesis of unrealizable counter_1", "[fair synthesis]") {
    Syft::Parser parser;
    parser = Syft::Parser::read_from_file(Syft::Test::SYFCO_LOCATION, Syft::Test::FAIRSYNTHESIS_COUNTER_UNREA_TEST_TLSF);

    bool sys_first = parser.get_sys_first();

    Syft::Player starting_player = sys_first? Syft::Player::Agent : Syft::Player::Environment;
    Syft::Player protagonist_player = Syft::Player::Agent;

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

    Syft::ExplicitStateDfaMona explicit_dfa_mona = Syft::ExplicitStateDfaMona::dfa_of_formula(*parsed_formula);

    Syft::ExplicitStateDfa explicit_dfa = Syft::ExplicitStateDfa::from_dfa_mona(var_mgr, explicit_dfa_mona);
//    explicit_dfa.dump_dot("explicit_dfa.dot");

    Syft::SymbolicStateDfa symbolic_dfa = Syft::SymbolicStateDfa::from_explicit(
            std::move(explicit_dfa));

//    symbolic_dfa.dump_dot("symbolic_dfa.dot");
    var_mgr->partition_variables(partition.input_variables,
                                 partition.output_variables);

    Syft::FairReachabilitySynthesizer synthesizer(symbolic_dfa, starting_player,
                                                  protagonist_player, symbolic_dfa.final_states(),
                                                  var_mgr->cudd_mgr()->bddOne(), Syft::Test::FAIRSYNTHESIS_TEST_ASSUMPTION);
    Syft::SynthesisResult result = synthesizer.run();

    bool expected = false;
    REQUIRE(result.realizability == expected);
}

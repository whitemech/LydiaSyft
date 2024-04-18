//
// Created by shuzhu on 21/01/24.
//
#include "catch2/catch_test_macros.hpp"
#include "catch2/generators/catch_generators_all.hpp"

#include "ExplicitStateDfa.h"
#include "InputOutputPartition.h"
#include "GR1ReachabilitySynthesizer.h"
#include "GR1.h"
#include "Preprocessing.h"
#include "Parser.h"
#include "utils.hpp"
#include <lydia/parser/ltlf/driver.hpp>

TEST_CASE("GR1 Synthesis test", "[gr1 synthesis]") {
    Syft::Parser parser;
    parser = Syft::Parser::read_from_file(Syft::Test::SYFCO_LOCATION, Syft::Test::GR1SYNTHESIS_TEST_AGN_GOAL);

    bool sys_first = parser.get_sys_first();

    Syft::Player starting_player = sys_first? Syft::Player::Agent : Syft::Player::Environment;
    Syft::Player protagonist_player = Syft::Player::Agent;

    Syft::InputOutputPartition partition =
            Syft::InputOutputPartition::construct_from_input(parser.get_input_variables(), parser.get_output_variables());
    std::shared_ptr<Syft::VarMgr> var_mgr = std::make_shared<Syft::VarMgr>();
    var_mgr->create_named_variables(partition.input_variables);
    var_mgr->create_named_variables(partition.output_variables);

    var_mgr->partition_variables(partition.input_variables,
                                 partition.output_variables);

    // Parsing the formula agn_goal
    std::shared_ptr<whitemech::lydia::parsers::ltlf::LTLfDriver> driver_agn_goal;
    driver_agn_goal = std::make_shared<whitemech::lydia::parsers::ltlf::LTLfDriver>();
    std::stringstream formula_stream(parser.get_formula());
    driver_agn_goal->parse(formula_stream);
    whitemech::lydia::ltlf_ptr parsed_formula_agn_goal = driver_agn_goal->get_result();
    // Apply no-empty semantics
    auto context = driver_agn_goal->context;
    auto not_end = context->makeLtlfNotEnd();
    parsed_formula_agn_goal = context->makeLtlfAnd({parsed_formula_agn_goal, not_end});

    Syft::ExplicitStateDfa explicit_dfa_agn_goal = Syft::ExplicitStateDfa::dfa_of_formula(*parsed_formula_agn_goal);

    Syft::ExplicitStateDfaAdd explicit_dfa_add_agn_goal = Syft::ExplicitStateDfaAdd::from_dfa_mona(var_mgr, explicit_dfa_agn_goal);


    Syft::SymbolicStateDfa symbolic_dfa_agn_goal = Syft::SymbolicStateDfa::from_explicit(
            std::move(explicit_dfa_add_agn_goal));

    // Parsing the formula env_safety
    std::shared_ptr<whitemech::lydia::parsers::ltlf::LTLfDriver> driver_env_safety;
    driver_env_safety = std::make_shared<whitemech::lydia::parsers::ltlf::LTLfDriver>();
    std::ifstream in_env_safety(Syft::Test::GR1SYNTHESIS_TEST_ENV_SAFETY);
    std::string env_safety;
    std::getline(in_env_safety, env_safety);
    std::stringstream formula_stream_env_safety(env_safety);
    driver_env_safety->parse(formula_stream_env_safety);
    whitemech::lydia::ltlf_ptr parsed_formula_env_safety = driver_env_safety->get_result();
    // Apply no-empty semantics
    parsed_formula_env_safety = context->makeLtlfAnd({parsed_formula_env_safety, not_end});

    Syft::ExplicitStateDfa explicit_dfa_env_safety = Syft::ExplicitStateDfa::dfa_of_formula(*parsed_formula_env_safety);

    Syft::ExplicitStateDfaAdd explicit_dfa_add_env_safety = Syft::ExplicitStateDfaAdd::from_dfa_mona(var_mgr, explicit_dfa_env_safety);


    Syft::SymbolicStateDfa symbolic_dfa_env_safety = Syft::SymbolicStateDfa::from_explicit(
            std::move(explicit_dfa_add_env_safety));

    // Parsing the formula agn_safety
    std::shared_ptr<whitemech::lydia::parsers::ltlf::LTLfDriver> driver_agn_safetyy;
    driver_env_safety = std::make_shared<whitemech::lydia::parsers::ltlf::LTLfDriver>();
    std::ifstream in_agn_safety(Syft::Test::GR1SYNTHESIS_TEST_AGN_SAFETY);
    std::string agn_safety;
    std::getline(in_agn_safety, agn_safety);
    std::stringstream formula_stream_agn_safety(agn_safety);
    driver_env_safety->parse(formula_stream_agn_safety);
    whitemech::lydia::ltlf_ptr parsed_formula_agn_safety = driver_env_safety->get_result();
    // Apply no-empty semantics
    parsed_formula_agn_safety = context->makeLtlfAnd({parsed_formula_agn_safety, not_end});

    Syft::ExplicitStateDfa explicit_dfa_agn_safety = Syft::ExplicitStateDfa::dfa_of_formula(*parsed_formula_agn_safety);

    Syft::ExplicitStateDfaAdd explicit_dfa_add_agn_safety = Syft::ExplicitStateDfaAdd::from_dfa_mona(var_mgr, explicit_dfa_agn_safety);

    Syft::SymbolicStateDfa symbolic_dfa_agn_safety = Syft::SymbolicStateDfa::from_explicit(
            std::move(explicit_dfa_add_agn_safety));

    // gr1
    Syft::GR1 gr1 = Syft::GR1::read_from_gr1_file(var_mgr, Syft::Test::GR1SYNTHESIS_TEST_ENV_GR1);



    std::string benchmark_name = "hand_shake";
    Syft::GR1ReachabilitySynthesizer synthesizer(var_mgr, gr1, symbolic_dfa_env_safety,
                                         symbolic_dfa_agn_goal, symbolic_dfa_agn_safety, Syft::Test::SLUGS_DIR_LOCATION, benchmark_name);

    Syft::SynthesisResult result = synthesizer.run();

    bool expected = true;
    REQUIRE(result.realizability == expected);
}


TEST_CASE("GR1 Synthesis finding_nemo", "[gr1 synthesis]") {
    Syft::Parser parser;
    parser = Syft::Parser::read_from_file(Syft::Test::SYFCO_LOCATION, Syft::Test::GR1SYNTHESIS_FINDING_NEMO_AGN_GOAL);

    bool sys_first = parser.get_sys_first();

    Syft::Player starting_player = sys_first? Syft::Player::Agent : Syft::Player::Environment;
    Syft::Player protagonist_player = Syft::Player::Agent;

    Syft::InputOutputPartition partition =
            Syft::InputOutputPartition::construct_from_input(parser.get_input_variables(), parser.get_output_variables());
    std::shared_ptr<Syft::VarMgr> var_mgr = std::make_shared<Syft::VarMgr>();
    var_mgr->create_named_variables(partition.input_variables);
    var_mgr->create_named_variables(partition.output_variables);

    var_mgr->partition_variables(partition.input_variables,
                                 partition.output_variables);

    // Parsing the formula agn_goal
    std::shared_ptr<whitemech::lydia::parsers::ltlf::LTLfDriver> driver_agn_goal;
    driver_agn_goal = std::make_shared<whitemech::lydia::parsers::ltlf::LTLfDriver>();
    std::stringstream formula_stream(parser.get_formula());
    driver_agn_goal->parse(formula_stream);
    whitemech::lydia::ltlf_ptr parsed_formula_agn_goal = driver_agn_goal->get_result();
    // Apply no-empty semantics
    auto context = driver_agn_goal->context;
    auto not_end = context->makeLtlfNotEnd();
    parsed_formula_agn_goal = context->makeLtlfAnd({parsed_formula_agn_goal, not_end});

    Syft::ExplicitStateDfa explicit_dfa_agn_goal = Syft::ExplicitStateDfa::dfa_of_formula(*parsed_formula_agn_goal);

    Syft::ExplicitStateDfaAdd explicit_dfa_add_agn_goal = Syft::ExplicitStateDfaAdd::from_dfa_mona(var_mgr, explicit_dfa_agn_goal);


    Syft::SymbolicStateDfa symbolic_dfa_agn_goal = Syft::SymbolicStateDfa::from_explicit(
            std::move(explicit_dfa_add_agn_goal));

    // Parsing the formula env_safety
    std::shared_ptr<whitemech::lydia::parsers::ltlf::LTLfDriver> driver_env_safety;
    driver_env_safety = std::make_shared<whitemech::lydia::parsers::ltlf::LTLfDriver>();
    std::ifstream in_env_safety(Syft::Test::GR1SYNTHESIS_FINDING_NEMO_ENV_SAFETY);
    std::string env_safety;
    std::getline(in_env_safety, env_safety);
    std::stringstream formula_stream_env_safety(env_safety);
    driver_env_safety->parse(formula_stream_env_safety);
    whitemech::lydia::ltlf_ptr parsed_formula_env_safety = driver_env_safety->get_result();
    // Apply no-empty semantics
    parsed_formula_env_safety = context->makeLtlfAnd({parsed_formula_env_safety, not_end});

    Syft::ExplicitStateDfa explicit_dfa_mona_env_safety = Syft::ExplicitStateDfa::dfa_of_formula(*parsed_formula_env_safety);

    Syft::ExplicitStateDfaAdd explicit_dfa_env_safety = Syft::ExplicitStateDfaAdd::from_dfa_mona(var_mgr, explicit_dfa_mona_env_safety);


    Syft::SymbolicStateDfa symbolic_dfa_env_safety = Syft::SymbolicStateDfa::from_explicit(
            std::move(explicit_dfa_env_safety));

    // Parsing the formula agn_safety
    std::shared_ptr<whitemech::lydia::parsers::ltlf::LTLfDriver> driver_agn_safetyy;
    driver_env_safety = std::make_shared<whitemech::lydia::parsers::ltlf::LTLfDriver>();
    std::ifstream in_agn_safety(Syft::Test::GR1SYNTHESIS_FINDING_NEMO_AGN_SAFETY);
    std::string agn_safety;
    std::getline(in_agn_safety, agn_safety);
    std::stringstream formula_stream_agn_safety(agn_safety);
    driver_env_safety->parse(formula_stream_agn_safety);
    whitemech::lydia::ltlf_ptr parsed_formula_agn_safety = driver_env_safety->get_result();
    // Apply no-empty semantics
    parsed_formula_agn_safety = context->makeLtlfAnd({parsed_formula_agn_safety, not_end});

    Syft::ExplicitStateDfa explicit_dfa_agn_safety = Syft::ExplicitStateDfa::dfa_of_formula(*parsed_formula_agn_safety);

    Syft::ExplicitStateDfaAdd explicit_dfa_add_agn_safety = Syft::ExplicitStateDfaAdd::from_dfa_mona(var_mgr, explicit_dfa_agn_safety);

    Syft::SymbolicStateDfa symbolic_dfa_agn_safety = Syft::SymbolicStateDfa::from_explicit(
            std::move(explicit_dfa_add_agn_safety));

    // gr1
    Syft::GR1 gr1 = Syft::GR1::read_from_gr1_file(var_mgr, Syft::Test::GR1SYNTHESIS_FINDING_NEMO_ENV_GR1);



    std::string benchmark_name = "finding_nemo";
    Syft::GR1ReachabilitySynthesizer synthesizer(var_mgr, gr1, symbolic_dfa_env_safety,
                                                 symbolic_dfa_agn_goal, symbolic_dfa_agn_safety, Syft::Test::SLUGS_DIR_LOCATION, benchmark_name);

    Syft::SynthesisResult result = synthesizer.run();

    bool expected = true;
    REQUIRE(result.realizability == expected);
}




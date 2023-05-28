#include <catch2/catch_test_macros.hpp>

#include <tuple>

#include "VarMgr.h"
#include "utils.hpp"
#include "ExplicitStateDfaMona.h"
#include "ExplicitStateDfa.h"
#include "catch2/generators/catch_generators_all.hpp"
#include "SymbolicStateDfa.h"
#include "ReachabilitySynthesizer.h"
#include "InputOutputPartition.h"


TEST_CASE( "Synthesis", "[synthesis]" ) {

  typedef std::tuple<std::string, std::string, std::vector<std::string>, std::vector<std::string>, bool> task_t;
  std::string name;
  std::string formula;
  std::vector<std::string> input_variables;
  std::vector<std::string> output_variables;
  bool expected_realizability;
  std::tie(name, formula, input_variables, output_variables, expected_realizability) =
      GENERATE(table<std::string, std::string, std::vector<std::string>, std::vector<std::string>, bool>(
          {
              std::make_tuple("true", "true", std::vector<std::string>{}, std::vector<std::string>{}, true),
              std::make_tuple("false", "false", std::vector<std::string>{}, std::vector<std::string>{}, false),
              std::make_tuple("a, controllable",   "a", std::vector<std::string>{}, std::vector<std::string>{"a"}, true),
              std::make_tuple("a, uncontrollable", "a", std::vector<std::string>{"a"}, std::vector<std::string>{}, false),
          }));

  SECTION("Run synthesis task " + name) {
    Syft::InputOutputPartition partition = Syft::InputOutputPartition::construct_from_input(input_variables, output_variables);
    std::shared_ptr<Syft::VarMgr> var_mgr = std::make_shared<Syft::VarMgr>();
    var_mgr->create_named_variables(partition.input_variables);
    var_mgr->create_named_variables(partition.output_variables);
    Syft::ExplicitStateDfaMona explicit_dfa_mona = Syft::ExplicitStateDfaMona::dfa_of_formula(formula);
    Syft::ExplicitStateDfa explicit_dfa =  Syft::ExplicitStateDfa::from_dfa_mona(var_mgr, explicit_dfa_mona);

    Syft::SymbolicStateDfa symbolic_dfa = Syft::SymbolicStateDfa::from_explicit(
        std::move(explicit_dfa));
    var_mgr->partition_variables(partition.input_variables,
                                 partition.output_variables);

    Syft::Player starting_player = Syft::Player::Agent;
    Syft::Player protagonist_player = Syft::Player::Agent;
    Syft::ReachabilitySynthesizer synthesizer(symbolic_dfa, starting_player,
                                              protagonist_player, symbolic_dfa.final_states(),
                                              var_mgr->cudd_mgr()->bddOne());
    Syft::SynthesisResult result = synthesizer.run();

    bool actual_realizability = result.realizability;

    REQUIRE(actual_realizability == expected_realizability);

  }

}

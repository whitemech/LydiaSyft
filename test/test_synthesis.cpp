#include <catch2/catch_test_macros.hpp>

#include <tuple>
#include <sstream>

#include "VarMgr.h"
#include "utils.hpp"
#include "ExplicitStateDfaMona.h"
#include "ExplicitStateDfa.h"
#include "catch2/generators/catch_generators_all.hpp"
#include "SymbolicStateDfa.h"
#include "ReachabilitySynthesizer.h"
#include "InputOutputPartition.h"
#include "lydia/parser/ltlf/driver.hpp"



bool get_realizability(const std::string& formula, std::vector<std::string> input_variables, std::vector<std::string> output_variables) {
  auto driver = std::make_shared<whitemech::lydia::parsers::ltlf::LTLfDriver>();
  auto parsed_formula = Syft::Test::parse_formula(formula, *driver);

  Syft::InputOutputPartition partition = Syft::InputOutputPartition::construct_from_input(input_variables, output_variables);
  std::shared_ptr<Syft::VarMgr> var_mgr = std::make_shared<Syft::VarMgr>();
  var_mgr->create_named_variables(partition.input_variables);
  var_mgr->create_named_variables(partition.output_variables);
  Syft::ExplicitStateDfaMona explicit_dfa_mona = Syft::ExplicitStateDfaMona::dfa_of_formula(*parsed_formula);
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

  return result.realizability;
}

TEST_CASE("Synthesis of true", "[synthesis]") {
  bool expected = true;
  bool actual = get_realizability("true", vars{}, vars{});
  REQUIRE(actual == expected);
}

TEST_CASE("Synthesis of false", "[synthesis]") {
  bool expected = false;
  bool actual = get_realizability("false", vars{}, vars{});
  REQUIRE(actual == expected);
}

TEST_CASE("Synthesis of a", "[synthesis]") {

  SECTION("a controllable"){
    bool expected = true;
    bool actual = get_realizability("a", vars{}, vars{"a"});
    REQUIRE(actual == expected);
  }

  SECTION("a uncontrollable"){
    bool expected = false;
    bool actual = get_realizability("a", vars{"a"}, vars{});
    REQUIRE(actual == expected);
  }

}

TEST_CASE("Synthesis of a and b", "[synthesis]") {

  std::string formula = "a & b";

  SECTION("a controllable, b controllable"){
    bool expected = true;
    bool actual = get_realizability(formula, vars{}, vars{"a", "b"});
    REQUIRE(actual == expected);
  }
  SECTION("a controllable, b uncontrollable"){
    bool expected = false;
    bool actual = get_realizability(formula, vars{"b"}, vars{"a"});
    REQUIRE(actual == expected);
  }
  SECTION("a uncontrollable, b controllable"){
    bool expected = false;
    bool actual = get_realizability(formula, vars{"a"}, vars{"b"});
    REQUIRE(actual == expected);
  }
  SECTION("a uncontrollable, b uncontrollable"){
    bool expected = false;
    bool actual = get_realizability(formula, vars{"a", "b"}, vars{});
    REQUIRE(actual == expected);
  }

}

TEST_CASE("Synthesis of a or b", "[synthesis]") {

  std::string formula = "a | b";

  SECTION("a controllable, b controllable") {
    bool expected = true;
    bool actual = get_realizability(formula, vars{}, vars{"a", "b"});
    REQUIRE(actual == expected);
  }
  SECTION("a controllable, b uncontrollable") {
    bool expected = true;
    bool actual = get_realizability(formula, vars{"b"}, vars{"a"});
    REQUIRE(actual == expected);
  }
  SECTION("a uncontrollable, b controllable") {
    bool expected = true;
    bool actual = get_realizability(formula, vars{"a"}, vars{"b"});
    REQUIRE(actual == expected);
  }
  SECTION("a uncontrollable, b uncontrollable") {
    bool expected = false;
    bool actual = get_realizability(formula, vars{"a", "b"}, vars{});
    REQUIRE(actual == expected);
  }
}

TEST_CASE("Synthesis of X\\[\\!\\] a", "[synthesis]") {

  std::string formula = "X[!] a";

  SECTION("a controllable") {
    bool expected = true;
    bool actual = get_realizability(formula, vars{}, vars{"a"});
    REQUIRE(actual == expected);
  }
  SECTION("a uncontrollable") {
    bool expected = false;
    bool actual = get_realizability(formula, vars{"a"}, vars{});
    REQUIRE(actual == expected);
  }
}

TEST_CASE("Synthesis of X a", "[synthesis]") {

  std::string formula = "X a";

  SECTION("a controllable") {
    bool expected = true;
    bool actual = get_realizability(formula, vars{}, vars{"a"});
    REQUIRE(actual == expected);
  }
  SECTION("a uncontrollable") {
    bool expected = true;
    bool actual = get_realizability(formula, vars{"a"}, vars{});
    REQUIRE(actual == expected);
  }
}

TEST_CASE("Synthesis of a U b", "[synthesis]") {

  std::string formula = "a U b";

  SECTION("a controllable, b controllable") {
    bool expected = true;
    bool actual = get_realizability(formula, vars{}, vars{"a", "b"});
    REQUIRE(actual == expected);
  }
  SECTION("a controllable, b uncontrollable") {
    bool expected = false;
    bool actual = get_realizability(formula, vars{"b"}, vars{"a"});
    REQUIRE(actual == expected);
  }
  SECTION("a uncontrollable, b controllable") {
    bool expected = true;
    bool actual = get_realizability(formula, vars{"a"}, vars{"b"});
    REQUIRE(actual == expected);
  }
  SECTION("a uncontrollable, b uncontrollable") {
    bool expected = false;
    bool actual = get_realizability(formula, vars{"a", "b"}, vars{});
    REQUIRE(actual == expected);
  }
}

TEST_CASE("Synthesis of a R b", "[synthesis]") {

  std::string formula = "a R b";

  SECTION("a controllable, b controllable") {
    bool expected = true;
    bool actual = get_realizability(formula, vars{}, vars{"a", "b"});
    REQUIRE(actual == expected);
  }
  SECTION("a controllable, b uncontrollable") {
    bool expected = false;
    bool actual = get_realizability(formula, vars{"b"}, vars{"a"});
    REQUIRE(actual == expected);
  }
  SECTION("a uncontrollable, b controllable") {
    bool expected = true;
    bool actual = get_realizability(formula, vars{"a"}, vars{"b"});
    REQUIRE(actual == expected);
  }
  SECTION("a uncontrollable, b uncontrollable") {
    bool expected = false;
    bool actual = get_realizability(formula, vars{"a", "b"}, vars{});
    REQUIRE(actual == expected);
  }
}

TEST_CASE("Synthesis of G(a)", "[synthesis]") {

  std::string formula = "G(a)";

  SECTION("a controllable") {
    bool expected = true;
    bool actual = get_realizability(formula, vars{}, vars{"a"});
    REQUIRE(actual == expected);
  }
  SECTION("a uncontrollable") {
    bool expected = false;
    bool actual = get_realizability(formula, vars{"a"}, vars{});
    REQUIRE(actual == expected);
  }

}

TEST_CASE("Synthesis of F(a)", "[synthesis]") {

  std::string formula = "F(a)";

  SECTION("a controllable") {
    bool expected = true;
    bool actual = get_realizability(formula, vars{}, vars{"a"});
    REQUIRE(actual == expected);
  }
  SECTION("a uncontrollable") {
    bool expected = false;
    bool actual = get_realizability(formula, vars{"a"}, vars{});
    REQUIRE(actual == expected);
  }

}

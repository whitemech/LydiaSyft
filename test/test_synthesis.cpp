#include "catch2/catch_test_macros.hpp"
#include "catch2/generators/catch_generators_all.hpp"

#include <tuple>
#include "utils.hpp"
#include "game/InputOutputPartition.h"


TEST_CASE("Synthesis of true", "[synthesis]") {
  bool expected = true;
  bool actual = Syft::Test::get_realizability_from_input("true", vars{}, vars{});
  REQUIRE(actual == expected);
}

TEST_CASE("Synthesis of false", "[synthesis]") {
  bool expected = false;
  bool actual = Syft::Test::get_realizability_from_input("false", vars{}, vars{});
  REQUIRE(actual == expected);
}

TEST_CASE("Synthesis of a", "[synthesis]") {

  SECTION("a controllable"){
    bool expected = true;
    bool actual = Syft::Test::get_realizability_from_input("a", vars{}, vars{"a"});
    REQUIRE(actual == expected);
  }

  SECTION("a uncontrollable"){
    bool expected = false;
    bool actual = Syft::Test::get_realizability_from_input("a", vars{"a"}, vars{});
    REQUIRE(actual == expected);
  }

}

TEST_CASE("Synthesis of a and b", "[synthesis]") {

  std::string formula = "a & b";

  SECTION("a controllable, b controllable"){
    bool expected = true;
    bool actual = Syft::Test::get_realizability_from_input(formula, vars{}, vars{"a", "b"});
    REQUIRE(actual == expected);
  }
  SECTION("a controllable, b uncontrollable"){
    bool expected = false;
    bool actual = Syft::Test::get_realizability_from_input(formula, vars{"b"}, vars{"a"});
    REQUIRE(actual == expected);
  }
  SECTION("a uncontrollable, b controllable"){
    bool expected = false;
    bool actual = Syft::Test::get_realizability_from_input(formula, vars{"a"}, vars{"b"});
    REQUIRE(actual == expected);
  }
  SECTION("a uncontrollable, b uncontrollable"){
    bool expected = false;
    bool actual = Syft::Test::get_realizability_from_input(formula, vars{"a", "b"}, vars{});
    REQUIRE(actual == expected);
  }

}

TEST_CASE("Synthesis of a or b", "[synthesis]") {

  std::string formula = "a | b";

  SECTION("a controllable, b controllable") {
    bool expected = true;
    bool actual = Syft::Test::get_realizability_from_input(formula, vars{}, vars{"a", "b"});
    REQUIRE(actual == expected);
  }
  SECTION("a controllable, b uncontrollable") {
    bool expected = true;
    bool actual = Syft::Test::get_realizability_from_input(formula, vars{"b"}, vars{"a"});
    REQUIRE(actual == expected);
  }
  SECTION("a uncontrollable, b controllable") {
    bool expected = true;
    bool actual = Syft::Test::get_realizability_from_input(formula, vars{"a"}, vars{"b"});
    REQUIRE(actual == expected);
  }
  SECTION("a uncontrollable, b uncontrollable") {
    bool expected = false;
    bool actual = Syft::Test::get_realizability_from_input(formula, vars{"a", "b"}, vars{});
    REQUIRE(actual == expected);
  }
}

TEST_CASE("Synthesis of strongX a", "[synthesis]") {

  std::string formula = "X[!] a";

  SECTION("a controllable") {
    bool expected = true;
    bool actual = Syft::Test::get_realizability_from_input(formula, vars{}, vars{"a"});
    REQUIRE(actual == expected);
  }
  SECTION("a uncontrollable") {
    bool expected = false;
    bool actual = Syft::Test::get_realizability_from_input(formula, vars{"a"}, vars{});
    REQUIRE(actual == expected);
  }
}

TEST_CASE("Synthesis of X a", "[synthesis]") {

  std::string formula = "X a";

  SECTION("a controllable") {
    bool expected = true;
    bool actual = Syft::Test::get_realizability_from_input(formula, vars{}, vars{"a"});
    REQUIRE(actual == expected);
  }
  SECTION("a uncontrollable") {
    bool expected = true;
    bool actual = Syft::Test::get_realizability_from_input(formula, vars{"a"}, vars{});
    REQUIRE(actual == expected);
  }
}

TEST_CASE("Synthesis of a U b", "[synthesis]") {

  std::string formula = "a U b";

  SECTION("a controllable, b controllable") {
    bool expected = true;
    bool actual = Syft::Test::get_realizability_from_input(formula, vars{}, vars{"a", "b"});
    REQUIRE(actual == expected);
  }
  SECTION("a controllable, b uncontrollable") {
    bool expected = false;
    bool actual = Syft::Test::get_realizability_from_input(formula, vars{"b"}, vars{"a"});
    REQUIRE(actual == expected);
  }
  SECTION("a uncontrollable, b controllable") {
    bool expected = true;
    bool actual = Syft::Test::get_realizability_from_input(formula, vars{"a"}, vars{"b"});
    REQUIRE(actual == expected);
  }
  SECTION("a uncontrollable, b uncontrollable") {
    bool expected = false;
    bool actual = Syft::Test::get_realizability_from_input(formula, vars{"a", "b"}, vars{});
    REQUIRE(actual == expected);
  }
}

TEST_CASE("Synthesis of a R b", "[synthesis]") {

  std::string formula = "a R b";

  SECTION("a controllable, b controllable") {
    bool expected = true;
    bool actual = Syft::Test::get_realizability_from_input(formula, vars{}, vars{"a", "b"});
    REQUIRE(actual == expected);
  }
  SECTION("a controllable, b uncontrollable") {
    bool expected = false;
    bool actual = Syft::Test::get_realizability_from_input(formula, vars{"b"}, vars{"a"});
    REQUIRE(actual == expected);
  }
  SECTION("a uncontrollable, b controllable") {
    bool expected = true;
    bool actual = Syft::Test::get_realizability_from_input(formula, vars{"a"}, vars{"b"});
    REQUIRE(actual == expected);
  }
  SECTION("a uncontrollable, b uncontrollable") {
    bool expected = false;
    bool actual = Syft::Test::get_realizability_from_input(formula, vars{"a", "b"}, vars{});
    REQUIRE(actual == expected);
  }
}

TEST_CASE("Synthesis of G(a)", "[synthesis]") {

  std::string formula = "G(a)";

  SECTION("a controllable") {
    bool expected = true;
    bool actual = Syft::Test::get_realizability_from_input(formula, vars{}, vars{"a"});
    REQUIRE(actual == expected);
  }
  SECTION("a uncontrollable") {
    bool expected = false;
    bool actual = Syft::Test::get_realizability_from_input(formula, vars{"a"}, vars{});
    REQUIRE(actual == expected);
  }

}

TEST_CASE("Synthesis of F(a)", "[synthesis]") {

  std::string formula = "F(a)";

  SECTION("a controllable") {
    bool expected = true;
    bool actual = Syft::Test::get_realizability_from_input(formula, vars{}, vars{"a"});
    REQUIRE(actual == expected);
  }
  SECTION("a uncontrollable") {
    bool expected = false;
    bool actual = Syft::Test::get_realizability_from_input(formula, vars{"a"}, vars{});
    REQUIRE(actual == expected);
  }

}

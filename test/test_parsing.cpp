#include <catch2/catch_test_macros.hpp>

#include "Parser.h"
#include "utils.hpp"

#include <set>


TEST_CASE( "Parse TLSF", "[tlsf_parser]" ) {
  Syft::Parser parser;
  parser = Syft::Parser::read_from_file(Syft::Test::SYFCO_LOCATION, Syft::Test::EXAMPLE_TEST_TLSF);

  bool sys_first = parser.get_sys_first();
  REQUIRE(!sys_first);

  auto expected_input_vars = std::set<std::string>({"a1", "a2"});
  auto input_vars_vec = parser.get_input_variables();
  auto actual_input_vars = std::set<std::string>(input_vars_vec.begin(), input_vars_vec.end());
  REQUIRE(expected_input_vars == actual_input_vars);

  auto expected_output_vars = std::set<std::string>({"b1", "b2"});
  auto output_vars_vec = parser.get_output_variables();
  auto actual_output_vars = std::set<std::string>(output_vars_vec.begin(), output_vars_vec.end());
  REQUIRE(expected_output_vars == actual_output_vars);

  auto expected_formula = "((a1) <-> (b1))";
  auto actual_formula = parser.get_formula();
  REQUIRE(expected_formula == actual_formula);

}
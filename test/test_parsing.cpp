#include <catch2/catch_test_macros.hpp>

#include "Parser.h"
#include "utils.hpp"

#include <set>


TEST_CASE( "Parse TLSF, test.tlsf", "[tlsf_parser]" ) {
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

TEST_CASE( "Parse TLSF, 001.tlsf", "[tlsf_parser]" ) {
  Syft::Parser parser;
  parser = Syft::Parser::read_from_file(Syft::Test::SYFCO_LOCATION, Syft::Test::EXAMPLE_001_TLSF);

  bool sys_first = parser.get_sys_first();
  REQUIRE(sys_first);

  auto expected_input_vars = std::set<std::string>({"p1", "p2", "p3", "p4", "p5", "p6", "p7", "p8", "p9"});
  auto input_vars_vec = parser.get_input_variables();
  auto actual_input_vars = std::set<std::string>(input_vars_vec.begin(), input_vars_vec.end());
  REQUIRE(expected_input_vars == actual_input_vars);

  auto expected_output_vars = std::set<std::string>({"p10", "p11", "p12", "p13", "p14", "p15", "p16", "p17"});
  auto output_vars_vec = parser.get_output_variables();
  auto actual_output_vars = std::set<std::string>(output_vars_vec.begin(), output_vars_vec.end());
  REQUIRE(expected_output_vars == actual_output_vars);

  auto expected_formula = "((((G (((false) || (! ((((((((((true) && (p9)) && (! (X[!] (p10)))) && (! (X[!] (p11)))) && (! (X[!] (p12)))) && (! (X[!] (p13)))) && (! (X[!] (p14)))) && (! (X[!] (p15)))) && (! (X[!] (p16)))) && (! (X[!] (p17)))))) || (X[!] (p9)))) && (G (F (p9)))) && (true)) -> ((((((((((((((((((((((((((G (((false) || (! (X[!] (p10)))) || (p1))) && (G (((false) || (! (X[!] (p11)))) || (p2)))) && (G (((false) || (! (X[!] (p12)))) || (p3)))) && (G (((false) || (! (X[!] (p13)))) || (p4)))) && (G (((false) || (! (X[!] (p14)))) || (p5)))) && (G (((false) || (! (X[!] (p15)))) || (p6)))) && (G (((false) || (! (X[!] (p16)))) || (p7)))) && (G (((false) || (! (X[!] (p17)))) || (p8)))) && (G (((((((((((((((((((((((((((((((((((((((((((((((((((((((((true) && (((false) || (! (X[!] (p10)))) || (! (X[!] (p11))))) && (((false) || (! (X[!] (p10)))) || (! (X[!] (p12))))) && (((false) || (! (X[!] (p10)))) || (! (X[!] (p13))))) && (((false) || (! (X[!] (p10)))) || (! (X[!] (p14))))) && (((false) || (! (X[!] (p10)))) || (! (X[!] (p15))))) && (((false) || (! (X[!] (p10)))) || (! (X[!] (p16))))) && (((false) || (! (X[!] (p10)))) || (! (X[!] (p17))))) && (((false) || (! (X[!] (p11)))) || (! (X[!] (p10))))) && (((false) || (! (X[!] (p11)))) || (! (X[!] (p12))))) && (((false) || (! (X[!] (p11)))) || (! (X[!] (p13))))) && (((false) || (! (X[!] (p11)))) || (! (X[!] (p14))))) && (((false) || (! (X[!] (p11)))) || (! (X[!] (p15))))) && (((false) || (! (X[!] (p11)))) || (! (X[!] (p16))))) && (((false) || (! (X[!] (p11)))) || (! (X[!] (p17))))) && (((false) || (! (X[!] (p12)))) || (! (X[!] (p10))))) && (((false) || (! (X[!] (p12)))) || (! (X[!] (p11))))) && (((false) || (! (X[!] (p12)))) || (! (X[!] (p13))))) && (((false) || (! (X[!] (p12)))) || (! (X[!] (p14))))) && (((false) || (! (X[!] (p12)))) || (! (X[!] (p15))))) && (((false) || (! (X[!] (p12)))) || (! (X[!] (p16))))) && (((false) || (! (X[!] (p12)))) || (! (X[!] (p17))))) && (((false) || (! (X[!] (p13)))) || (! (X[!] (p10))))) && (((false) || (! (X[!] (p13)))) || (! (X[!] (p11))))) && (((false) || (! (X[!] (p13)))) || (! (X[!] (p12))))) && (((false) || (! (X[!] (p13)))) || (! (X[!] (p14))))) && (((false) || (! (X[!] (p13)))) || (! (X[!] (p15))))) && (((false) || (! (X[!] (p13)))) || (! (X[!] (p16))))) && (((false) || (! (X[!] (p13)))) || (! (X[!] (p17))))) && (((false) || (! (X[!] (p14)))) || (! (X[!] (p10))))) && (((false) || (! (X[!] (p14)))) || (! (X[!] (p11))))) && (((false) || (! (X[!] (p14)))) || (! (X[!] (p12))))) && (((false) || (! (X[!] (p14)))) || (! (X[!] (p13))))) && (((false) || (! (X[!] (p14)))) || (! (X[!] (p15))))) && (((false) || (! (X[!] (p14)))) || (! (X[!] (p16))))) && (((false) || (! (X[!] (p14)))) || (! (X[!] (p17))))) && (((false) || (! (X[!] (p15)))) || (! (X[!] (p10))))) && (((false) || (! (X[!] (p15)))) || (! (X[!] (p11))))) && (((false) || (! (X[!] (p15)))) || (! (X[!] (p12))))) && (((false) || (! (X[!] (p15)))) || (! (X[!] (p13))))) && (((false) || (! (X[!] (p15)))) || (! (X[!] (p14))))) && (((false) || (! (X[!] (p15)))) || (! (X[!] (p16))))) && (((false) || (! (X[!] (p15)))) || (! (X[!] (p17))))) && (((false) || (! (X[!] (p16)))) || (! (X[!] (p10))))) && (((false) || (! (X[!] (p16)))) || (! (X[!] (p11))))) && (((false) || (! (X[!] (p16)))) || (! (X[!] (p12))))) && (((false) || (! (X[!] (p16)))) || (! (X[!] (p13))))) && (((false) || (! (X[!] (p16)))) || (! (X[!] (p14))))) && (((false) || (! (X[!] (p16)))) || (! (X[!] (p15))))) && (((false) || (! (X[!] (p16)))) || (! (X[!] (p17))))) && (((false) || (! (X[!] (p17)))) || (! (X[!] (p10))))) && (((false) || (! (X[!] (p17)))) || (! (X[!] (p11))))) && (((false) || (! (X[!] (p17)))) || (! (X[!] (p12))))) && (((false) || (! (X[!] (p17)))) || (! (X[!] (p13))))) && (((false) || (! (X[!] (p17)))) || (! (X[!] (p14))))) && (((false) || (! (X[!] (p17)))) || (! (X[!] (p15))))) && (((false) || (! (X[!] (p17)))) || (! (X[!] (p16))))))) && (! (F (G (((true) && (p1)) && (! (X[!] (p10)))))))) && (! (F (G (((true) && (p2)) && (! (X[!] (p11)))))))) && (! (F (G (((true) && (p3)) && (! (X[!] (p12)))))))) && (! (F (G (((true) && (p4)) && (! (X[!] (p13)))))))) && (! (F (G (((true) && (p5)) && (! (X[!] (p14)))))))) && (! (F (G (((true) && (p6)) && (! (X[!] (p15)))))))) && (! (F (G (((true) && (p7)) && (! (X[!] (p16)))))))) && (! (F (G (((true) && (p8)) && (! (X[!] (p17)))))))) && (G (((false) || (((((((((true) && (! (X[!] (p10)))) && (! (X[!] (p11)))) && (! (X[!] (p12)))) && (! (X[!] (p13)))) && (! (X[!] (p14)))) && (! (X[!] (p15)))) && (! (X[!] (p16)))) && (! (X[!] (p17))))) || (p9)))) && (G (((false) || (! (p1))) || (! (X[!] (p11)))))) && (G (((false) || (! (p1))) || (! (X[!] (p12)))))) && (G (((false) || (! (p1))) || (! (X[!] (p13)))))) && (G (((false) || (! (p1))) || (! (X[!] (p14)))))) && (G (((false) || (! (p1))) || (! (X[!] (p15)))))) && (G (((false) || (! (p1))) || (! (X[!] (p16)))))) && (G (((false) || (! (p1))) || (! (X[!] (p17)))))) && (true)))";
  auto actual_formula = parser.get_formula();
  REQUIRE(expected_formula == actual_formula);

}
#ifndef LYDIASYFT_TEST_UTILS_HPP
#define LYDIASYFT_TEST_UTILS_HPP

#include <string>
#include <sstream>
#include "lydia/parser/ltlf/driver.hpp"
#include "InputOutputPartition.h"

typedef std::vector<std::string> vars;

namespace Syft::Test {

  static std::string ROOT_DIRECTORY = __ROOT_DIRECTORY;
  static std::string SYFCO_LOCATION = ROOT_DIRECTORY + "/syfco";
  static std::string EXAMPLE_TEST_TLSF = ROOT_DIRECTORY + "/example/test.tlsf";
  static std::string EXAMPLE_001_TLSF = ROOT_DIRECTORY + "/example/001.tlsf";

  whitemech::lydia::ltlf_ptr parse_formula(const std::string& formula, whitemech::lydia::parsers::ltlf::LTLfDriver& driver);
  bool get_realizability_from_input(const std::string& formula, const std::vector<std::string>& input_variables, const std::vector<std::string>& output_variables);
  bool get_realizability(const whitemech::lydia::ltlf_ptr & formula, const Syft::InputOutputPartition& partition);

}

#endif //LYDIASYFT_TEST_UTILS_HPP

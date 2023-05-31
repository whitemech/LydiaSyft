#ifndef LYDIASYFT_TEST_UTILS_HPP
#define LYDIASYFT_TEST_UTILS_HPP

#include <string>
#include <sstream>
#include <chrono>
#include <ctime>
#include <iomanip>
#include "lydia/parser/ltlf/driver.hpp"
#include "InputOutputPartition.h"
#include "string_utilities.h"

typedef std::vector<std::string> vars;

namespace Syft::Test {

  static std::string ROOT_DIRECTORY = __ROOT_DIRECTORY;
  static std::string SYFCO_LOCATION = ROOT_DIRECTORY + "/syfco";
  static std::string EXAMPLE_TEST_TLSF = ROOT_DIRECTORY + "/example/test.tlsf";
  static std::string EXAMPLE_001_TLSF = ROOT_DIRECTORY + "/example/001.tlsf";
  static std::string DATASET_FOLDER = ROOT_DIRECTORY + "/example/TLSF";

  inline std::string get_time_str() {
    // Get current time as a time_point object
    auto now = std::chrono::system_clock::now();

    // Convert to time_t for use with gmtime
    std::time_t nowAsTimeT = std::chrono::system_clock::to_time_t(now);

    // Get current time as a tm object
    std::tm* nowAsTm = std::gmtime(&nowAsTimeT);

    // Get the current milliseconds
    auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

    // Create a stringstream to hold the time
    std::stringstream ss;

    // Write the time into the stringstream in ISO 8601 format
    ss << std::put_time(nowAsTm, "%Y-%m-%dT%H:%M:%S") << '.' << std::setfill('0') << std::setw(3) << milliseconds.count() << 'Z';

    // Return the time as a string
    return ss.str();
  }

  whitemech::lydia::ltlf_ptr parse_formula(const std::string& formula, whitemech::lydia::parsers::ltlf::LTLfDriver& driver);
  bool get_realizability_from_input(const std::string& formula, const std::vector<std::string>& input_variables, const std::vector<std::string>& output_variables);
  bool get_realizability(const whitemech::lydia::ltlf_ptr & formula, const Syft::InputOutputPartition& partition);

}

#endif //LYDIASYFT_TEST_UTILS_HPP

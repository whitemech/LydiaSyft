#include "game/InputOutputPartition.h"
#include "string_utilities.h"

#include <algorithm>
#include <fstream>


namespace Syft {

std::runtime_error InputOutputPartition::bad_file_format_exception(
    std::size_t line_number) {
  return std::runtime_error("Incorrect format in line " +
                            std::to_string(line_number) +
                            " of the partition file.");
}
  
InputOutputPartition::InputOutputPartition()
{}

InputOutputPartition InputOutputPartition::read_from_file(
    const std::string& filename) {
  InputOutputPartition partition;		     
			     
  std::ifstream in(filename);

  std::size_t line_number = 1;
  std::string line;
  std::getline(in, line);
  
  std::vector<std::string> input_substr;
  input_substr = Syft::split(line, ":");

  if (input_substr.size() != 2 || input_substr[0] != ".inputs") {
    throw bad_file_format_exception(line_number);
  }

  std::string trimmed_input_substr = Syft::trim(input_substr[1]); // remove leading and trailing whitespace
  partition.input_variables = Syft::split(trimmed_input_substr, " ");

  ++line_number;
  std::getline(in, line);
  
  std::vector<std::string> output_substr;
  output_substr = Syft::split(line, ":");

  if (output_substr.size() != 2 || output_substr[0] != ".outputs") {
    throw bad_file_format_exception(line_number);
  }

  std::string trimmed_output_substr = Syft::trim(output_substr[1]); // remove leading and trailing whitespace
  partition.output_variables = Syft::split(trimmed_output_substr, " ");

  return partition;
}

InputOutputPartition InputOutputPartition::construct_from_input(const std::vector<std::string> inputs_substr,
                                                               const std::vector<std::string> outputs_substr) {
    InputOutputPartition partition;
    partition.input_variables = inputs_substr;
    partition.output_variables = outputs_substr;
    return partition;
}

  bool InputOutputPartition::is_input(const std::string &var_name) {
    return std::find(input_variables.begin(), input_variables.end(), var_name) != input_variables.end();
  }

  bool InputOutputPartition::is_output(const std::string &var_name) {
    return std::find(output_variables.begin(), output_variables.end(), var_name) != output_variables.end();
  }

}

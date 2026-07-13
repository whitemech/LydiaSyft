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

  if (!in.is_open()) {
    throw std::runtime_error("Impossibile aprire il file: " + filename);
  }

  std::size_t line_number = 0;
  std::string line;

  auto get_next_valid_line = [&in, &line, &line_number]() -> bool {
    while (std::getline(in, line)) {
      line_number++;
      line.erase(std::remove(line.begin(), line.end(), '\r'), line.end());
      line = Syft::trim(line);
      if (!line.empty()) return true;
    }
    return false;
  };

  if (!get_next_valid_line()) throw bad_file_format_exception(line_number);

  std::vector<std::string> input_substr = Syft::split(line, ":");

  if (input_substr.size() != 2 || Syft::trim(input_substr[0]) != ".inputs") {
    throw bad_file_format_exception(line_number);
  }

  std::string trimmed_input_vals = Syft::trim(input_substr[1]);
  partition.input_variables = Syft::split(trimmed_input_vals, " ");

  // Read agents
  std::size_t agent_id = 0;
  while (get_next_valid_line()) {
    std::vector<std::string> agent_substr = Syft::split(line, ":");
    std::string expected_tag = ".agent" + std::to_string(agent_id);

    if (agent_substr.size() != 2 || Syft::trim(agent_substr[0]) != expected_tag) {
      throw bad_file_format_exception(line_number);
    }

    std::string trimmed_agent_vals = Syft::trim(agent_substr[1]);
    partition.agent_variables.emplace_back(Syft::split(trimmed_agent_vals, " "));
    ++agent_id;
  }

  return partition;
}

InputOutputPartition InputOutputPartition::construct_from_input(const std::vector<std::string> inputs_substr,
                                                               const std::vector<std::vector<std::string>> agents_substr) {
    InputOutputPartition partition;
    partition.input_variables = inputs_substr;
    partition.agent_variables = agents_substr;
    if (!agents_substr.empty()) {
        partition.output_variables = agents_substr[0];  // Backward compatibility
    }
    return partition;
}

  bool InputOutputPartition::is_input(const std::string &var_name) {
    return std::find(input_variables.begin(), input_variables.end(), var_name) != input_variables.end();
  }

  bool InputOutputPartition::is_agent(const std::string &var_name, std::size_t agent_id) {
    if (agent_id >= agent_variables.size()) return false;
    return std::find(agent_variables[agent_id].begin(), agent_variables[agent_id].end(), var_name) != agent_variables[agent_id].end();
  }

}

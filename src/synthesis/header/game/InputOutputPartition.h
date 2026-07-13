#ifndef INPUT_OUTPUT_PARTITION_H
#define INPUT_OUTPUT_PARTITION_H

#include <stdexcept>
#include <vector>

namespace Syft {

/**
 * \brief A partition of variables into input and agent variables.
 */
class InputOutputPartition {
private:
  
  static std::runtime_error bad_file_format_exception(std::size_t line_number);
  
public:
  
  std::vector<std::string> input_variables;
  std::vector<std::vector<std::string>> agent_variables;  // agent_variables[0] is main agent, 1..n are peer agents

  /**
   * \brief Creates a partition with no variables.
   */
  InputOutputPartition();

  /**
   * \brief check if a variable is an input variable
   */
   bool is_input(const std::string& var_name);

  /**
   * \brief check if a variable is an agent variable
   */
   bool is_agent(const std::string& var_name, std::size_t agent_id);

   /**
    * \brief Constructs a partition from a file.
    *                                                  
    * The file should look like
    *   .inputs: X1 X2 X3 X4
    *   .agent0: Y1 Y2 Y3  # main agent
    *   .agent1: Z1 Z2     # peer agent 1
    *
    * \param filename The name of the partition file.    
    * \return A partition with the input and output variables listed in the file
    */
  static InputOutputPartition read_from_file(const std::string& filename);

    /**
   * \brief Constructs a partition from inputs.
   *
   *
   * \param inputs_substr A string vector of input variables.
   * \param agents_substr A vector of string vectors, one for each agent.
   * \return A partition with the input and output variables listed in the file
   */
    static InputOutputPartition construct_from_input(const std::vector<std::string> inputs_substr, const std::vector<std::vector<std::string>> agents_substr);

    // Backward compatibility
    std::vector<std::string> output_variables;  // Alias for agent_variables[0]
    bool is_output(const std::string& var_name) {
        return is_agent(var_name, 0);
    }
    static InputOutputPartition construct_from_input(const std::vector<std::string> inputs_substr,
                                                    const std::vector<std::string> outputs_substr) {
        std::vector<std::vector<std::string>> agents = {outputs_substr};
        auto part = construct_from_input(inputs_substr, agents);
        part.output_variables = outputs_substr;  // Set alias
        return part;
    }
};

}

#endif // INPUT_OUTPUT_PARTITION_H

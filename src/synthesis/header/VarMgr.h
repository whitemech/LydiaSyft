#ifndef VAR_MGR_H
#define VAR_MGR_H

#include <memory>
#include <unordered_map>
#include <vector>
#include <iostream>

#include "cuddObj.hh"

namespace Syft {

/**
 * \brief A dictionary that maps variable names to indices and vice versa.
 */
    class VarMgr {
    private:

        std::shared_ptr<CUDD::Cudd> mgr_;
        std::unordered_map<int, std::string> index_to_name_;
        std::unordered_map<std::string, CUDD::BDD> name_to_variable_;
        std::size_t state_variable_count_ = 0;
        std::vector<std::vector<CUDD::BDD>> state_variables_;
        std::vector<CUDD::BDD> input_variables_; //enviroment variables
        std::vector<std::vector<CUDD::BDD>> agent_variables_; // 0 is the main agent, 1..n are peer agents
        std::size_t total_variable_count_ = 0;

    public:

        /**
         * \brief Constructs a VarMgr with no variables.
         */
        VarMgr();

        /**
        * \brief Prints the VarMgr
        */
        void print_mgr() const;

        /**
        * \brief Prints BDD variables
        */
        void print_bdd_vars(CUDD::BDD cube) const;

        /**
         * \brief Creates BDD variables and associates each with a name.
         *
         * \param variable_names The names of the variables to create. A new variable
         *   is only created if a variable by that name does not already exist.
         */
        void create_named_variables(const std::vector<std::string> &variable_names);

        /**
        * \brief Create and store input variables
        * 
        * \param input_vars The input variables to create
        * \return void. Adds input_vars to input variables
        */
        void create_input_variables(const std::vector<std::string>& input_vars);

        /**
        * \brief Create and store agent variables for a specific agent
        * 
        * \param agent_id The ID of the agent (0 for main agent, 1..n for peer agents)
        * \param agent_vars The agent variables to create
        * \return void. Adds agent_vars to the specified agent's variables
        */
        void create_agent_variables(std::size_t agent_id, const std::vector<std::string>& agent_vars);

        /**
         * \brief Creates and stores state variables.
         *
         * Multiple calls of this function create separate groups of state variables.
         * The call generates an ID for the automaton whose state space the variables
         * represent, so that the correct group of variables can be retrieved later.
         *
         * \param variable_count The number of state variables to create.
         * \return The automaton ID the variables are associated with.
         */
        std::size_t create_state_variables(std::size_t variable_count);

        /**
         * \brief Creates and stores named state variables
         * 
         * Multiple calls of this function create separate groups of state variables.
         * The call generates an ID for the automaton whose state space the variables
         * represent, so that the correct group of variables can be retrieved later.
         *
         * \param variable_names The names of the variables being created.
         * \return The automaton ID the variables are associated with.
         */
        std::size_t create_named_state_variables(const std::vector<std::string>& variable_names);

        /**
         * \brief Registers a new automaton ID associated with a product state space.
         *
         * This function does not create new state variables. Instead, the variables
         * associated with the new ID are the union of all variables for the automata
         * that form the product.
         *
         * \param automaton_ids A vector of automaton IDs all of which must already
         *   exist in the manager.
         * \return The automaton ID for the automaton formed by taking the product of
         *   the automata whose IDs were provided in \a automaton_ids.
         */
        std::size_t create_product_state_space(
                const std::vector<std::size_t> &automaton_ids);


        /**
         * \brief Returns the i-th state variable for a given automaton.
         */
        CUDD::BDD state_variable(std::size_t automaton_id, std::size_t i) const;

        /**
        * \brief get the state variables of the automaton with the given automaton_id
        * 
        * \param automaton_id. The automaton_id of the DFA whose state variables are being returned
        * \return std::vector<CUDD::BDD> of state variables
        */
        std::vector<CUDD::BDD> state_variables(std::size_t automaton_id) const;

        /**
         * \brief Converts a state vector to a BDD.
         *
         * \param automaton_id The ID of the automaton whose variables to use.
         * \param state_vector A vector of 1s and 0s encoding a state's binary
         *   representation.
         * \return A BDD representing a conjunction (l_1 & l_2 & ... & l_n) where
         *   l_i = !x_i if \a state_vector[i] = 0 and l_i = x_i otherwise, where x_i
         *   is the i-th state variable of the automaton with the given ID.
         */
        CUDD::BDD state_vector_to_bdd(std::size_t automaton_id,
                                      const std::vector<int> &state_vector) const;

        /**
         * \brief Partitions the named variables between inputs and agents.
         *
         * \param input_names The names of the variables that should be considered
         *   inputs.
         * \param agent_names A vector of vectors, where agent_names[0] contains
         *   the names for the main agent, and agent_names[1..n] for peer agents.
         */
        void partition_variables(const std::vector<std::string> &input_names,
                                 const std::vector<std::vector<std::string>> &agent_names);

        /**
         * \brief Returns the CUDD manager used to create the variables.
         */
        std::shared_ptr<CUDD::Cudd> cudd_mgr() const;

        /**
         * \brief Returns the index of the variable with the given name.
         */
        CUDD::BDD name_to_variable(const std::string &name) const;

        /**
         * \brief Returns the name of the variable at index \a index.
         */
        std::string index_to_name(int index) const;

        /**
         * \brief Returns the map between integers and var names
         */
        std::unordered_map<int, std::string> get_index_to_name_map() const;

        /**
         * \brief Returns the total number of variables, including named and state.
         */
        std::size_t total_variable_count() const;

        /**
         * \brief Returns the total number of state variables.
         */
        std::size_t total_state_variable_count() const;

        /**
         * \brief Returns the number of state variables for a given automaton.
         */
        std::size_t state_variable_count(std::size_t automaton_id) const;

        /**
         * \brief Returns the number of input variables.
         */
        std::size_t input_variable_count() const;

        /**
         * \brief Returns the number of environment variables.
         */
        std::size_t environment_variable_count() const;

        /**
         * \brief Returns the number of variables for a given agent.
         */
        std::size_t agent_variable_count(std::size_t agent_id) const;
        
        /**
         * \brief Returns the number agents.
         */
        std::size_t agents_count() const;

        /**
         * \brief Backword compatibility: Returns a BDD formed by the conjunction of all environment variables.
         */
        CUDD::BDD input_cube() const;

        /**
         * \brief Returns a BDD formed by the conjunction of all agents variables.
         */
        CUDD::BDD environment_input_cube() const;

        /**
         * \brief Returns a BDD formed by the conjunction of all environment variables.
         */
        CUDD::BDD environment_output_cube() const;


        /**
         * \brief Returns a BDD formed by the conjunction of all the variables wich are not under the control of agent with ID agent_id.
         */
        CUDD::BDD agent_input_cube(std::size_t agent_id) const;

        /**
         * \brief Returns a BDD formed by the conjunction of all agent variables for a given agent.
         */
        CUDD::BDD agent_output_cube(std::size_t agent_id) const;

        /**
        * @brief Determine whether a string is an input variable
        * \param var. The name of a variable as a string
        */
        bool is_input_variable(const std::string& var) const;

        /**
        * @brief Determine whether a string is an agent variable
        * \param var. The name of a variable as a string
        * \param agent_id. The ID of the agent         */
        bool is_agent_variable(const std::string& var, std::size_t agent_id) const;

        /**
         * \brief Returns a BDD formed by the conjunction of all state variables of automaton automaton_id.
         */
        CUDD::BDD state_variables_cube(std::size_t automaton_id) const;

        /**
         * \brief Creates a valid input to CUDD::BDD::Eval.
         *
         * \param automaton_id The ID of the automaton whose variables to use.
         * \param state_vector A vector of 1s and 0s encoding a state's binary
         *   representation.
         * \return A vector with one position for each BDD variable, including both
         *   named and state variables, where the index corresponding to the i-th
         *   state variable of the requested automaton contains \a state_vector[i].
         *   Indices corresponding to other variables have a 0.
         */
        std::vector<int> make_eval_vector(std::size_t automaton_id,
                                          const std::vector<int> &state_vector) const;

        /**
       * \brief Creates a valid input to CUDD::BDD::Eval.
       *
       * \param automaton_id The ID of the automaton whose variables to use.
       * \param state_vector A vector of 1s and 0s encoding a state's binary
       *   representation.
       * \return A vector with one position for each BDD variable, including only
       *   state variables, where the index corresponding to the i-th
       *   state variable of the requested automaton contains \a state_vector[i].
       *   Indices corresponding to other variables have a 0.
       */
        std::vector<int> make_state_eval_vector(std::size_t automaton_id,
                                                const std::vector<int> &state_vector) const;

        /**
         * \brief Creates a valid input to CUDD::BDD::VectorCompose.
         *
         * \param automaton_id The ID of the automaton whose variables to use.
         * \param state_bdds A vector containing one BDD for each state variable of
         *   the automaton.
         * \return A vector with one position for each BDD variable, including both
         *   named and state variables, where the index corresponding to the i-th
         *   state variable of the requested automaton contains \a state_bdds[i].
         *   Indices corresponding to other variables have the identity BDD for that
         *   variable.
         */
        std::vector<CUDD::BDD> make_compose_vector(
                std::size_t automaton_id,
                const std::vector<CUDD::BDD> &state_bdds) const;

        /**
         * \brief Returns a vector with a label for each variable.
         *
         * To be used with CUDD::Cudd::DumpDot.
         *
         * \return A vector \a v such that \a v[i] has a label for the variable with
         *   index \a i. For named variables the label is the variable's name. For
         *   state variables the label is a representation that indicates which
         *   automaton the variable is from and which bit of the state it represents.
         */
        std::vector<std::string> variable_labels() const;

        /**
         * \brief Returns a vector with a label for each input variable.
         *
         * To be used with CUDD::Cudd::DumpDot.
         *
         * \return A vector \a v such that \a v[i] contains the name of the i-th
         *   input variable.
         */
        std::vector<std::string> input_variable_labels() const;

        /**
         * \brief Returns a vector with a label for each agent variable.
         *
         * To be used with CUDD::Cudd::DumpDot.
         *
         * \param agent_id The ID of the agent whose variables to label.
         * \return A vector \a v such that \a v[i] contains the name of the i-th
         *   agent variable.
         */
        std::vector<std::string> agent_variable_labels(std::size_t agent_id) const;

        /**
         * \brief Returns a vector with a label for each state variable.
         *
         * To be used with CUDD::Cudd::DumpDot.
         *
         * \return A vector \a v such that \a v[i] has a label for the i-th state
         *   variable. The label is the same as the one used in \a variable_labels().
         */
        std::vector<std::string> state_variable_labels(
                std::size_t automaton_id) const;

        /**
         * \brief Saves an ADD in a .dot file.
         *
         * Uses ADD for better visualization in the .dot file. To call with a BDD, use
         * \a dump_dot(bdd.Add(), filename).
         *
         * \param An ADD \a add and \a filename.
         */
        void dump_dot(const CUDD::ADD &add, const std::string &filename) const;

        /**
         * \brief Saves ADDs to a .dot file with corresponding labels.
         *
         * \param adds A vector of ADDs to be saved to file.
         * \param function_labels A vector such that \a function_labels[i] contains
         *   the label for \a adds[i]. The labels appear above the corresponding ADD
         *   in the file.
         * \param filename The name of the .dot file.
         */
        void dump_dot(const std::vector<CUDD::ADD> &adds,
                      const std::vector<std::string> &function_labels,
                      const std::string &filename) const;

        /**
         * \brief Returns the number of DFAs the manager handles.
         *
         */
        std::size_t automaton_num() const;

        /**
         * \brief Prints a BDD as a Boolean formula, where each variable has a specific label.
         *
         * \param A given BDD.
         * \return A string that represents a Boolean formula.
         */
        std::string bdd_to_string(const CUDD::BDD &bdd) const;

        /**
         * \brief Create the state space when complementing a DFA.
         *
         */
        std::size_t create_complement_state_space(
                const std::size_t automaton_id);
        
        /**
         * \brief Create the state space when creating the core of a DFA.
         *
         */
        std::size_t create_core_state_space(
                const std::size_t automaton_id, const std::string& sink_name);

        /**
         * \brief Create the non empty space.
         *
         */
        std::size_t create_nonempty_state_space(
                const std::size_t automaton_id, const std::string& ne_name);


        // Backward compatibility for two-player
        void create_output_variables(const std::vector<std::string>& output_vars) {
            create_agent_variables(0, output_vars);  // Main agent is index 0
        }
        std::size_t output_variable_count() const {
            return agent_variable_count(0);
        }
        CUDD::BDD output_cube() const {
            return agent_output_cube(0);
        }
        bool is_output_variable(const std::string& var) const {
            return is_agent_variable(var, 0);
        }
        std::vector<std::string> output_variable_labels() const {
            return agent_variable_labels(0);
        }
        // Overload partition_variables for old signature
        void partition_variables(const std::vector<std::string> &input_names,
                                 const std::vector<std::string> &output_names) {
            std::vector<std::vector<std::string>> agent_names = {output_names};  // Only main agent
            partition_variables(input_names, agent_names);
        }
    };

}

#endif // VAR_MGR_H

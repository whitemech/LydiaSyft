#include "VarMgr.h"

#include <cstring>
#include <stdexcept>
#include <sstream>
#include <algorithm>

namespace Syft {

VarMgr::VarMgr() {
  mgr_ = std::make_shared<CUDD::Cudd>();
}

void VarMgr::print_mgr() const {
  // prints the number of managed automata
  std::cout << "Number of managed automata: " << state_variables_.size() << std::endl;

  // prints number of vars
  std::cout << "Number of variables: " << total_variable_count() << std::endl;

  // prints named variables
  std::cout << "Named variables (name, var): " << std::endl;

  for (const auto& name_var : name_to_variable_)
    std::cout << "Name: " << name_var.first << ". Var: " << name_var.second << std::endl;
  
  std::cout << "Var indexes (index, name): " << std::endl;

  for (const auto& index_name: index_to_name_)
    std::cout << "Index: " << index_name.first << ". Name: " << index_name.second << std::endl;

  // prints X vars
  std::cout << "Input variables: " << std::endl;
  for (const auto& var: input_variables_) std::cout << "Var: " << var << std::endl;

  // prints Agent vars
  for (std::size_t i = 0; i < agent_variables_.size(); ++i) {
    std::cout << "Agent " << i << " variables: " << std::endl;
    for (const auto& var: agent_variables_[i]) std::cout << "Var: " << var << std::endl;
  }
  
  std::cout << "\nCUBE TEST \n" << std::endl;
  std::cout << "---Environment role---" << std::endl;
  CUDD::BDD env_out = environment_output_cube();
  CUDD::BDD env_in = environment_input_cube();
  std::cout << "Env Output Cube controls: " << std::endl;
  print_bdd_vars(env_out);
  std::cout << "Env Input Cube see others moves: " << std::endl;
  print_bdd_vars(env_in);

  for(std::size_t i=0; i< agent_variables_.size(); ++i){
    std::cout << "\n---Agent "<< i << " role---"<<std::endl;
    CUDD::BDD ag_out = agent_output_cube(i);
    CUDD::BDD ag_in = agent_input_cube(i);
    std::cout << "Agent "<< i << " Output Cube controls: " << std::endl;
    print_bdd_vars(ag_out);
    std::cout << "Agent " << i << " Input Cube see others moves: " << std::endl;
    print_bdd_vars(ag_in);
    
  }

  // prints Z vars for each managed automaton
  for (int i = 0; i < state_variables_.size(); ++i) {
    std::cout << "Automaton ID " << i << " state variables" << std::endl;
    for (const auto& var: state_variables_[i]) std::cout << var << " ";
    std::cout << std::endl;
  }
}

void VarMgr::print_bdd_vars(CUDD::BDD cube)const{
  if(cube.IsOne()){
    std::cout << "Empty Cube" << std::endl;
    return;
  }
  std::vector<unsigned int> indices = cube.SupportIndices();
  std::cout << "{";
  for(unsigned int idx : indices){
    auto it= index_to_name_.find(idx);
    if(it != index_to_name_.end()){
      std::cout << it->second << "(v" << idx <<") ";
    }else{
      std::cout << "v" << idx << " ";
    }
  }
  std::cout << "} (Tot: " << indices.size() << ")" << std::endl;
}

void VarMgr::create_named_variables(
    const std::vector<std::string>& variable_names) {
  for (const std::string& name : variable_names) {
    // Only create the variable if it doesn't already exist
    if (name_to_variable_.find(name) == name_to_variable_.end()) {
      CUDD::BDD new_variable = mgr_->bddVar();
      int new_index = new_variable.NodeReadIndex();
      name_to_variable_[name] = new_variable;
      index_to_name_[new_index] = name;
    }
  }
}

void VarMgr::create_input_variables(
  const std::vector<std::string>& input_vars
) {
  for (const std::string& input_var : input_vars) {
    input_variables_.push_back(name_to_variable_[input_var]);
    }
}

//Create Agent vars
void VarMgr::create_agent_variables(std::size_t agent_id, const std::vector<std::string>& agent_vars) {
  // Ensure agent_variables_ has enough space
  if (agent_variables_.size() <= agent_id) {
    agent_variables_.resize(agent_id + 1);
  }
  for (const std::string& agent_var : agent_vars) {
    agent_variables_[agent_id].push_back(name_to_variable_[agent_var]);
  }
}

std::size_t VarMgr::create_state_variables(std::size_t variable_count) {
  std::size_t automaton_id = state_variables_.size();

  // Creates an additional space for variables at index automaton_id,
  // then reserves enough memory for all the new variables
  state_variables_.emplace_back();
  state_variables_[automaton_id].reserve(variable_count);

    for (std::size_t i = 0; i < variable_count; ++i) {
        // Creates a new variable at the top of the variable ordering
        CUDD::BDD new_state_variable = mgr_->bddNewVarAtLevel(0);

        state_variables_[automaton_id].push_back(new_state_variable);
        int new_index = new_state_variable.NodeReadIndex();
        std::string name = "Z"+std::to_string(state_variable_count_+i);
        name_to_variable_[name] = new_state_variable;
        index_to_name_[new_index] = name;
    }

  state_variable_count_ += variable_count;

  return automaton_id;
}

std::size_t VarMgr::create_named_state_variables(const std::vector<std::string>& vars) {
  std::size_t automaton_id = state_variables_.size();

  // Creates an additional space for variables at index automaton_id,
  // then reserves enough memory for all the new variables
  state_variables_.emplace_back();
  state_variables_[automaton_id].reserve(vars.size());
  int new_state_variable_counter = 0;

  for (int i = 0; i < vars.size(); ++i) {
    // Create a new variable if this named variable does not already exist
    // add the new variable to the state variables of the automaton 
    if (name_to_variable_.find(vars[i]) == name_to_variable_.end()) {
        CUDD::BDD new_state_variable = mgr_->bddNewVarAtLevel(0);
        state_variables_[automaton_id].push_back(new_state_variable);
        name_to_variable_[vars[i]] = new_state_variable;
        index_to_name_[new_state_variable.NodeReadIndex()] = vars[i]; 
        new_state_variable_counter++;
    } else { // Else add the existing variable to the state variables of the automaton
      state_variables_[automaton_id].push_back(name_to_variable_[vars[i]]);
    }
  }

  state_variable_count_ += new_state_variable_counter;
  return automaton_id;
}

std::size_t VarMgr::create_product_state_space(
    const std::vector<std::size_t>& automaton_ids) {
  std::size_t product_automaton_id = state_variables_.size();

  state_variables_.emplace_back();

  for (std::size_t automaton_id : automaton_ids) {
    state_variables_[product_automaton_id].insert(
	state_variables_[product_automaton_id].end(),
	state_variables_[automaton_id].begin(),
	state_variables_[automaton_id].end());
  }

  return product_automaton_id;
}

    std::size_t VarMgr::create_complement_state_space(
            const std::size_t automaton_id) {
        std::size_t complement_automaton_id = state_variables_.size();

        state_variables_.emplace_back();

        state_variables_[complement_automaton_id].insert(
                state_variables_[complement_automaton_id].end(),
                state_variables_[automaton_id].begin(),
                state_variables_[automaton_id].end());

        return complement_automaton_id;
    }

    std::size_t VarMgr::create_core_state_space(const std::size_t automaton_id, const std::string& sink_name) {
      std::size_t core_automaton_id = state_variables_.size();

      state_variables_.emplace_back();

      state_variables_[core_automaton_id].insert(
              state_variables_[core_automaton_id].end(),
              state_variables_[automaton_id].begin(),
              state_variables_[automaton_id].end());

      for (const CUDD::BDD& var : state_variables_[automaton_id]) {
          std::size_t index = var.NodeReadIndex();
          if (index_to_name_.find(index) != index_to_name_.end()) {
              index_to_name_[index] = index_to_name_[index]; 
          }
      }
      
      //Create sink state variable for the core space     
      std::string unique_sink_name = sink_name + "_" + std::to_string(core_automaton_id);

      // Alloca ed inserisce la nuova variabile di stato (z_sink) per questo core space
      CUDD::BDD new_state_variable = mgr_->bddNewVarAtLevel(0);
      state_variables_[core_automaton_id].push_back(new_state_variable);
      name_to_variable_[unique_sink_name] = new_state_variable;
      index_to_name_[new_state_variable.NodeReadIndex()] = unique_sink_name;

      // Aggiorna il contatore globale delle variabili del manager
      state_variable_count_ += 1;


      return core_automaton_id;
}


 std::size_t VarMgr::create_nonempty_state_space(const std::size_t automaton_id, const std::string& ne_name) {
      std::size_t core_automaton_id = state_variables_.size();

      state_variables_.emplace_back();

      state_variables_[core_automaton_id].insert(
              state_variables_[core_automaton_id].end(),
              state_variables_[automaton_id].begin(),
              state_variables_[automaton_id].end());

      for (const CUDD::BDD& var : state_variables_[automaton_id]) {
          std::size_t index = var.NodeReadIndex();
          if (index_to_name_.find(index) != index_to_name_.end()) {
              index_to_name_[index] = index_to_name_[index]; 
          }
      }
      
      //Create sink state variable for the core space     
      std::string unique_ne_name = ne_name + "_" + std::to_string(core_automaton_id);

      // Alloca ed inserisce la nuova variabile di stato (z_sink) per questo core space
      CUDD::BDD new_state_variable = mgr_->bddNewVarAtLevel(0);
      state_variables_[core_automaton_id].push_back(new_state_variable);
      name_to_variable_[unique_ne_name] = new_state_variable;
      index_to_name_[new_state_variable.NodeReadIndex()] = unique_ne_name;

      // Aggiorna il contatore globale delle variabili del manager
      state_variable_count_ += 1;


      return core_automaton_id;
}


    
CUDD::BDD VarMgr::state_variable(std::size_t automaton_id, std::size_t i)
    const {
  return state_variables_[automaton_id][i];
}

std::vector<CUDD::BDD> VarMgr::state_variables(std::size_t automaton_id) const {
  return state_variables_[automaton_id];
}
  
CUDD::BDD VarMgr::state_vector_to_bdd(std::size_t automaton_id,
				      const std::vector<int>& state_vector)
    const {
  CUDD::BDD bdd = mgr_->bddOne();
  
  for (std::size_t i = 0; i < state_vector.size(); ++i) {
    if (state_vector[i]) {
      bdd &= state_variables_[automaton_id][i];
    } else {
      bdd &= !state_variables_[automaton_id][i];
    }
  }

  return bdd;
}

void VarMgr::partition_variables(const std::vector<std::string>& input_names,
				 const std::vector<std::vector<std::string>>& agent_names) {
  //if (!input_variables_.empty() || !agent_variables_.empty()) {
  //  throw std::runtime_error(
  // 	"Error: Only one input-agent partition is allowed.");
  // }
  input_variables_.clear();
  agent_variables_.clear();

  for(const std::string& input_name : input_names) {
    if (name_to_variable_.find(input_name) != name_to_variable_.end()) {
      input_variables_.push_back(name_to_variable_[input_name]);
    }
  }
  for(std::size_t i = 0; i < agent_names.size(); ++i) {
    agent_variables_.emplace_back();
    for(const std::string& agent_name : agent_names[i]) {
      if (name_to_variable_.find(agent_name) != name_to_variable_.end()) {
        agent_variables_[i].push_back(name_to_variable_[agent_name]);
      }
    }
  } 

  std::size_t total_agent_vars = 0;
  for (const auto& names : agent_names) {
    total_agent_vars += names.size();
  }

  if (input_names.size() + total_agent_vars != (index_to_name_.size() - state_variable_count_)) {
    throw std::runtime_error(
         "Error: Input-agent partition is the wrong size.");
  }


  std::size_t actual_partition_size = input_names.size() + total_agent_vars;
  std::size_t expected_partition_size = index_to_name_.size() - state_variable_count_;
  if (actual_partition_size != expected_partition_size) {
   std::string error_msg = "Partition Error:";
    error_msg += " Expected partition size: " + std::to_string(expected_partition_size) + ".";
    error_msg += " Actual partition size: " + std::to_string(actual_partition_size) + ".";
   throw std::runtime_error(error_msg);
  }

}

std::shared_ptr<CUDD::Cudd> VarMgr::cudd_mgr() const {
  return mgr_;
}

CUDD::BDD VarMgr::name_to_variable(const std::string& name) const {
  return name_to_variable_.at(name);
}

std::string VarMgr::index_to_name(int index) const {
  return index_to_name_.at(index);
}

std::unordered_map<int, std::string> VarMgr::get_index_to_name_map() const {
  return index_to_name_;
}

std::size_t VarMgr::total_variable_count() const {
  return name_to_variable_.size();
}
  
std::size_t VarMgr::total_state_variable_count() const {
  return state_variable_count_;
}

std::size_t VarMgr::state_variable_count(std::size_t automaton_id) const {
  return state_variables_[automaton_id].size();
}
  
std::size_t VarMgr::input_variable_count() const {
  return input_variables_.size();
}

std::size_t VarMgr::environment_variable_count() const {
  return input_variables_.size();
}

std::size_t VarMgr::agent_variable_count(std::size_t agent_id) const {
  if (agent_id >= agent_variables_.size()) return 0;
  return agent_variables_[agent_id].size();
}

std::size_t VarMgr::agents_count() const {
  return agent_variables_.size();
}

// all agents variables
CUDD::BDD VarMgr::environment_input_cube() const {
  CUDD::BDD all_agents_cube = mgr_->bddOne();
  for(std::size_t i = 0; i < agent_variables_.size(); ++i){
    all_agents_cube &= agent_output_cube(i);
  }
  return all_agents_cube;  
}

CUDD::BDD VarMgr::environment_output_cube() const {
  return mgr_->computeCube(input_variables_);
}

//env variables + all agents variables, except agent with agent_id ones
CUDD::BDD VarMgr::agent_input_cube(std::size_t agent_id)const{
  CUDD::BDD combined_input = environment_output_cube();
  for (std::size_t i = 0; i < agent_variables_.size(); ++i){
    if((int)i == agent_id) continue;
    combined_input &= agent_output_cube(i);
  }
  return combined_input;
}

CUDD::BDD VarMgr::agent_output_cube(std::size_t agent_id) const {
  if (agent_id >= agent_variables_.size()) return mgr_->bddOne();
  return mgr_->computeCube(agent_variables_[agent_id]);
}

//Backword compatibility
CUDD::BDD VarMgr::input_cube() const {
  return mgr_->computeCube(input_variables_);
}

bool VarMgr::is_input_variable(const std::string& var) const {
  CUDD::BDD bdd_var = name_to_variable_.at(var);
  if (std::find(input_variables_.begin(), input_variables_.end(), bdd_var) != input_variables_.end())
    return true;
  return false;
}

bool VarMgr::is_agent_variable(const std::string& var, std::size_t agent_id) const {
  if (agent_id >= agent_variables_.size()) return false;
  CUDD::BDD bdd_var = name_to_variable_.at(var);
  if (std::find(agent_variables_[agent_id].begin(), agent_variables_[agent_id].end(), bdd_var) != agent_variables_[agent_id].end())
    return true;
  return false;
}

CUDD::BDD VarMgr::state_variables_cube(std::size_t automaton_id) const {
    return mgr_->computeCube(state_variables_[automaton_id]);
}

std::vector<int> VarMgr::make_eval_vector(
    std::size_t automaton_id, const std::vector<int>& state_vector) const {
  std::vector<int> eval_vector(total_variable_count(), 0);

  for (std::size_t i = 0; i < state_variables_[automaton_id].size(); ++i) {
    std::size_t index = state_variables_[automaton_id][i].NodeReadIndex();
    eval_vector[index] = state_vector[i];
  }

  return eval_vector;
}

std::vector<int> VarMgr::make_state_eval_vector(
        std::size_t automaton_id, const std::vector<int>& state_vector) const {
    std::vector<int> eval_vector(state_variables_[automaton_id].size(), 0);

    for (std::size_t i = 0; i < state_variables_[automaton_id].size(); ++i) {
        std::size_t index = state_variables_[automaton_id][i].NodeReadIndex();
        eval_vector[index] = state_vector[i];
    }

    return eval_vector;
}

std::vector<CUDD::BDD> VarMgr::make_compose_vector(
    std::size_t automaton_id, const std::vector<CUDD::BDD>& state_bdds) const {
  std::vector<CUDD::BDD> compose_vector(total_variable_count(),
					mgr_->bddZero());

  // All named variables get mapped to the variable itself
  for (const auto& name_and_variable : name_to_variable_) {
    CUDD::BDD variable = name_and_variable.second;
    std::size_t index = variable.NodeReadIndex();
    compose_vector[index] = variable;
  }

  // State variables for other automaton ids get mapped to the variable itself
  
  for (std::size_t id = 0; id < automaton_id; ++id) {
    for (const CUDD::BDD& variable : state_variables_[id]) {
      std::size_t index = variable.NodeReadIndex();
      compose_vector[index] = variable;
    }
  }

  for (std::size_t id = automaton_id + 1; id < state_variables_.size(); ++id) {
    for (const CUDD::BDD& variable : state_variables_[id]) {
      std::size_t index = variable.NodeReadIndex();
      compose_vector[index] = variable;
    }
  }

  // The i-th state variable gets mapped to the i-th BDD from the input
  for (std::size_t i = 0; i < state_variables_[automaton_id].size(); ++i) {
    std::size_t index = state_variables_[automaton_id][i].NodeReadIndex();
    compose_vector[index] = state_bdds[i];
  }

  return compose_vector;
}
  
std::vector<std::string> VarMgr::variable_labels() const {
  std::vector<std::string> labels(total_variable_count());

  for (const auto& index_and_name : index_to_name_) {
    labels[index_and_name.first] = index_and_name.second;
  }

  for (std::size_t id = 0; id < state_variables_.size(); ++id) {
    for (std::size_t i = 0; i < state_variables_[id].size(); ++i) {
      std::size_t index = state_variables_[id][i].NodeReadIndex();
      // labels[index] = "A" + std::to_string(id) + ":Z" + std::to_string(i);
      labels[index] = index_to_name_.at(index);
    }
  }

  return labels;
}

std::vector<std::string> VarMgr::agent_variable_labels(std::size_t agent_id) const {
  if (agent_id >= agent_variables_.size()) return {};
  std::vector<std::string> labels(agent_variables_[agent_id].size());

  for (std::size_t i = 0; i < agent_variables_[agent_id].size(); ++i) {
    std::size_t index = agent_variables_[agent_id][i].NodeReadIndex();
    labels[i] = index_to_name(index);
  }

  return labels;
}

std::vector<std::string> VarMgr::input_variable_labels() const {
  std::vector<std::string> labels(input_variable_count());

  for (std::size_t i = 0; i < input_variable_count(); ++i) {
    std::size_t index = input_variables_[i].NodeReadIndex();
    labels[i] = index_to_name(index);
  }

  return labels;
}

std::vector<std::string> VarMgr::state_variable_labels(
    std::size_t automaton_id) const {
	std::vector<std::string> labels;
	labels.reserve(total_state_variable_count());

	std::string id_string = "A" + std::to_string(automaton_id);

	for (std::size_t i = 0; i < state_variables_[automaton_id].size(); ++i) {
		// labels.push_back(id_string + ":Z" + std::to_string(i));
    labels.push_back(index_to_name_.at(state_variables_[automaton_id][i].NodeReadIndex()));
	}

	return labels;
}

void VarMgr::dump_dot(const CUDD::ADD& add, const std::string& filename) const {
  std::vector<CUDD::ADD> single(1, add);
  std::vector<std::string> single_label(1, "");

  dump_dot(single, single_label, filename);
}

void VarMgr::dump_dot(const std::vector<CUDD::ADD>& adds,
                      const std::vector<std::string>& function_labels,
                      const std::string& filename) const {
	std::vector<std::string> variable_labels = this->variable_labels();
  std::size_t variable_count = variable_labels.size();
  std::vector<char*> inames(variable_count);

  for (std::size_t i = 0; i < variable_count; ++i) {
    std::string label = variable_labels[i];

    // Return value of label.c_str() is temporary, so need to make a copy
    inames[i] = new char[label.length() + 1]; // need space for '\0' terminator
    std::strcpy(inames[i], label.c_str());
  }

  std::size_t function_count = function_labels.size();
  std::vector<char*> onames(function_count);

  for (std::size_t i = 0; i < function_count; ++i) {
    std::string label = function_labels[i];

    // Return value of label.c_str() is temporary, so need to make a copy
    onames[i] = new char[label.length() + 1]; // need space for '\0' terminator
    std::strcpy(onames[i], label.c_str());
  }

  FILE* fp = fopen(filename.c_str(), "w");

  mgr_->DumpDot(adds, inames.data(), onames.data(), fp);

  fclose(fp);

  for (std::size_t i = 0; i < variable_count; ++i) {
    delete[] inames[i];
  }

  for (std::size_t i = 0; i < function_count; ++i) {
    delete[] onames[i];
  }
}

std::size_t VarMgr::automaton_num() const {
  return state_variables_.size();
}

std::string VarMgr::bdd_to_string(const CUDD::BDD& bdd) const {
    std::stringstream ss;
    ss << bdd;
    std::string s = ss.str();

    // Replace x with $ in the variable representation to avoid problems if a
    // var label happens to have the form x#. This assumes that the var labels
    // cannot have the form $#.
    while (true) {
        std::size_t pos = s.find("x");

        if (pos == std::string::npos) {
            break;
        }

        s.replace(pos, 1, "$");
    }

    // Sort ids from highest to lowest so that ids with more digits get replaced
    // before ids with fewer digits.
    std::vector<unsigned int> ids = bdd.SupportIndices();
    std::sort(ids.begin(), ids.end(), std::greater<unsigned int>());


    for (unsigned int id : ids) {
        std::string var_string = "$" + std::to_string(id);

        while (true) {
            std::size_t pos = s.find(var_string);

            if (pos == std::string::npos) {
                break;
            }

            s.replace(pos, var_string.size(), "("+index_to_name(id)+")");
        }
    }

    return s;
}
}

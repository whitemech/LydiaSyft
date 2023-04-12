#include "ExplicitStateDfa.h"

#include <algorithm>
#include <fstream>

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/trim.hpp>

namespace Syft {

ExplicitStateDfa::ExplicitStateDfa(std::shared_ptr<VarMgr> var_mgr)
  : var_mgr_(std::move(var_mgr))
{}

std::runtime_error ExplicitStateDfa::bad_file_format_exception(
    std::size_t line_number) {
  return std::runtime_error("Incorrect format in line " +
			    std::to_string(line_number) +
			    " of the DFA file.");
}

std::vector<std::string> ExplicitStateDfa::read_mona_line(
    std::istream& in,
    std::size_t line_number) {
  std::string line;
  std::getline(in, line);

  std::vector<std::string> substr;
  boost::split(substr, line, boost::is_any_of(":"));

  if (substr.size() != 2) {
    throw bad_file_format_exception(line_number);
  }

  return substr;
}

std::vector<std::string> ExplicitStateDfa::read_variable_names(
    std::istream& in,
    std::size_t line_number) {
  std::vector<std::string> substr = read_mona_line(in, line_number);
  
  if (substr[0] != "variables") {
    throw bad_file_format_exception(line_number);
  }

  std::vector<std::string> variables;
  boost::trim(substr[1]); // remove leading and trailing whitespace
  boost::split(variables, substr[1], boost::is_any_of(" "));
  
  return variables;
}

std::size_t ExplicitStateDfa::read_state_count(std::istream& in,
						      std::size_t line_number) {
  std::vector<std::string> substr = read_mona_line(in, line_number);
  
  if (substr[0] != "states") {
    throw bad_file_format_exception(line_number);
  }
  
  try {
    return std::stoi(substr[1]);
  }
  catch (const std::exception& e) {
    throw bad_file_format_exception(line_number);
  }
}

std::vector<std::size_t> ExplicitStateDfa::read_final_states(
    std::istream& in,
    std::size_t line_number) {
  std::vector<std::string> substr = read_mona_line(in, line_number);
  
  if (substr[0] != "final") {
    throw bad_file_format_exception(line_number);
  }

  std::vector<std::string> tokens;
  boost::trim(substr[1]); // remove leading and trailing whitespace
  boost::split(tokens, substr[1], boost::is_any_of(" "));

  std::vector<std::size_t> final_states;

  for (std::size_t i = 0; i < tokens.size(); ++i) {
    if (tokens[i] == "1") {
      final_states.push_back(i);
    }
  }
  
  return final_states;
}

std::vector<std::size_t> ExplicitStateDfa::read_behaviour(
    std::istream& in,
    std::size_t line_number) {
  std::vector<std::string> substr = read_mona_line(in, line_number);
  
  if (substr[0] != "behaviour") {
    throw bad_file_format_exception(line_number);
  }

  std::vector<std::string> tokens;
  boost::trim(substr[1]); // remove leading and trailing whitespace
  boost::split(tokens, substr[1], boost::is_any_of(" "));

  std::vector<std::size_t> behaviour;

  try {
    std::transform(tokens.begin(), tokens.end(), std::back_inserter(behaviour),
		   [] (const std::string& token) { return std::stoi(token); });
  }
  catch (const std::exception& e) {
    throw bad_file_format_exception(line_number);
  }
  
  return behaviour;
}

std::vector<std::vector<int>> ExplicitStateDfa::read_node_table(
    std::istream& in,
    std::size_t line_number) {
  std::string line;
  std::getline(in, line);

  if (line != "bdd:") {
    throw bad_file_format_exception(line_number);
  }

  std::vector<std::vector<int>> node_table;
  
  while (std::getline(in, line) && line != "end") {
    ++line_number;

    std::vector<std::string> tokens;
    boost::trim(line); // remove leading and trailing whitespace
    boost::split(tokens, line, boost::is_any_of(" "));

    std::vector<int> node_triple;
    std::transform(tokens.begin(), tokens.end(),
		   std::back_inserter(node_triple),
		   [] (const std::string& token) { return std::stoi(token); });

    if (node_triple.size() != 3) {
      throw bad_file_format_exception(line_number);
    }

    node_table.push_back(std::move(node_triple));
  }

  if (line != "end") {
    throw bad_file_format_exception(line_number + 1);
  }

  return node_table;
}

CUDD::ADD ExplicitStateDfa::build_add(
    std::size_t node_index,
    const std::shared_ptr<VarMgr>& var_mgr,
    const std::vector<std::string>& variable_names,
    const std::vector<std::vector<int>>& node_table,
    std::unordered_map<std::size_t, CUDD::ADD>& add_table) {
  auto it = add_table.find(node_index);
  
  if (it != add_table.end()) {
    return it->second;
  }
  else {
    int name_index = node_table[node_index][0];
    int low_child = node_table[node_index][1];
    int high_child = node_table[node_index][2];

    if (name_index == -1) {
      add_table[node_index] = var_mgr->cudd_mgr()->constant(low_child);
    }
    else {
      std::string variable_name = variable_names[name_index];
      CUDD::ADD root_node = var_mgr->name_to_variable(variable_name).Add();
      CUDD::ADD low_node = build_add(low_child, var_mgr, variable_names,
				     node_table, add_table);
      CUDD::ADD high_node = build_add(high_child, var_mgr, variable_names,
				      node_table, add_table);

      add_table[node_index] = root_node.Ite(high_node, low_node);
    }

    return add_table[node_index];
  }
}

std::string ExplicitStateDfa::ltlf_to_dfa_file(const std::string& ltlf_filename) {
    std::string fol_filename = ltlf_filename + ".mona";
    std::string ltlf_to_fol_command = "./ltlf2fol NNF " + ltlf_filename + " >" + fol_filename;
    system(ltlf_to_fol_command.c_str());

    std::string dfa_filename = ltlf_filename + ".dfa";
    std::string run_mona_command = "mona -xw " + fol_filename + " >" + dfa_filename;
    system(run_mona_command.c_str());

    return dfa_filename;
}

std::string ExplicitStateDfa::safetyltl_to_dfa_file(const std::string& safetyltl_filename) {
    std::string fol_filename = safetyltl_filename + ".mona";
    std::string ltlf_to_fol_command = "./safe2fol NNF " + safetyltl_filename + " >" + fol_filename;
    system(ltlf_to_fol_command.c_str());

    std::string dfa_filename = safetyltl_filename + ".dfa";
    std::string run_mona_command = "mona -xw -u " + fol_filename + " >" + dfa_filename;
    system(run_mona_command.c_str());

    return dfa_filename;
}

ExplicitStateDfa ExplicitStateDfa::from_dfa_mona(std::shared_ptr<VarMgr> var_mgr,
                                                 const ExplicitStateDfaMona &explicit_dfa) {
    std::size_t initial_state = explicit_dfa.get_initial_state();

    std::vector<std::string> variable_names = explicit_dfa.names;
    var_mgr->create_named_variables(variable_names);

    std::size_t state_count = explicit_dfa.get_nb_states();
    std::vector<std::size_t> final_states;
    for (int i = 0; i < state_count; i++) {
        std::size_t curr_state = explicit_dfa.get_initial_state() + i;
        if(explicit_dfa.is_final(curr_state)){
            final_states.push_back(curr_state);
        }
    }
    std::unordered_map<std::size_t, CUDD::ADD> add_table;
    std::vector<CUDD::ADD> transition_function(state_count);

    for (std::size_t i = 0; i < state_count; ++i) {
        transition_function[i] = build_add_from_dfa_mona(explicit_dfa.dfa_->q[i], var_mgr, variable_names,
                                           explicit_dfa, add_table);
    }

    ExplicitStateDfa dfa(std::move(var_mgr));
    dfa.initial_state_ = initial_state;
    dfa.state_count_ = state_count;
    dfa.final_states_ = std::move(final_states);
    dfa.transition_function_ = std::move(transition_function);

    return dfa;

}

CUDD::ADD ExplicitStateDfa::build_add_from_dfa_mona(unsigned node_index, const std::shared_ptr<VarMgr> &var_mgr,
                                                    const std::vector<std::string> &variable_names,
                                                    const ExplicitStateDfaMona& explicit_dfa,
                                                    std::unordered_map<std::size_t, CUDD::ADD> &add_table) {
    bdd_manager* mgr = explicit_dfa.dfa_->bddm;
    auto it = add_table.find(node_index);

    if (it != add_table.end()) {
        return it->second;
    }
    else {
        unsigned name_index, low_child, high_child;
        LOAD_lri(&mgr->node_table[node_index], low_child, high_child,
                 name_index);

        if (name_index == BDD_LEAF_INDEX) {
            add_table[node_index] = var_mgr->cudd_mgr()->constant(low_child);
        }
        else {
            std::string variable_name = variable_names[name_index];
            CUDD::ADD root_node = var_mgr->name_to_variable(variable_name).Add();
            CUDD::ADD low_node = build_add_from_dfa_mona(low_child, var_mgr, variable_names,
                                           explicit_dfa, add_table);
            CUDD::ADD high_node = build_add_from_dfa_mona(high_child, var_mgr, variable_names,
                                            explicit_dfa, add_table);

            add_table[node_index] = root_node.Ite(high_node, low_node);
        }

        return add_table[node_index];
    }

}

ExplicitStateDfa ExplicitStateDfa::read_from_file(
    std::shared_ptr<VarMgr> var_mgr,
    const std::string& filename) {
  std::string explicit_state_dfa_filename = safetyltl_to_dfa_file(filename);
  std::ifstream in(explicit_state_dfa_filename);

  std::size_t line_number = 1;
  std::string line;
  std::getline(in, line);

  if (line != "MONA DFA") {
    throw bad_file_format_exception(line_number);
  }

  ++line_number;
  std::getline(in, line); // variable count; can be ignored
  
  ++line_number;
  std::vector<std::string> variable_names =
    read_variable_names(in, line_number);
  var_mgr->create_named_variables(variable_names);
  
  ++line_number;
  std::getline(in, line); // variable orders; can be ignored

  ++line_number;
  std::size_t state_count = read_state_count(in, line_number);

  ++line_number;
  std::getline(in, line); // initial state; can be ignored (always 0)
  std::size_t initial_state = 1; // state 0 is a dummy state, so ignore it

  ++line_number;
  std::getline(in, line); // node count; can be ignored

  ++line_number;
  std::vector<std::size_t> final_states = read_final_states(in, line_number);

  ++line_number;
  std::vector<std::size_t> behaviour = read_behaviour(in, line_number);

  ++line_number;
  std::vector<std::vector<int>> node_table = read_node_table(in, line_number);

  std::unordered_map<std::size_t, CUDD::ADD> add_table;
  std::vector<CUDD::ADD> transition_function(state_count);

  for (std::size_t i = 0; i < state_count; ++i) {
    transition_function[i] = build_add(behaviour[i], var_mgr, variable_names,
				       node_table, add_table);
  }

  // Turn state 0 into a sink state
  transition_function[0] = var_mgr->cudd_mgr()->constant(0);

  ExplicitStateDfa dfa(std::move(var_mgr));
  dfa.initial_state_ = initial_state;
  dfa.state_count_ = state_count;
  dfa.final_states_ = std::move(final_states);
  dfa.transition_function_ = std::move(transition_function);

  return dfa;
}

ExplicitStateDfa ExplicitStateDfa::complement_dfa(ExplicitStateDfa& d) {
    std::vector<size_t> final_states;
    std::vector<size_t> current_final_states = d.final_states();
    for (size_t i = d.initial_state(); i < d.state_count(); i++){
        if(std::find(current_final_states.begin(), current_final_states.end(), i) == current_final_states.end()){
            final_states.push_back(i);
        }
    }


    ExplicitStateDfa dfa(d.var_mgr());
    dfa.initial_state_ = d.initial_state();
    dfa.state_count_ = d.state_count();
    dfa.final_states_ = std::move(final_states);
    dfa.transition_function_ = d.transition_function();

    return dfa;
}

std::shared_ptr<VarMgr> ExplicitStateDfa::var_mgr() const {
  return var_mgr_;
}

std::size_t ExplicitStateDfa::initial_state() const {
  return initial_state_;
}

std::size_t ExplicitStateDfa::state_count() const {
  return state_count_;
}

std::vector<std::size_t> ExplicitStateDfa::final_states() const {
  return final_states_;
}

std::vector<CUDD::ADD> ExplicitStateDfa::transition_function() const {
  return transition_function_;
}

void ExplicitStateDfa::dump_dot(const std::string& filename) const {
  std::vector<std::string> function_labels(state_count_);

  for (std::size_t i = 0; i < state_count_; ++i) {
	  function_labels[i] = "S" + std::to_string(i);
  }

  var_mgr_->dump_dot(transition_function_, function_labels, filename);
}

};

#include "game/Transducer_multiagent.h"

#include <cstring>

namespace Syft {

Transducer_multiagent::Transducer_multiagent(const std::shared_ptr<VarMgr>& var_mgr,
                       const std::vector<int>& initial_vector,
                       const std::unordered_map<int, CUDD::BDD>& output_function,
                       const std::vector<CUDD::BDD>& transition_function,
                       Actor starting_actor,
                       Actor protagonist_actor)
    : var_mgr_(var_mgr),
    initial_vector_(initial_vector),
    output_function_(output_function),
    transition_function_(transition_function),
    starting_actor_(starting_actor),
    protagonist_actor_(protagonist_actor)
{}

void Transducer_multiagent::dump_dot(const std::string& filename) const {
  //var_mgr_->print_mgr();
  std::vector<std::string> actor_labels;
  //std::cout << "Current actor: " << (protagonist_actor_.is_environment() ? "ENV" : "Agent " + std::to_string(protagonist_actor_.id())) << std::endl;

  if (protagonist_actor_.is_environment()) {
    actor_labels = var_mgr_->input_variable_labels();
  } else {
    actor_labels = var_mgr_->agent_variable_labels(protagonist_actor_.id());
  }

  std::size_t output_count = output_function_.size();
  std::vector<CUDD::ADD> output_vector(output_count);

  for (std::size_t i = 0; i < output_count; ++i) {
    std::string label = actor_labels[i];
    //std::cout << "Label: " << label << std::endl;
    int index = var_mgr_->name_to_variable(label).NodeReadIndex();
    //std::cout << "Index: " << index << std::endl;
    output_vector[i] = output_function_.at(index).Add();
  }

  var_mgr_->dump_dot(output_vector, actor_labels, filename);
}   

}

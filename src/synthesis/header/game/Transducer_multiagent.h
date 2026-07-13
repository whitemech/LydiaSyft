#ifndef TRANSDUCER_MULTIAGENT_H
#define TRANSDUCER_MULTIAGENT_H

#include <memory>
#include <unordered_map>
#include <vector>

#include <cuddObj.hh>

#include "Actor.h"
#include "VarMgr.h"

namespace Syft {

/**
 * \brief A symbolic tranducer representing a winning strategy for a game.
 *
 * May be either a Moore or Mealy machine.
 */
class Transducer_multiagent {
 private:

  const std::shared_ptr<VarMgr> var_mgr_;
  const std::vector<int> initial_vector_;
  const std::unordered_map<int, CUDD::BDD> output_function_;
  const std::vector<CUDD::BDD> transition_function_;
  const Actor starting_actor_;
  const Actor protagonist_actor_;

 public:

  Transducer_multiagent(const std::shared_ptr<VarMgr>& var_mgr,
             const std::vector<int>& initial_vector,
             const std::unordered_map<int, CUDD::BDD>& output_function,
             const std::vector<CUDD::BDD>& transition_function,
             Actor starting_actor,
             Actor protagonist_actor);

  /**
   * \brief Saves the output function of the transducer in a .dot file.
   */
  void dump_dot(const std::string& filename) const;

  std::unordered_map<int, CUDD::BDD> get_output_function() const {
    return output_function_;
  }

  std::vector<CUDD::BDD> get_transition_function() const {
    return transition_function_;
  }
  std::vector<int> get_initial_vector() const {
    return initial_vector_;
  }
  const std::unordered_map<int, CUDD::BDD>& output_function() const {
    return output_function_;
  }
};

}

#endif // TRANSDUCER_MULTIAGENT_H

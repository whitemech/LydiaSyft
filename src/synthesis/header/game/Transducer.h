#ifndef TRANSDUCER_H
#define TRANSDUCER_H

#include <memory>
#include <unordered_map>
#include <vector>

#include <cuddObj.hh>

#include "Player.h"
#include "VarMgr.h"

namespace Syft {

/**
 * \brief A symbolic tranducer representing a winning strategy for a game.
 *
 * May be either a Moore or Mealy machine.
 */
class Transducer {
 private:

  const std::shared_ptr<VarMgr>& var_mgr_;
  const std::vector<int>& initial_vector_;
  const std::unordered_map<int, CUDD::BDD>& output_function_;
  const std::vector<CUDD::BDD>& transition_function_;
  const Player starting_player_;
  const Player protagonist_player_;

 public:

  Transducer(const std::shared_ptr<VarMgr>& var_mgr,
             const std::vector<int>& initial_vector,
             const std::unordered_map<int, CUDD::BDD>& output_function,
             const std::vector<CUDD::BDD>& transition_function,
             Player starting_player,
             Player protagonist_player = Player::Agent);

  /**
   * \brief Saves the output function of the transducer in a .dot file.
   */
  void dump_dot(const std::string& filename) const;
};

}

#endif // TRANSDUCER_H

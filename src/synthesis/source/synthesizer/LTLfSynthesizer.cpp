#include "synthesizer/LTLfSynthesizer.h"
#include "game/Reachability.hpp"

#include <cassert>

namespace Syft {

    LTLfSynthesizer::LTLfSynthesizer(SymbolicStateDfa spec,
                                     Player starting_player, Player protagonist_player,
                                     CUDD::BDD goal_states,
                                     CUDD::BDD state_space)
            : spec_(spec), starting_player_(starting_player), protagonist_player_(protagonist_player),
              goal_states_(goal_states),
              state_space_(state_space) {
        var_mgr_ = spec_.var_mgr();
    }


    SynthesisResult LTLfSynthesizer::run() const {
        Reachability solver(spec_, starting_player_, protagonist_player_,
                            goal_states_, state_space_);
        return solver.run();

    }

}

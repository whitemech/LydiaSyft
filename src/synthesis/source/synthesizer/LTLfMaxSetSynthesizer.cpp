//
// Created by shuzhu on 20/01/24.
//
#include "synthesizer/LTLfMaxSetSynthesizer.h"
#include "game/ReachabilityMaxSet.hpp"
#include "game/DfaGameSynthesizer.h"

#include <cassert>

namespace Syft {

    LTLfMaxSetSynthesizer::LTLfMaxSetSynthesizer(SymbolicStateDfa spec,
                                                 Player starting_player, Player protagonist_player,
                                                 CUDD::BDD goal_states,
                                                 CUDD::BDD state_space)
            : spec_(spec), starting_player_(starting_player), protagonist_player_(protagonist_player),
              goal_states_(goal_states),
              state_space_(state_space) {
        var_mgr_ = spec_.var_mgr();
//        maxset.deferring_strategy = var_mgr_->cudd_mgr()->bddZero();
//        maxset.nondeferring_strategy = var_mgr_->cudd_mgr()->bddZero();
    }


    MaxSetSynthesisResult LTLfMaxSetSynthesizer::run() const {
        ReachabilityMaxSet solver(spec_, starting_player_, protagonist_player_,
                                  goal_states_, state_space_);
        return solver.run_maxset();
    }

    void LTLfMaxSetSynthesizer::dump_dot(MaxSetSynthesisResult maxset, const std::string &def_filename,
                                         const std::string &nondef_filename) const {
        var_mgr_->dump_dot(maxset.deferring_strategy.Add(), def_filename);
        var_mgr_->dump_dot(maxset.nondeferring_strategy.Add(), nondef_filename);
    }


}
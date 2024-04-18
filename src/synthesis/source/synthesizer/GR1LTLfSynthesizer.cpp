//
// Created by shuzhu on 21/01/24.
//
#include "synthesizer/GR1LTLfSynthesizer.h"
#include "string_utilities.h"
#include "game/coGR1Reachability.hpp"
#include <iostream>


namespace Syft {

    GR1LTLfSynthesizer::GR1LTLfSynthesizer(const std::shared_ptr<VarMgr> &var_mgr, const GR1 &gr1,
                                           const SymbolicStateDfa &env_safety,
                                           const SymbolicStateDfa &agn_reach,
                                           const SymbolicStateDfa &agn_safety,
                                           const std::string &slugs_dir,
                                           const std::string &benchmark_name)
            : var_mgr_(var_mgr), gr1_(gr1), env_safety_(env_safety), agn_reach_(agn_reach),
              agn_safety_(agn_safety), slugs_dir_{slugs_dir}, benchmark_name_{benchmark_name} {}


    SynthesisResult GR1LTLfSynthesizer::run() const {
        Syft::Stopwatch reduction_to_gr1_stopwatch; // stopwatch for reducing to GR1
        reduction_to_gr1_stopwatch.start();
        std::cout << "* Start reducing to GR1 game...\n";

        SymbolicStateDfa reach_safe = SymbolicStateDfa::product_AND({agn_reach_, agn_safety_});

//        CUDD::BDD reach_goal_t1 = agn_reach_.final_states();
        CUDD::BDD safe_region_t2 = agn_safety_.final_states() | agn_safety_.initial_state_bdd();

        reach_safe.new_sink_states(!safe_region_t2);

        SymbolicStateDfa arena = SymbolicStateDfa::product_AND({env_safety_, reach_safe});
        CUDD::BDD arena_initial_state_bdd =
                agn_reach_.initial_state_bdd() & agn_safety_.initial_state_bdd() & env_safety_.initial_state_bdd();

        CUDD::BDD safe_states = (env_safety_.final_states() & !(reach_safe.final_states())) | arena_initial_state_bdd;

        auto reduction_to_gr1_time = reduction_to_gr1_stopwatch.stop();
        std::cout << "* Finish reducing to GR1 game, took time: "
                  << reduction_to_gr1_time.count() << " ms" << std::endl;

        coGR1Reachability solver(var_mgr_, gr1_, arena, safe_states, arena_initial_state_bdd, slugs_dir_,
                                 benchmark_name_);

        return solver.run();
    }


}

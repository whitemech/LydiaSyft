//
// Created by shuzhu on 16/04/24.
//

#include "game/BuchiReachability_multiagent.hpp"
#include "game/DfaGameSynthesizer_multiagent.h"

namespace Syft {
    BuchiReachability_multiagent::BuchiReachability_multiagent(const SymbolicStateDfa &spec, Actor starting_actor,
                                         Actor protagonist_actor,
                                         const CUDD::BDD &Buchi, 
                                         const CUDD::BDD &state_space,
                                         size_t num_agents)
            : DfaGameSynthesizer_multiagent(spec, starting_actor, protagonist_actor, num_agents), 
              Buchi_(Buchi),
              state_space_(state_space) {
    }

    SynthesisResult BuchiReachability_multiagent::run() const {
        SynthesisResult result;
        CUDD::BDD winning_states = state_space_;
        CUDD::BDD winning_moves = winning_states;
        int c = 0;

        while (true) {

            var_mgr_->dump_dot(winning_states.Add(), "winning_states_"+ std::to_string(c)+".dot");
            var_mgr_->dump_dot(winning_moves.Add(), "winning_moves_"+ std::to_string(c)+".dot");

          
            CUDD::BDD new_winning_states, new_winning_moves;
            // Outer greatest fixpoint: W_{i+1} 
            CUDD::BDD pre_adv_W = preimage(winning_states);
            // Inner least fixpoint initialization: Z_{0} 
            CUDD::BDD projection = project_into_states(pre_adv_W);
            CUDD::BDD inner_winning_states = Buchi_ & projection;
            //CUDD::BDD inner_winning_moves = inner_winning_states;
            CUDD::BDD inner_winning_moves = preimage(inner_winning_states); 
            //int inner_c = 0;

            while (true) {
                // var_mgr_->dump_dot(inner_winning_states.Add(), "inner_winning_states_"+ std::to_string(c)+".dot");
                // var_mgr_->dump_dot(inner_winning_moves.Add(), "inner_winning_moves_"+ std::to_string(c)+".dot");

                
                CUDD::BDD new_inner_winning_states, new_inner_winning_moves;
                // PreAdv(Z_j) - compute preimage of inner_winning_states
                CUDD::BDD pre_adv_Z = preimage(inner_winning_states);
                if(starting_actor_.is_agent()){
                    new_inner_winning_moves = inner_winning_moves | 
                    (state_space_ & (!inner_winning_states) & pre_adv_Z);
                    new_inner_winning_states = project_into_states(new_inner_winning_moves);
                }else{
                    // Inner least fixpoint: Z_{j+1} 
                    CUDD::BDD new_collected_winning_states = project_into_states(pre_adv_Z);
                    new_inner_winning_states = inner_winning_states | new_collected_winning_states;
                    new_inner_winning_moves = inner_winning_moves | 
                                              ((!inner_winning_states) & new_collected_winning_states 
                                              & pre_adv_Z); 
                }               
                
                // Check for fixpoint of inner least fixpoint
                if (new_inner_winning_states == inner_winning_states) {
                    if(starting_actor_.is_agent()){
                        new_winning_moves = winning_moves & new_inner_winning_moves;
                        new_winning_states = winning_states & inner_winning_states;
                    }else{
                        CUDD::BDD transition_to_winning_states = preimage(inner_winning_states);
                        new_winning_states = winning_states & inner_winning_states;
                        new_winning_moves = winning_moves & transition_to_winning_states;                        
                    }
                    break;
                }

                inner_winning_moves = new_inner_winning_moves;
                inner_winning_states = new_inner_winning_states;
                //inner_c++;
            }      

            if(new_winning_states == winning_states){
                
                
                if (includes_initial_state(new_winning_states)) {
                    result.realizability = true;
                    result.winning_states = new_winning_states;
                    result.winning_moves = new_winning_moves;
                    result.transducer_multiagent = AbstractSingleStrategy(result);
                    result.transducer = nullptr;
                    return result;

                } else {
                    result.realizability = false;
                    result.winning_states = new_winning_states;
                    result.winning_moves = new_winning_moves;
                    result.transducer_multiagent = nullptr;
                    result.transducer = nullptr;
                    return result;
                }
            }

            winning_moves = new_winning_moves;
            winning_states = new_winning_states;
            c++;
        }
    }

}


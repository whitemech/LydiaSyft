//
// Created by shuzhu on 16/04/24.
//

#include "game/Reachability_multiagent.hpp"

namespace Syft {
    Reachability_multiagent::Reachability_multiagent(const SymbolicStateDfa &spec, Actor starting_actor,
                               Actor protagonist_actor, const CUDD::BDD &goal_states,
                               const CUDD::BDD &state_space, size_t num_agents)
            : DfaGameSynthesizer_multiagent(spec, starting_actor, protagonist_actor, num_agents), 
              goal_states_(goal_states),
              state_space_(state_space) {
    }

    SynthesisResult Reachability_multiagent:: run() const {
        SynthesisResult result;
        CUDD::BDD winning_states = state_space_ & goal_states_;
        CUDD::BDD winning_moves = winning_states;

        while (true) {
            CUDD::BDD new_winning_states, new_winning_moves;

            if (starting_actor_.is_agent()){
                CUDD::BDD quantified_X_transitions_to_winning_states = preimage(winning_states);
                new_winning_moves = winning_moves |
                                    (state_space_ & (!winning_states) & quantified_X_transitions_to_winning_states);

                new_winning_states = project_into_states(new_winning_moves);
            } else {
                CUDD::BDD transitions_to_winning_states = preimage(winning_states);
                CUDD::BDD new_collected_winning_states = project_into_states(transitions_to_winning_states);
                new_winning_states = winning_states | new_collected_winning_states;
                new_winning_moves = winning_moves |
                                    ((!winning_states) & new_collected_winning_states & transitions_to_winning_states);
            }

            if (includes_initial_state(new_winning_states)) {
                result.realizability = true;
                result.winning_states = new_winning_states;
                result.winning_moves = new_winning_moves;
                result.transducer_multiagent = AbstractSingleStrategy(result);
                result.transducer = nullptr;
                return result;

            } else if (new_winning_states == winning_states) {
                result.realizability = false;
                result.winning_states = new_winning_states;
                result.winning_moves = new_winning_moves;
                result.transducer_multiagent = nullptr;
                result.transducer = nullptr;
                return result;
            }

            winning_moves = new_winning_moves;
            winning_states = new_winning_states;
        }
    }

}


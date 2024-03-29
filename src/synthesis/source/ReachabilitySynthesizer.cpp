#include "ReachabilitySynthesizer.h"

#include <cassert>

namespace Syft {

    ReachabilitySynthesizer::ReachabilitySynthesizer(SymbolicStateDfa spec,
                                                     Player starting_player, Player protagonist_player,
                                                     CUDD::BDD goal_states,
                                                     CUDD::BDD state_space)
            : DfaGameSynthesizer(spec, starting_player, protagonist_player)
            , goal_states_(goal_states), state_space_(state_space)
    {}


    SynthesisResult ReachabilitySynthesizer::run() const {
        SynthesisResult result;
        CUDD::BDD winning_states = state_space_ & goal_states_;
        CUDD::BDD winning_moves = winning_states;

        while (true) {
            CUDD::BDD new_winning_states, new_winning_moves;

            if (starting_player_ == Player::Agent){
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
                result.transducer = nullptr;
                return result;

            } else if (new_winning_states == winning_states) {
                result.realizability = false;
                result.winning_states = new_winning_states;
                result.winning_moves = new_winning_moves;
                result.transducer = nullptr;
                return result;
            }

            winning_moves = new_winning_moves;
            winning_states = new_winning_states;
        }

    }

}

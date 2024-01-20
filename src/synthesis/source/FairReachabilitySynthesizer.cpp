//
// Created by shuzhu on 20/01/24.
//

#include "FairReachabilitySynthesizer.h"

#include <cassert>

namespace Syft {

    FairReachabilitySynthesizer::FairReachabilitySynthesizer(SymbolicStateDfa spec,
                                                     Player starting_player, Player protagonist_player,
                                                     CUDD::BDD goal_states,
                                                     CUDD::BDD state_space,
                                                     CUDD::BDD assumption)
            : DfaGameSynthesizer(spec, starting_player, protagonist_player)
            , goal_states_(goal_states), state_space_(state_space), assumption_(assumption)
    {}


    SynthesisResult FairReachabilitySynthesizer::run() const {
        SynthesisResult result;
        CUDD::BDD winning_states = state_space_ & goal_states_;
        CUDD::BDD winning_moves = winning_states;

        while (true) {
            CUDD::BDD new_winning_states, new_winning_moves;
             // inner greatest fixpoint
                CUDD::BDD inner_winning_states = state_space_;
                CUDD::BDD inner_winning_moves = inner_winning_states;
                while (true) {
                    CUDD::BDD new_inner_winning_states, new_inner_winning_moves;

                    CUDD::BDD transitions_to_winning_states_or_goal =
                            (winning_states | goal_states_).VectorCompose(transition_vector_);
                    CUDD::BDD transitions_to_inner_winning_states_or_goal =
                            (inner_winning_states | goal_states_).VectorCompose(transition_vector_);
                    CUDD::BDD assumption_constrained_transitions_to_inner_winning_states = ((!assumption_) | transitions_to_winning_states_or_goal) * transitions_to_inner_winning_states_or_goal;
                    if (starting_player_ == Player::Agent) {
                        // Quantify all variables that the outputs don't depend on
                        CUDD::BDD quantified_X_transitions_to_inner_winning_states = quantify_independent_variables_->apply(assumption_constrained_transitions_to_inner_winning_states);
                        new_inner_winning_moves = inner_winning_moves & quantified_X_transitions_to_inner_winning_states;

                        new_inner_winning_states = project_into_states(new_inner_winning_moves);
                    } else {
                        CUDD::BDD transitions_to_inner_winning_states = assumption_constrained_transitions_to_inner_winning_states;
                        CUDD::BDD new_collected_inner_winning_states = project_into_states(
                                transitions_to_inner_winning_states);
                        new_inner_winning_states = inner_winning_states & new_collected_inner_winning_states;
                        new_inner_winning_moves = inner_winning_moves & transitions_to_inner_winning_states;
                    }

                    if (new_inner_winning_states == inner_winning_states) {
                        if (starting_player_ == Player::Agent) {
                            new_winning_moves = winning_moves | new_inner_winning_moves;
                            new_winning_states = project_into_states(new_winning_moves);
                        } else {
                            CUDD::BDD transitions_to_winning_states = preimage(winning_states);
                            CUDD::BDD new_collected_winning_states = project_into_states(transitions_to_winning_states);
                            new_winning_states = winning_states | new_collected_winning_states;
                            new_winning_moves = winning_moves |
                                                ((!winning_states) & new_inner_winning_states & transitions_to_winning_states);
                        }
                        break;
                    }

                    inner_winning_moves = new_inner_winning_moves;
                    inner_winning_states = new_inner_winning_states;
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

    std::unique_ptr<Transducer> FairReachabilitySynthesizer::AbstractSingleStrategy(SynthesisResult result) const {
        std::unordered_map<int, CUDD::BDD> strategy = synthesize_strategy(
                result.winning_moves);

        auto transducer = std::make_unique<Transducer>(
                var_mgr_, initial_vector_, strategy, spec_.transition_function(),
                starting_player_);
        return transducer;
    }



}
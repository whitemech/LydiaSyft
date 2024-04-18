//
// Created by shuzhu on 16/04/24.
//

#include "game/coBuchiReachability.hpp"

namespace Syft {
    coBuchiReachability::coBuchiReachability(const SymbolicStateDfa &spec, Player starting_player,
                                             Player protagonist_player,
                                             const CUDD::BDD &goal_states, const CUDD::BDD &coBuchi,
                                             const CUDD::BDD &state_space)
            : DfaGameSynthesizer(spec, starting_player, protagonist_player), goal_states_(goal_states),
              coBuchi_(coBuchi),
              state_space_(state_space) {
    }

    SynthesisResult coBuchiReachability::run() const {
        SynthesisResult result;
        CUDD::BDD winning_states = state_space_ & goal_states_;
        CUDD::BDD winning_moves = winning_states;
        int c = 0;
        int inner_c = 0;

        while (true) {
            CUDD::BDD new_winning_states, new_winning_moves;
            // inner greatest fixpoint
            CUDD::BDD inner_winning_states = state_space_;
            CUDD::BDD inner_winning_moves = inner_winning_states;
            while (true) {
                CUDD::BDD new_inner_winning_states, new_inner_winning_moves;

                CUDD::BDD transitions_to_winning_states_or_goal =
                        (winning_states | goal_states_).VectorCompose(transition_vector_);
//                    var_mgr_->dump_dot(transitions_to_winning_states_or_goal.Add(), "transitions_to_winning_states_or_goal.dot");
                CUDD::BDD transitions_to_inner_winning_states_or_goal =
                        (inner_winning_states | goal_states_).VectorCompose(transition_vector_);
//                    var_mgr_->dump_dot(transitions_to_inner_winning_states_or_goal.Add(), "transitions_to_inner_winning_states_or_goal.dot");
//                    var_mgr_->dump_dot(assumption_.Add(), "assumption.dot");
                CUDD::BDD assumption_constrained_transitions =
                        ((coBuchi_ | transitions_to_winning_states_or_goal)) *
                        transitions_to_inner_winning_states_or_goal;
//                    var_mgr_->dump_dot(assumption_constrained_transitions.Add(), "assumption_constrained_transitions.dot");
                if (starting_player_ == Player::Agent) {
                    // Quantify all variables that the outputs don't depend on
                    CUDD::BDD quantified_X_transitions_to_inner_winning_states = quantify_independent_variables_->apply(
                            assumption_constrained_transitions);
                    new_inner_winning_moves = inner_winning_moves & quantified_X_transitions_to_inner_winning_states;

                    new_inner_winning_states = project_into_states(new_inner_winning_moves);
                } else {
                    new_inner_winning_moves = inner_winning_moves & quantify_independent_variables_->apply(
                            assumption_constrained_transitions);
//                        var_mgr_->dump_dot((quantify_independent_variables_->apply(assumption_constrained_transitions)).Add(), "quantified_assumption_constrained_transitions.dot");
                    new_inner_winning_states = project_into_states(
                            new_inner_winning_moves);
//                        var_mgr_->dump_dot(new_inner_winning_states.Add(), "states.dot");

                }

                if (new_inner_winning_states == inner_winning_states) {
                    if (starting_player_ == Player::Agent) {
                        CUDD::BDD transitions_to_winning_states = preimage(inner_winning_states);
                        CUDD::BDD quantified_X_transitions_to_inner_winning_states = quantify_independent_variables_->apply(
                                transitions_to_winning_states);
                        new_inner_winning_moves = (!winning_states) & inner_winning_moves &
                                                  quantified_X_transitions_to_inner_winning_states;
                        new_winning_moves = winning_moves | new_inner_winning_moves;
                        new_winning_states = winning_states | inner_winning_states;
                    } else {
                        CUDD::BDD transitions_to_winning_states = preimage(inner_winning_states);
                        new_winning_states = winning_states | inner_winning_states;
                        new_winning_moves = winning_moves |
                                            ((!winning_states) & new_winning_states & transitions_to_winning_states);
                    }
                    break;
                }

                inner_winning_moves = new_inner_winning_moves;
                inner_winning_states = new_inner_winning_states;
                inner_c++;
            }


            if (includes_initial_state(new_winning_states)) {
                result.realizability = true;
                result.winning_states = new_winning_states;
                result.winning_moves = new_winning_moves;
                result.transducer = AbstractSingleStrategy(result);
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
            c++;
        }

    }

}

//
// Created by shuzhu on 20/01/24.
//

#include "FairReachabilitySynthesizer.h"
#include "string_utilities.h"

#include <fstream>
#include <cassert>

namespace Syft {

    FairReachabilitySynthesizer::FairReachabilitySynthesizer(SymbolicStateDfa spec,
                                                     Player starting_player, Player protagonist_player,
                                                     CUDD::BDD goal_states,
                                                     CUDD::BDD state_space,
                                                     std::string& assumption_filename)
            : DfaGameSynthesizer(spec, starting_player, protagonist_player)
            , goal_states_(goal_states), state_space_(state_space)
    {
        assumption_ = load_CNF(assumption_filename);
    }


    SynthesisResult FairReachabilitySynthesizer::run() const {
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
                    CUDD::BDD assumption_constrained_transitions = ((!assumption_ | transitions_to_winning_states_or_goal)) * transitions_to_inner_winning_states_or_goal;
//                    var_mgr_->dump_dot(assumption_constrained_transitions.Add(), "assumption_constrained_transitions.dot");
                    if (starting_player_ == Player::Agent) {
                        // Quantify all variables that the outputs don't depend on
                        CUDD::BDD quantified_X_transitions_to_inner_winning_states = quantify_independent_variables_->apply(assumption_constrained_transitions);
                        new_inner_winning_moves = inner_winning_moves & quantified_X_transitions_to_inner_winning_states;

                        new_inner_winning_states = project_into_states(new_inner_winning_moves);
                    } else {
                        new_inner_winning_moves = inner_winning_moves & quantify_independent_variables_->apply(assumption_constrained_transitions);
//                        var_mgr_->dump_dot((quantify_independent_variables_->apply(assumption_constrained_transitions)).Add(), "quantified_assumption_constrained_transitions.dot");
                        new_inner_winning_states = project_into_states(
                                new_inner_winning_moves);
//                        var_mgr_->dump_dot(new_inner_winning_states.Add(), "states.dot");

                    }

                    if (new_inner_winning_states == inner_winning_states) {
                        if (starting_player_ == Player::Agent) {
                            CUDD::BDD transitions_to_winning_states = preimage(inner_winning_states);
                            CUDD::BDD quantified_X_transitions_to_inner_winning_states = quantify_independent_variables_->apply(transitions_to_winning_states);
                            new_inner_winning_moves = (!winning_states) & inner_winning_moves & quantified_X_transitions_to_inner_winning_states;
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
            c++;
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

    CUDD::BDD FairReachabilitySynthesizer::load_CNF(const std::string& filename) const{
        std::ifstream f(filename.c_str());
        CUDD::BDD assumption = var_mgr_->cudd_mgr()->bddOne();
        std::vector<std::string> clause_line;
        std::string line;
        while(getline(f, line)) {
            CUDD::BDD clause = var_mgr_->cudd_mgr()->bddZero();
            std::set<std::string> clause_set;
            std::set<std::string> neg_clause_set;

            if (f.is_open()) {
                clause_line = split(line, " ");

                for (int i = 0; i < clause_line.size(); i++){
                    std::string c = clause_line[i];
                    CUDD::BDD literal_bdd;
                    if(c[0] == '!'){
                        std::string var_name = c.substr(1, c.length()-1);
                        literal_bdd = !(var_mgr_->name_to_variable(var_name));
                    }
                    else{
                        literal_bdd = var_mgr_->name_to_variable(c);
                    }
                    clause = clause + literal_bdd;
                }
            }
            if(clause != var_mgr_->cudd_mgr()->bddZero()){
                assumption = assumption * clause;
            }

        }
        f.close();
        return assumption;

    }

}
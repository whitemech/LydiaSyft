//
// Created by shuzhu on 21/01/24.
//


#include "synthesizer/StabilityLtlfSynthesizer.h"
#include "game/BuchiReachability.hpp"
#include "string_utilities.h"

#include <fstream>

namespace Syft {

    StabilityLtlfSynthesizer::StabilityLtlfSynthesizer(SymbolicStateDfa spec,
                                                       Player starting_player, Player protagonist_player,
                                                       CUDD::BDD goal_states,
                                                       CUDD::BDD state_space,
                                                       const std::string &assumption_filename)
            : spec_(spec), starting_player_(starting_player), protagonist_player_(protagonist_player),
              goal_states_(goal_states),
              state_space_(state_space) {
        var_mgr_ = spec_.var_mgr();
        assumption_ = load_CNF(assumption_filename);
    }


    SynthesisResult StabilityLtlfSynthesizer::run() const {
        BuchiReachability solver(spec_, starting_player_, protagonist_player_,
                                 goal_states_, !assumption_, state_space_);
        return solver.run();
    }


    CUDD::BDD StabilityLtlfSynthesizer::load_CNF(const std::string &filename) const {
        std::ifstream f(filename.c_str());
        CUDD::BDD assumption = var_mgr_->cudd_mgr()->bddOne();
        std::vector<std::string> clause_line;
        std::string line;
        while (getline(f, line)) {
            CUDD::BDD clause = var_mgr_->cudd_mgr()->bddZero();
            std::set<std::string> clause_set;
            std::set<std::string> neg_clause_set;

            if (f.is_open()) {
                clause_line = split(line, " ");

                for (int i = 0; i < clause_line.size(); i++) {
                    std::string c = clause_line[i];
                    CUDD::BDD literal_bdd;
                    if (c[0] == '!') {
                        std::string var_name = c.substr(1, c.length() - 1);
                        literal_bdd = !(var_mgr_->name_to_variable(var_name));
                    } else {
                        literal_bdd = var_mgr_->name_to_variable(c);
                    }
                    clause = clause + literal_bdd;
                }
            }
            if (clause != var_mgr_->cudd_mgr()->bddZero()) {
                assumption = assumption * clause;
            }

        }
        f.close();
        return assumption;

    }

}

//
// Created by marcofavorito on 22/01/24.
//

#include <CLI/CLI.hpp>

#include "synthesis.hpp"

#include "Stopwatch.h"

#include "ExplicitStateDfaMona.h"
#include "ReachabilityMaxSetSynthesizer.h"
#include "InputOutputPartition.h"
#include "Preprocessing.h"
#include "gr1.hpp"
#include "GR1.h"
#include "GR1ReachabilitySynthesizer.h"

#include <lydia/parser/ltlf/driver.hpp>


namespace Syft {

    void Syft::GR1Runner::run() const {
        Syft::Stopwatch total_time_stopwatch;
        total_time_stopwatch.start();

        // proceed with DFAs construction
        auto agent_safety_dfa = do_dfa_construction_with_message_(*agent_safety_, var_mgr_, "Agent Safety DFA construction time");
        auto env_safety_dfa = do_dfa_construction_with_message_(*env_safety_, var_mgr_, "Env Safety DFA construction time");
        auto agent_goal_dfa = do_dfa_construction_with_message_(*args_.formula, var_mgr_, "Agent Goal DFA construction time");

        // do GR(1) synthesis
        do_gr1_synthesis_(agent_safety_dfa, env_safety_dfa, agent_goal_dfa);

        auto total_time = total_time_stopwatch.stop();
        printer_.print_times_if_enabled("Total time", total_time);
    }

    Syft::SymbolicStateDfa GR1Runner::do_dfa_construction_with_message_(const whitemech::lydia::LTLfFormula& formula, const std::shared_ptr<Syft::VarMgr>& var_mgr, const std::string& msg) const {
        Stopwatch aut_time_stopwatch_;
        aut_time_stopwatch_.start();
        auto symbolic_dfa = do_dfa_construction(formula, var_mgr);
        auto aut_time = aut_time_stopwatch_.stop();
        printer_.print_times_if_enabled(msg, aut_time);
        return symbolic_dfa;
    }

    void GR1Runner::do_gr1_synthesis_(const SymbolicStateDfa& agent_safety_dfa, const SymbolicStateDfa& env_safety_dfa, const SymbolicStateDfa& agent_goal_dfa) const {
        Syft::GR1 gr1 = Syft::GR1::read_from_gr1_file(var_mgr_, gr1_file_);
        Syft::GR1ReachabilitySynthesizer synthesizer(var_mgr_, gr1, env_safety_dfa,
                                                     agent_goal_dfa, agent_safety_dfa, path_to_slugs_, "problem");
        Syft::SynthesisResult result = synthesizer.run();
        handle_gr1_synthesis_result_(synthesizer, result);
    }

    void GR1Runner::handle_gr1_synthesis_result_(const GR1ReachabilitySynthesizer &synthesizer,
                                                 const SynthesisResult &result) const {
        if (result.realizability) {
            printer_.print_realizable();

            // TODO
            // // abstract single strategy
            // Syft::Stopwatch abstract_single_strategy_time_stopwatch;
            // abstract_single_strategy_time_stopwatch.start();
            // auto transducer = abstract_single_strategy(result.winning_moves, var_mgr_, ...);
            // auto abstract_single_strategy_time = abstract_single_strategy_time_stopwatch.stop();
            // printer_.print_times_if_enabled("Abstract single strategy time", abstract_single_strategy_time);
            // // dump strategy
            // printer_.dump_transducer_if_enabled(*transducer, "strategy.dot");
        } else {
            printer_.print_unrealizable();
        }
    }

    std::string read_assumption_file_if_file_specified_(const std::optional<std::string>& filename) {
        if (!filename.has_value()) {
            return "true";
        }
        std::ifstream file(filename.value());
        if (!file.good()) {
            throw std::runtime_error("File " + filename.value() + " not found");
        }
        std::string assumption_str;
        std::getline(file, assumption_str);
        return assumption_str;
    }

}

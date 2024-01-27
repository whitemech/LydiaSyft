//
// Created by marcofavorito on 22/01/24.
//

#include <CLI/CLI.hpp>

#include "synthesis.hpp"
#include <cstring>
#include <iostream>
#include <memory>

#include "Stopwatch.h"

#include "../parser/Parser.h"
#include "ExplicitStateDfaMona.h"
#include "ReachabilitySynthesizer.h"
#include "ReachabilityMaxSetSynthesizer.h"
#include "InputOutputPartition.h"
#include "Preprocessing.h"
#include "stability.hpp"
#include "StableReachabilitySynthesizer.h"

#include <lydia/parser/ltlf/driver.hpp>
#include <CLI/CLI.hpp>
#include <istream>


namespace Syft {

    void StabilityRunner::run() {
        Syft::Stopwatch total_time_stopwatch;
        total_time_stopwatch.start();

        // proceed with DFA construction
        auto symbolic_dfa = do_dfa_construction_();

        // do maxset synthesis
        do_stability_synthesis_(symbolic_dfa);

        auto total_time = total_time_stopwatch.stop();
        printer_.print_times_if_enabled("Total time", total_time);
    }

    void StabilityRunner::do_stability_synthesis_(const Syft::SymbolicStateDfa &symbolic_dfa) {
        Syft::StableReachabilitySynthesizer synthesizer(symbolic_dfa, args_.starting_player,
                                                        args_.protagonist_player, symbolic_dfa.final_states(),
                                                        var_mgr_->cudd_mgr()->bddOne(), assumption_filename_);
        Syft::SynthesisResult result = synthesizer.run();
        handle_synthesis_result_(synthesizer, result);
    }
}

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
#include "automata/ExplicitStateDfa.h"
#include "synthesizer/LTLfSynthesizer.h"
#include "synthesizer/LTLfMaxSetSynthesizer.h"
#include "game/InputOutputPartition.h"
#include "Preprocessing.h"
#include "stability.hpp"
#include "synthesizer/StabilityLtlfSynthesizer.h"

#include <lydia/parser/ltlf/driver.hpp>
#include <CLI/CLI.hpp>
#include <istream>


namespace Syft {

    void StabilityRunner::run() {
        Syft::Stopwatch total_time_stopwatch;
        total_time_stopwatch.start();

        // proceed with DFA construction
        auto symbolic_dfa = do_dfa_construction_();

        // do stability synthesis
        do_stability_synthesis_(symbolic_dfa);

        auto total_time = total_time_stopwatch.stop();
        printer_.print_times_if_enabled("Total time", total_time);
    }

    void StabilityRunner::do_stability_synthesis_(const Syft::SymbolicStateDfa &symbolic_dfa) {
        var_mgr_->partition_variables(args_.partition.input_variables, args_.partition.output_variables);
        Syft::StabilityLtlfSynthesizer synthesizer(symbolic_dfa, args_.starting_player,
                                                   args_.protagonist_player, symbolic_dfa.final_states(),
                                                   var_mgr_->cudd_mgr()->bddOne(), assumption_filename_);
        Syft::SynthesisResult result = synthesizer.run();
        handle_synthesis_result_(result);
    }
}

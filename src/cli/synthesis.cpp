//
// Created by marcofavorito on 22/01/24.
//

#include "synthesis.hpp"
#include "Stopwatch.h"
#include "synthesizer/LTLfSynthesizer.h"
#include "Preprocessing.h"
#include <lydia/parser/ltlf/driver.hpp>


namespace Syft {

    void SynthesisRunner::do_synthesis_(const SymbolicStateDfa &symbolic_dfa) {
        var_mgr_->partition_variables(args_.partition.input_variables, args_.partition.output_variables);
        Syft::LTLfSynthesizer synthesizer(symbolic_dfa, args_.starting_player,
                                          args_.protagonist_player, symbolic_dfa.final_states(),
                                          var_mgr_->cudd_mgr()->bddOne());
        Syft::SynthesisResult result = synthesizer.run();
        handle_synthesis_result_(result);
    }

    void SynthesisRunner::run() {
        Syft::Stopwatch total_time_stopwatch;
        total_time_stopwatch.start();

        // preprocessing
        auto one_step_result = preprocessing(*args_.formula, args_.partition, *var_mgr_, args_.starting_player);
        if (handle_preprocessing_result_(one_step_result, total_time_stopwatch)) {
            // preprocessing was successful
            return;
        }

        // proceed with DFA construction
        auto symbolic_dfa = do_dfa_construction_();

        // do synthesis
        do_synthesis_(symbolic_dfa);

        auto total_time = total_time_stopwatch.stop();
        printer_.print_times_if_enabled("Total time", total_time);
    }

}

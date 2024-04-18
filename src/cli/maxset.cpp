//
// Created by marcofavorito on 22/01/24.
//

#include "synthesis.hpp"
#include "Stopwatch.h"
#include "synthesizer/LTLfMaxSetSynthesizer.h"
#include "Preprocessing.h"
#include "maxset.hpp"
#include <lydia/parser/ltlf/driver.hpp>


namespace Syft {

    void MaxSetRunner::run() {
        Syft::Stopwatch total_time_stopwatch;
        total_time_stopwatch.start();

        // proceed with DFA construction
        auto symbolic_dfa = do_dfa_construction_();

        // do maxset synthesis
        do_maxset_synthesis(symbolic_dfa);

        auto total_time = total_time_stopwatch.stop();
        printer_.print_times_if_enabled("Total time", total_time);
    }

    void MaxSetRunner::do_maxset_synthesis(const SymbolicStateDfa &symbolic_dfa) {
        var_mgr_->partition_variables(args_.partition.input_variables, args_.partition.output_variables);

        Syft::LTLfMaxSetSynthesizer synthesizer(symbolic_dfa, args_.starting_player,
                                                args_.protagonist_player, symbolic_dfa.final_states(),
                                                var_mgr_->cudd_mgr()->bddOne());
        Syft::MaxSetSynthesisResult result = synthesizer.run();

        if (result.realizability) {
            printer_.print_realizable();
            printer_.dump_maxset_if_enabled(synthesizer, result, "def_strategy.dot", "nondef_strategy.dot");
        } else {
            printer_.print_unrealizable();
        }
    }

}

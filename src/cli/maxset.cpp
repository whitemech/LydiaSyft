//
// Created by marcofavorito on 22/01/24.
//

#include "synthesis.hpp"
#include "Stopwatch.h"
#include "ReachabilityMaxSetSynthesizer.h"
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

    void MaxSetRunner::do_maxset_synthesis(const SymbolicStateDfa& symbolic_dfa) {
        Syft::Stopwatch nondef_strategy_time_stopwatch; // stopwatch for strategy_generator construction
        nondef_strategy_time_stopwatch.start();

        var_mgr_->partition_variables(args_.partition.input_variables, args_.partition.output_variables);

        Syft::ReachabilityMaxSetSynthesizer synthesizer(symbolic_dfa, args_.starting_player,
                                                        args_.protagonist_player, symbolic_dfa.final_states(),
                                                        var_mgr_->cudd_mgr()->bddOne());
        Syft::SynthesisResult result = synthesizer.run();

        if (result.realizability) {
            printer_.print_realizable();

            auto nondef_strategy_time = nondef_strategy_time_stopwatch.stop();
            printer_.print_times_if_enabled("Nondeferring strategy generator construction time", nondef_strategy_time);

            Syft::Stopwatch def_strategy_time_stopwatch;
            def_strategy_time_stopwatch.start();
            Syft::MaxSet maxset_strategy = synthesizer.AbstractMaxSet(result);
            auto def_strategy_time = def_strategy_time_stopwatch.stop();
            printer_.print_times_if_enabled("Deferring strategy generator construction time", def_strategy_time);

            printer_.dump_maxset_if_enabled(synthesizer, maxset_strategy, "def_strategy.dot", "nondef_strategy.dot");
        }
        else{
            printer_.print_unrealizable();
        }
    }

}

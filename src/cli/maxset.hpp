//
// Created by marcofavorito on 22/01/24.
//

#ifndef LYDIASYFT_CLI_MAXSET_H
#define LYDIASYFT_CLI_MAXSET_H

#include <CLI/CLI.hpp>


namespace Syft {


    class MaxSetRunner : public BaseRunner {
        private:
            Syft::Stopwatch aut_time_stopwatch_; // stopwatch for automata construction
            Syft::Stopwatch nondef_strategy_time_stopwatch_;  // stopwatch for nondeferring strategy
            Syft::Stopwatch def_strategy_time_stopwatch_;  // stopwatch for deferring strategy

            void do_maxset_synthesis(const SymbolicStateDfa& dfa);
        public:
            MaxSetRunner(const std::shared_ptr<whitemech::lydia::parsers::ltlf::LTLfDriver>& driver, const std::string& formula_file, const std::string& path_to_syfco, bool print_strategy, bool print_times) :
            BaseRunner(driver, formula_file, path_to_syfco, print_strategy, print_times) {}
            void run();

    };


}


#endif //LYDIASYFT_CLI_MAXSET_H

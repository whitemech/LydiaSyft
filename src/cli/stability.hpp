//
// Created by marcofavorito on 22/01/24.
//

#ifndef LYDIASYFT_CLI_STABILITY_H
#define LYDIASYFT_CLI_STABILITY_H

#include <CLI/CLI.hpp>


namespace Syft {

    class StabilityRunner : public BaseRunner {
        private:
        std::string assumption_filename_;

        void do_stability_synthesis_(const SymbolicStateDfa &symbolic_dfa);

        public:
        StabilityRunner(const std::shared_ptr<whitemech::lydia::parsers::ltlf::LTLfDriver>& driver,
                        const std::string &formula_file, const std::string &path_to_syfco,
                        const std::string &assumption_filename, bool print_strategy, bool print_times) : BaseRunner(
            driver, formula_file, path_to_syfco, print_strategy, print_times), assumption_filename_(assumption_filename) {}

        void run();

    };
}

#endif //LYDIASYFT_CLI_STABILITY_H
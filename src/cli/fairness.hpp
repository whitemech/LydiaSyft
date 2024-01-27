//
// Created by marcofavorito on 22/01/24.
//

#ifndef LYDIASYFT_CLI_FAIRNESS_H
#define LYDIASYFT_CLI_FAIRNESS_H

#include <CLI/CLI.hpp>


namespace Syft {

    class FairnessRunner : public BaseRunner {
        private:
        const std::string assumption_filename_;

        void do_fairness_synthesis_(const SymbolicStateDfa &dfa) const;

        public:
        FairnessRunner(const std::shared_ptr<whitemech::lydia::parsers::ltlf::LTLfDriver>& driver,
                       const std::string &formula_file, const std::string &path_to_syfco,
                       const std::string &assumption_filename, bool print_strategy, bool print_times) : BaseRunner(
            driver, formula_file, path_to_syfco, print_strategy, print_times), assumption_filename_(assumption_filename) {}

        void run() const;

    };

}


#endif //LYDIASYFT_CLI_FAIRNESS_H

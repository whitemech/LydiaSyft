//
// Created by marcofavorito on 22/01/24.
//

#ifndef LYDIASYFT_CLI_SYNTHESIS_H
#define LYDIASYFT_CLI_SYNTHESIS_H

#include <CLI/CLI.hpp>
#include "VarMgr.h"
#include "Stopwatch.h"
#include "base.hpp"
#include "synthesizer/LTLfSynthesizer.h"


namespace Syft {

    class SynthesisRunner : public BaseRunner {
    private:

        void do_synthesis_(const SymbolicStateDfa &symbolic_dfa);

    public:
        SynthesisRunner(const std::shared_ptr<whitemech::lydia::parsers::ltlf::LTLfDriver> &driver,
                        const std::string &formula_file, const std::string &path_to_syfco, bool print_strategy,
                        bool print_times) : BaseRunner(driver, formula_file, path_to_syfco, print_strategy,
                                                       print_times) {}

        void run();

    };
}

#endif //LYDIASYFT_CLI_SYNTHESIS_H

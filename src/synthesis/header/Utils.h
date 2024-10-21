#ifndef LYDIASYFT_UTILS_H
#define LYDIASYFT_UTILS_H

#include "synthesizer/GR1LTLfSynthesizer.h"
#include "synthesizer/LTLfMaxSetSynthesizer.h"
#include "Synthesizer.h"
#include "Stopwatch.h"
#include "game/Transducer.h"
#include "lydia/parser/ltlf/driver.hpp"
#include <CLI/CLI.hpp>
#include <string>
#include <optional>
#include "lydia/types.hpp"
#include "Player.h"
#include "VarMgr.h"
#include "game/InputOutputPartition.h"



namespace Syft {

    static const std::string SYFCO_EXECUTABLE_NAME = "syfco";
    static const std::string DEFAULT_SYFCO_PATH_ = "./syfco";

    struct TLSFArgs {
        const Player starting_player;
        const Player protagonist_player;
        const InputOutputPartition partition;
        const whitemech::lydia::ltlf_ptr formula;
    };

    TLSFArgs parse_tlsf(const std::shared_ptr<whitemech::lydia::parsers::ltlf::LTLfDriver> &driver,
                        const std::string &formula_file, const std::optional<std::string> &path_to_syfco_opt = std::nullopt);

    std::string find_syfco_path(const std::optional<std::string> & syfco_path_opt) ;

    std::string find_binary_path(const std::optional<std::string> & binary_path_opt, const std::string& executable_name, const std::string& default_value) ;

    whitemech::lydia::ltlf_ptr parse_formula(const std::shared_ptr<whitemech::lydia::parsers::ltlf::LTLfDriver> &driver,
                                             const std::string &formula);

    std::shared_ptr<Syft::VarMgr> build_var_mgr(const Syft::InputOutputPartition &partition);

    Syft::SymbolicStateDfa
    do_dfa_construction(const whitemech::lydia::LTLfFormula &formula, const std::shared_ptr<Syft::VarMgr> &var_mgr);

    std::string read_assumption_file_if_file_specified(const std::optional<std::string> &filename);

}

#endif //LYDIASYFT_UTILS_H

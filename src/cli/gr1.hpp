//
// Created by marcofavorito on 22/01/24.
//

#ifndef LYDIASYFT_CLI_GR1_H
#define LYDIASYFT_CLI_GR1_H

#include <CLI/CLI.hpp>
#include "synthesizer/GR1LTLfSynthesizer.h"


namespace Syft {

    std::string read_assumption_file_if_file_specified_(const std::optional<std::string> &filename);

    class GR1Runner : public BaseRunner {
    private:
        const std::string path_to_slugs_;
        const std::string gr1_file_;
        const std::optional<std::string> env_safety_file_;
        const std::optional<std::string> agent_safety_file_;

        const whitemech::lydia::ltlf_ptr agent_safety_;
        const whitemech::lydia::ltlf_ptr env_safety_;

        Syft::SymbolicStateDfa do_dfa_construction_with_message_(const whitemech::lydia::LTLfFormula &formula,
                                                                 const std::shared_ptr<VarMgr> &var_mgr,
                                                                 const std::string &msg) const;

        void do_gr1_synthesis_(const SymbolicStateDfa &dfa, const SymbolicStateDfa &dfa1,
                               const SymbolicStateDfa &dfa2) const;

        void handle_gr1_synthesis_result_(const GR1LTLfSynthesizer &synthesizer,
                                          const SynthesisResult &result) const;


    public:
        GR1Runner(const std::shared_ptr<whitemech::lydia::parsers::ltlf::LTLfDriver> &driver,
                  const std::string &formula_file, const std::string &path_to_syfco, const std::string &path_to_slugs,
                  const std::string &gr1_file, const std::optional<std::string> &env_safety_file,
                  const std::optional<std::string> &agent_safety_file,
                  bool print_strategy, bool print_times) : BaseRunner(driver, formula_file, path_to_syfco,
                                                                      print_strategy, print_times),
                                                           path_to_slugs_(path_to_slugs), gr1_file_(gr1_file),
                                                           env_safety_file_(env_safety_file),
                                                           agent_safety_file_(agent_safety_file),
                                                           agent_safety_(parse_formula(driver_,
                                                                                       read_assumption_file_if_file_specified_(
                                                                                               agent_safety_file_))),
                                                           env_safety_(parse_formula(driver_,
                                                                                     read_assumption_file_if_file_specified_(
                                                                                             env_safety_file_))) {}

        void run() const;
    };

}


#endif //LYDIASYFT_CLI_GR1_H

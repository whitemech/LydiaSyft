#include <filesystem>
#include <memory>
#include <string>

#include <lydia/parser/ltlf/driver.hpp>

#include "automata/SymbolicStateDfa.h"
#include "VarMgr.h"
#include "Utils.h"
#include "Preprocessing.h"
#include "synthesizer/FairnessLtlfSynthesizer.h"


int main(int argc, char ** argv) {

    // parse TLSF file
    std::filesystem::path ROOT_DIRECTORY = __ROOT_DIRECTORY;
    std::filesystem::path tlsf_file_test = ROOT_DIRECTORY / "examples" / "fair_stable_test.tlsf";
    auto driver = std::make_shared<whitemech::lydia::parsers::ltlf::LTLfDriver>();
    Syft::TLSFArgs args = Syft::parse_tlsf(driver, tlsf_file_test.string());

    std::filesystem::path assumption_file = ROOT_DIRECTORY / "examples" / "fair_stable_test_assumption.txt";

    // build variable manager
    auto var_mgr = Syft::build_var_mgr(args.partition);

    std::cout << "Building DFA of the formula..." << std::endl;
    Syft::SymbolicStateDfa dfa = Syft::do_dfa_construction(*args.formula, var_mgr);

    std::cout << "Solving the DFA game with fairness constraints..." << std::endl;
    var_mgr->partition_variables(args.partition.input_variables, args.partition.output_variables);
    Syft::FairnessLtlfSynthesizer synthesizer(dfa, args.starting_player,
                                              args.protagonist_player, dfa.final_states(),
                                              var_mgr->cudd_mgr()->bddOne(), assumption_file.c_str());
    Syft::SynthesisResult result = synthesizer.run();

    if (result.realizability) {
        std::cout << "Specification is realizable!" << std::endl;
    }
    else {
        std::cout << "Specification is unrealizable!" << std::endl;
    }

}
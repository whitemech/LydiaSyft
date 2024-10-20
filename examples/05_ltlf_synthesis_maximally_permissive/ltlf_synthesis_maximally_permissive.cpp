#include <filesystem>
#include <memory>
#include <string>

#include <lydia/parser/ltlf/driver.hpp>

#include "automata/SymbolicStateDfa.h"
#include "Player.h"
#include "VarMgr.h"
#include "Utils.h"
#include "Preprocessing.h"


int main(int argc, char ** argv) {

    // parse TLSF file
    std::filesystem::path ROOT_DIRECTORY = __ROOT_DIRECTORY;
    std::filesystem::path tlsf_file_test = ROOT_DIRECTORY / "examples" / "test1.tlsf";
    auto driver = std::make_shared<whitemech::lydia::parsers::ltlf::LTLfDriver>();
    Syft::TLSFArgs args = Syft::parse_tlsf(driver, tlsf_file_test.string());
    std::cout << "TLSF file parsed: " << tlsf_file_test.string() << std::endl;
    std::cout << "Starting Player: " << (args.protagonist_player == Syft::Player::Agent? "Agent" : "Environment") << std::endl;
    std::cout << "Protagonist Player: " << (args.starting_player == Syft::Player::Agent? "Agent" : "Environment") << std::endl;
    std::cout << "Input variables: ";
    for (const auto& var : args.partition.input_variables){
        std::cout << var << ", ";
    }
    std::cout << std::endl;
    std::cout << "Output variables: ";
    for (const auto& var : args.partition.output_variables){
        std::cout << var << ", ";
    }
    std::cout << std::endl;

    // build variable manager
    auto var_mgr = Syft::build_var_mgr(args.partition);

    std::cout << "Building DFA of the formula..." << std::endl;
    Syft::SymbolicStateDfa dfa = Syft::do_dfa_construction(*args.formula, var_mgr);

    std::cout << "Solving the DFA game (with maximally permissive strategies)..." << std::endl;
    var_mgr->partition_variables(args.partition.input_variables, args.partition.output_variables);

    Syft::LTLfMaxSetSynthesizer synthesizer(dfa, args.starting_player,
                                            args.protagonist_player, dfa.final_states(),
                                            var_mgr->cudd_mgr()->bddOne());
    Syft::MaxSetSynthesisResult result = synthesizer.run();
    if (result.realizability) {
        std::cout << "Specification is realizable!" << std::endl;
        std::cout << "Printing the (maximally permissive) strategy in DOT format..." << std::endl;
        var_mgr->dump_dot(result.deferring_strategy.Add(), "deferring_strategy.dot");
        var_mgr->dump_dot(result.nondeferring_strategy.Add(), "nondeferring_strategy.dot");
    }
    else {
        std::cout << "Specification is unrealizable!" << std::endl;
    }


}
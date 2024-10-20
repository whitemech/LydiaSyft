#include <filesystem>
#include <memory>
#include <string>

#include <lydia/parser/ltlf/driver.hpp>

#include "automata/SymbolicStateDfa.h"
#include "synthesizer/LTLfSynthesizer.h"
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


    // do preprocessing before constructing the entire DFA
    auto one_step_result = Syft::preprocessing(*args.formula, args.partition, *var_mgr, args.starting_player);
    bool preprocessing_success = one_step_result.realizability.has_value();
    if (preprocessing_success) {
        std::cout << "Preprocessing successful" << std::endl;
    }
    else {
        std::cout << "Preprocessing not successful, continuing with full synthesis" << std::endl;
    }

    if (preprocessing_success and one_step_result.realizability.value()) {
        std::cout << "Specification is realizable in one step!";
        CUDD::BDD move = one_step_result.winning_move;
        var_mgr->dump_dot(move.Add(), "test1_winning_move.dot");
    }

    std::cout << "Building DFA of the formula..." << std::endl;
    Syft::SymbolicStateDfa dfa = Syft::do_dfa_construction(*args.formula, var_mgr);

    std::cout << "Solving the DFA game..." << std::endl;
    var_mgr->partition_variables(args.partition.input_variables, args.partition.output_variables);
    Syft::LTLfSynthesizer synthesizer(dfa, args.starting_player, args.protagonist_player, dfa.final_states(), var_mgr->cudd_mgr()->bddOne());
    Syft::SynthesisResult result = synthesizer.run();
    if (result.realizability) {
        std::cout << "Specification is realizable!" << std::endl;
        std::cout << "Printing the strategy in DOT format..." << std::endl;
        result.transducer->dump_dot("test1_winning_strategy.dot");
    }
    else {
        std::cout << "Specification is unrealizable!" << std::endl;
    }


}
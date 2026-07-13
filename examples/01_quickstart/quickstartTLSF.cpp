#include <iostream>
#include <string>
#include "Parser.h"
#include "VarMgr.h"
#include <memory>
#include <sstream>
#include <string>
#include <vector>
#include <filesystem>
#include "lydia/mona_ext/mona_ext_base.hpp"
#include <lydia/parser/ltlf/driver.hpp>
#include "automata/ExplicitStateDfa.h"
#include "automata/ExplicitStateDfaAdd.h"
#include "automata/SymbolicStateDfa.h"
#include "game/InputOutputPartition.h"
#include "Player.h"
#include "synthesizer/LTLfSynthesizer.h"

int main(int argc, char ** argv) {
    auto parser = Syft::Parser::read_from_file("/LydiaSyft/syfco", "/examples/01_quickstart/test_multi.tlsf");
    auto input = parser.get_input_variables();
    auto agents = parser.get_agent_variables();
    auto formulas = parser.get_agent_formulas();
    std::cout << "Input variables found: " << input.size() << std::endl;
    std::cout << "Number of agents found: " << agents.size() << std::endl; 
    std::cout << "Agent formulas found: " << formulas.size() << std::endl;

    std::cout << "Input variables: " << std::endl;  
    for (const auto& var : input) {
        std::cout << var << std::endl;
    }
    for (std::size_t i = 0; i < agents.size(); ++i) {
        std::cout << "Agent " << i << " variables: " << std::endl;
        for (const auto& var : agents[i]) {
            std::cout << var << std::endl;
        }
        std::cout << "Agent " << i << " formula: " << formulas[i] << std::endl;
    }


    std::shared_ptr<Syft::VarMgr> var_mgr = std::make_shared<Syft::VarMgr>();
    var_mgr->create_named_variables(parser.get_input_variables());
    for (const auto& agent_vars : parser.get_agent_variables()) {
        var_mgr->create_named_variables(agent_vars);
    }
    var_mgr->partition_variables(parser.get_input_variables(), parser.get_agent_variables());

    //Directory for showing results
    std::string out_dir = "dfa_outputs";
    std::filesystem::remove_all(out_dir);
    std::filesystem::create_directories(out_dir);
    
    auto driver = std::make_shared<whitemech::lydia::parsers::ltlf::LTLfDriver>();
    
    for (size_t i = 0; i < formulas.size(); ++i){
        std::string name = (i == 0) ? " main_agent" : " peer" + std::to_string(i);
        std::string prefix= out_dir + "/" + name;
        std::cout << "Generating DFA for" << name << " with formula: " << formulas[i] << std::endl;
        std::stringstream formula_stream(formulas[i]);
        driver->parse(formula_stream);
        auto ltlf_ptr = driver->get_result();

        //Explicit DFA
        auto ltlf_formula = std::dynamic_pointer_cast<const whitemech::lydia::LTLfFormula>(ltlf_ptr);
        Syft::ExplicitStateDfa dfa = Syft::ExplicitStateDfa::dfa_of_formula(*ltlf_formula);
        dfa.export_dfa(prefix + ".mona");
        whitemech::lydia::print_mona_dfa(
            dfa.dfa_,
            prefix,
            dfa.get_nb_variables()
        );
        
        //Explicit DFA ADD
        Syft::ExplicitStateDfaAdd explicit_dfa_add = Syft::ExplicitStateDfaAdd::from_dfa_mona(var_mgr, dfa);
        explicit_dfa_add.dump_dot(prefix + "_add.dot");

        //Symbolic DFA
        Syft::SymbolicStateDfa symbolic_dfa = Syft::SymbolicStateDfa::from_explicit(std::move(explicit_dfa_add));
        symbolic_dfa.dump_dot(prefix + "_symbolic.dot");

        
    }
    std::cout << "\nCompleted" << std::endl;

    return 0;

}
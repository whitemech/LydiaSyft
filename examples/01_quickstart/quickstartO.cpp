#include <memory>
#include <sstream>
#include <string>
#include <vector>
#include <filesystem>

#include <lydia/parser/ltlf/driver.hpp>

#include "automata/ExplicitStateDfa.h"
#include "automata/ExplicitStateDfaAdd.h"
#include "automata/SymbolicStateDfa.h"
#include "game/InputOutputPartition.h"
#include "Player.h"
#include "VarMgr.h"
#include "synthesizer/LTLfSynthesizer.h"


int main(int argc, char ** argv) {

    // Define formulas: one for environment, one for main agent, and for peer agents
    std::vector<std::string> formula_strs = {
        "F(a & b)",          // Environment formula
        "G(c)",              // Main agent formula
        "F(d | e)",          // Peer agent 1 formula
        "G(f)",              // Peer agent 2 formula
        "F(g & h)",          // Peer agent 3 formula
        "G(i)"               // Peer agent 4 formula
    };
    
    // Define variables: inputs and agents (vector of vectors)
    std::vector<std::string> input_vars = {"a", "b"};
    std::vector<std::vector<std::string>> agent_vars = {
        {"c"},        // Main agent (index 0)
        {"d", "e"},   // Peer agent 1 (index 1)
        {"f"},        // Peer agent 2 (index 2)
        {"g", "h"},   // Peer agent 3 (index 3)
        {"i"}         // Peer agent 4 (index 4)
    };
    
    // parse the formula
    std::vector<whitemech::lydia::ltlf_ptr> formulas;
    auto driver = std::make_shared<whitemech::lydia::parsers::ltlf::LTLfDriver>();
    for (const auto& formula_str : formula_strs) {
        std::stringstream formula_stream(formula_str);
        driver->parse(formula_stream);
        formulas.push_back(std::dynamic_pointer_cast<const whitemech::lydia::LTLfFormula>(driver->get_result()));
    }

    // Initialize partition and variables
    Syft::InputOutputPartition partition = Syft::InputOutputPartition::construct_from_input(input_vars, agent_vars);
    std::shared_ptr<Syft::VarMgr> var_mgr = std::make_shared<Syft::VarMgr>();
    
    // Create all named variables
    std::vector<std::string> all_named_vars = partition.input_variables;
    for (const auto& ag : partition.agent_variables) {
        all_named_vars.insert(all_named_vars.end(), ag.begin(), ag.end());
    }
    var_mgr->create_named_variables(all_named_vars);
    
    for (std::size_t i = 0; i < partition.agent_variables.size(); ++i) {
        var_mgr->create_agent_variables(i, partition.agent_variables[i]);
    }
    var_mgr->partition_variables(partition.input_variables, partition.agent_variables);
    
    std::string output_dir = "quickstart_outputs";
    std::filesystem::remove_all(output_dir);
    std::filesystem::create_directories(output_dir);
    
    // Build DFAs for each agent
    std::vector<Syft::ExplicitStateDfaAdd> explicit_dfas_add;
    std::vector<Syft::SymbolicStateDfa> symbolic_dfas;
    for (size_t i = 0; i < formulas.size(); ++i) {
        Syft::ExplicitStateDfa explicit_dfa = Syft::ExplicitStateDfa::dfa_of_formula(*formulas[i]);
        Syft::ExplicitStateDfaAdd explicit_dfa_add = Syft::ExplicitStateDfaAdd::from_dfa_mona(var_mgr, explicit_dfa);
        std::cout << "DFA " << i << " (agent " << i << "): " << explicit_dfa_add.state_count() << " states" << std::endl;
        
        std::string mona_file = output_dir + "/agent_" + std::to_string(i) + ".mona";
        //std::cout << "Exporting DFA " << i << " to " << mona_file << std::endl;
        explicit_dfa.export_dfa(mona_file);
        
        explicit_dfas_add.push_back(explicit_dfa_add);
        
        // Build symbolic DFA
        Syft::SymbolicStateDfa symbolic_dfa = Syft::SymbolicStateDfa::from_explicit(std::move(explicit_dfa_add));
        symbolic_dfas.push_back(symbolic_dfa);
    }
    
    // Check if everything worked
    std::cout << "Multi-agent DFA construction successful! All modifications work." << std::endl;

    
    
    // do synthesis
    //var_mgr->partition_variables(partition.input_variables, partition.output_variables);
    //Syft::Player starting_player = Syft::Player::Agent;
    //Syft::Player protagonist_player = Syft::Player::Agent;
    //Syft::LTLfSynthesizer synthesizer(symbolic_dfa, starting_player,
    //                                    protagonist_player, symbolic_dfa.final_states(),
    //                                    var_mgr->cudd_mgr()->bddOne());
    //Syft::SynthesisResult result = synthesizer.run();


    //std::cout << (result.realizability? "" : "NOT ") << "REALIZABLE" << std::endl;
    return 0;
}
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

    // Define formulas
    std::vector<std::string> formula_strs = {
        "F(a & b)", "G(c)", "F(d | e)", "G(f)", "F(g & h)", "G(i)"
    };
    
    std::vector<std::string> input_vars = {"a", "b"};
    std::vector<std::vector<std::string>> agent_vars = {
        {"c"}, {"d", "e"}, {"f"}, {"g", "h"}, {"i"}
    };
    
    // Parse the formulas
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
    

    // Build and Save DFAs
    for (size_t i = 0; i < formulas.size(); ++i) {
        std::string base_name;
        if (i == 0) {
            base_name = "environment";
        } else if (i == 1) {
            base_name = "main_agent";
        } else {
            base_name = "peer_agent_" + std::to_string(i - 1);
        }

        // build the explicit-state DFA
        Syft::ExplicitStateDfa explicit_dfa = Syft::ExplicitStateDfa::dfa_of_formula(*formulas[i]);
        Syft::ExplicitStateDfaAdd explicit_dfa_add = Syft::ExplicitStateDfaAdd::from_dfa_mona(var_mgr, explicit_dfa);
        
        explicit_dfa.export_dfa(output_dir + "/" + base_name + ".mona");
        explicit_dfa_add.dump_dot(output_dir + "/" + base_name + "_explicit.dot");

        // build the symbolic-state DFA from the explicit-state DFA
        Syft::SymbolicStateDfa symbolic_dfa = Syft::SymbolicStateDfa::from_explicit(std::move(explicit_dfa_add));
        
    
        std::string symbolic_dot_path = output_dir + "/" + base_name + "_symbolic.dot";
        symbolic_dfa.dump_dot(symbolic_dot_path);
        
        //std::cout << "Saved symbolic DFA for " << base_name << " to " << symbolic_dot_path << std::endl;
    }
    
    return 0;
}

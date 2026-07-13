#include <memory>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <filesystem>

#include <lydia/parser/ltlf/driver.hpp>

#include "automata/ExplicitStateDfa.h"
#include "automata/ExplicitStateDfaAdd.h"
#include "automata/SymbolicStateDfa.h"
#include "game/InputOutputPartition.h"
#include "VarMgr.h"
#include "lydia/mona_ext/mona_ext_base.hpp"


int main(int argc, char ** argv) {

    // Multi-agent formulas
    std::vector<std::string> formula_strs = {
        "G(req -> F(grant))",     // Environment: If request, eventually grant
        "F(serve)",               // Main agent: Eventually serve
        "G(ready)",               // Peer agent 1: Always ready
        "F(complete)"             // Peer agent 2: Eventually complete
    };
    std::vector<std::string> input_vars = {"req"};
    std::vector<std::vector<std::string>> agent_vars = {
        {"grant"},    // Main agent
        {"serve"},    // Peer 1
        {"ready"},    // Peer 2
        {"complete"}  // Peer 3
    };

    // Parse formulas
    std::vector<whitemech::lydia::ltlf_ptr> formulas;
    auto driver = std::make_shared<whitemech::lydia::parsers::ltlf::LTLfDriver>();
    for (const auto& formula_str : formula_strs) {
        std::cout << "Input formula: " << formula_str << std::endl;
        std::stringstream formula_stream(formula_str);
        driver->parse(formula_stream);
        formulas.push_back(std::dynamic_pointer_cast<const whitemech::lydia::LTLfFormula>(driver->get_result()));
    }

    // Build explicit-state DFAs
    std::vector<Syft::ExplicitStateDfa> dfas;
    for (const auto& formula : formulas) {
        Syft::ExplicitStateDfa dfa = Syft::ExplicitStateDfa::dfa_of_formula(*formula);
        dfas.push_back(dfa);
    }

    // Initialize VarMgr for ADD representation
    Syft::InputOutputPartition partition = Syft::InputOutputPartition::construct_from_input(input_vars, agent_vars);
    std::shared_ptr<Syft::VarMgr> var_mgr = std::make_shared<Syft::VarMgr>();
    std::vector<std::string> all_vars = input_vars;
    for (const auto& ag : agent_vars) {
        all_vars.insert(all_vars.end(), ag.begin(), ag.end());
    }
    var_mgr->create_named_variables(all_vars);
    for (size_t i = 0; i < agent_vars.size(); ++i) {
        var_mgr->create_agent_variables(i, agent_vars[i]);
    }
    var_mgr->partition_variables(input_vars, agent_vars);

    std::vector<std::string> labels;
    labels.push_back("Environment");
    labels.push_back("Main Agent");
    for (size_t i = 2; i < dfas.size(); ++i) {
        labels.push_back("Peer Agent " + std::to_string(i - 1));
    }

    std::string output_dir = "dfa_representation_outputs";
    std::filesystem::remove_all(output_dir);
    std::filesystem::create_directories(output_dir);

    // Show representations for each DFA
    for (size_t i = 0; i < dfas.size(); ++i) {
        std::string file_prefix = labels[i];
        std::replace(file_prefix.begin(), file_prefix.end(), ' ', '_');
        std::cout << "\n--- DFA " << i << " (" << labels[i] << ") ---" << std::endl;
        std::cout << "Printing the DFA in textual form: " << std::endl;
        dfas[i].dfa_print();

        // Export in MONA format
        std::string mona_file = output_dir + "/" + file_prefix + ".mona";
        //std::cout << "Printing the DFA in MONA format to " << mona_file << "..." << std::endl;
        dfas[i].export_dfa(mona_file);

        // Export in DOT and SVG
        std::string dot_base = output_dir + "/" + file_prefix;
        //std::cout << "Exporting the explicit-state MONA DFA in DOT and SVG to '" << dot_base << ".dot' and '" << dot_base << ".svg'..." << std::endl;
        whitemech::lydia::print_mona_dfa(
            dfas[i].dfa_,
            dot_base,
            dfas[i].get_nb_variables()
        );

        // Transform to explicit state form with ADD
        Syft::ExplicitStateDfaAdd explicit_dfa_add = Syft::ExplicitStateDfaAdd::from_dfa_mona(var_mgr, dfas[i]);
        std::cout << "Number of states: " << explicit_dfa_add.state_count() << std::endl;
        std::string add_dot = output_dir + "/" + file_prefix + "_add.dot";
        //std::cout << "Exporting the explicit-state ADD DFA in DOT format to file '" << add_dot << "'..." << std::endl;
        explicit_dfa_add.dump_dot(add_dot);

        // Build symbolic-state DFA
        Syft::SymbolicStateDfa symbolic_dfa = Syft::SymbolicStateDfa::from_explicit(std::move(explicit_dfa_add));
        std::string sym_dot = output_dir + "/" + file_prefix + "_symbolic.dot";
        //std::cout << "Exporting the symbolic-state DFA in DOT format to file '" << sym_dot << "'..." << std::endl;
        symbolic_dfa.dump_dot(sym_dot);
    }

    std::cout << "\nMulti-agent DFA representation example completed successfully!" << std::endl;

    return 0;
}
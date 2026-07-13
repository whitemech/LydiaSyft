#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <sstream>
#include <fstream>
#include <filesystem>
#include "lydia/mona_ext/mona_ext_base.hpp"
#include <lydia/parser/ltlf/driver.hpp>
#include "automata/ExplicitStateDfa.h"
#include "automata/ExplicitStateDfaAdd.h"
#include "automata/SymbolicStateDfa.h"
#include "game/InputOutputPartition.h"

#include "Actor.h"
#include "VarMgr.h"
#include "game/BuchiReachability_multiagent.hpp"
#include "game/DfaGameSynthesizer_multiagent.h"

using namespace Syft;

int main(int argc, char ** argv) {
    // 1. Command line arguments check
    if (argc < 2) {
        std::cerr << "[ERROR] You must specify the test case directory.\n";
        std::cerr << "Usage: " << argv[0] << " <test_directory_path>\n";
        return 1;
    }

    try {
        // Clean path handling using std::filesystem
        std::filesystem::path root_dir = argv[1];
        std::filesystem::path part_file = root_dir / "var.part";

        if (!std::filesystem::exists(part_file)) {
            throw std::runtime_error("Partition file not found: " + part_file.string());
        }

        // 2. Read partition
        InputOutputPartition partition = InputOutputPartition::read_from_file(part_file.string());
        auto input_vars = partition.input_variables;
        auto agent_vars_groups = partition.agent_variables;
        size_t num_agents = agent_vars_groups.size(); 

        // Initial informative prints
        std::cout << "Input variables: " << input_vars.size() << std::endl;  
        for (const auto& var : input_vars) std::cout << "  - " << var << std::endl;
        
        for (std::size_t i = 0; i < num_agents; ++i) {
            std::cout << "Agent " << i << " variables: " << std::endl;
            for (const auto& var : agent_vars_groups[i]) std::cout << "  - " << var << std::endl;
        }

        // 3. Initialize Variable Manager
        auto var_mgr = std::make_shared<VarMgr>();
        var_mgr->create_named_variables(input_vars);
        for (const auto& vars : agent_vars_groups) {
            var_mgr->create_named_variables(vars);
        }
        var_mgr->partition_variables(input_vars, agent_vars_groups);

        // 4. Automatic Actor and Formula files generation
        std::vector<Actor> actors;
        actors.push_back(Actor::Environment()); 
        for (size_t i = 0; i < num_agents; ++i) {
            actors.push_back((i == 0) ? Actor::MainAgent() : Actor::PeerAgent(i));
        }

        std::vector<std::filesystem::path> formula_files = { root_dir / "env.ltlf" };
        for (size_t i = 0; i < num_agents; ++i) {
            formula_files.push_back(root_dir / ("agent" + std::to_string(i) + ".ltlf"));
        }

        // 5. Formula parsing and DFA construction (Symbolic & Explicit via Mona)
        std::vector<SymbolicStateDfa> dfas;
        std::vector<ExplicitStateDfa> explicit_dfas;

        auto driver = std::make_shared<whitemech::lydia::parsers::ltlf::LTLfDriver>();

        for (const auto& filepath : formula_files) {
            if (!std::filesystem::exists(filepath)) {
                throw std::runtime_error("Missing formula file: " + filepath.string());
            }

            std::ifstream file(filepath);
            std::stringstream buffer; 
            buffer << file.rdbuf();
            std::stringstream formula_stream(buffer.str());
            
            driver->parse(formula_stream);
            
            auto ltlf_formula = std::dynamic_pointer_cast<const whitemech::lydia::LTLfFormula>(driver->get_result());
            if (!ltlf_formula) {
                throw std::runtime_error("Error casting formula for: " + filepath.string());
            }

            // Generate explicit MONA DFA
            ExplicitStateDfa dfa_mona = ExplicitStateDfa::dfa_of_formula(*ltlf_formula);
            
            // Convert to Symbolic State DFA
            ExplicitStateDfaAdd explicit_add = ExplicitStateDfaAdd::from_dfa_mona(var_mgr, dfa_mona);
            SymbolicStateDfa symbolic_dfa = SymbolicStateDfa::from_explicit(std::move(explicit_add));
            
            dfas.push_back(std::move(symbolic_dfa));
            explicit_dfas.push_back(std::move(dfa_mona));
        }

        // 6. Build Arena (OR product of DFAs)
        SymbolicStateDfa arena = SymbolicStateDfa::product_OR(dfas);
        ExplicitStateDfa arena_explicit = ExplicitStateDfa::dfa_product_or(explicit_dfas);

        // Initial actor setup
        Actor starting_actor = Actor::MainAgent(); 

        // 7. Protagonist loop: Solving the game for each actor
        std::cout << "\n=== MULTI-AGENT BUCHI REACHABILITY SOLVING ===" << std::endl;
        for (size_t i = 0; i < actors.size(); ++i) {  
            const Actor& protagonist_actor = actors[i];

            // Goal is based on protagonist's formula only
            size_t actor_dfa_index = protagonist_actor.is_environment() ? 0 : 1 + protagonist_actor.id();
            
            CUDD::BDD buchi_condition = dfas[actor_dfa_index].final_states();
            CUDD::BDD space = var_mgr->cudd_mgr()->bddOne();

            std::cout << "--------------------------------------------" << std::endl;
            std::cout << "Starting actor:    " << (starting_actor.is_environment() ? "ENV" : "Agent " + std::to_string(starting_actor.id())) << std::endl;
            std::cout << "Protagonist actor: " << (protagonist_actor.is_environment() ? "ENV" : "Agent " + std::to_string(protagonist_actor.id())) << std::endl;

            // Synthesis execution for Buchi Game
            BuchiReachability_multiagent game(arena, starting_actor, protagonist_actor, buchi_condition, space, num_agents);
            SynthesisResult result = game.run();

            if (result.realizability) {
                std::cout << "Result: REALIZABLE (TRUE)" << std::endl;
                if (result.transducer_multiagent != nullptr) {
                    std::string out_name = "winning_strategy_" + 
                        (protagonist_actor.is_environment() ? "env" : "agent" + std::to_string(protagonist_actor.id())) + ".dot";
                    result.transducer_multiagent->dump_dot(out_name);
                    std::cout << " -> Winning strategy saved to: " << out_name << std::endl;
                }
            } else {
                std::cout << "Result: UNREALIZABLE (FALSE)" << std::endl;
            }
        }

    } catch (const std::exception& e) {
        std::cerr << "\n[FATAL ERROR] " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
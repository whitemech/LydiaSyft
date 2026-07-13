#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <sstream>
#include <fstream>
#include <filesystem>
#include "lydia/mona_ext/mona_ext_base.hpp"
#include <lydia/parser/ltlf/driver.hpp>
#include "Parser.h"
#include <lydia/parser/ltlfplus/driver.hpp>

#include "automata/ExplicitStateDfa.h"
#include "automata/ExplicitStateDfaAdd.h"
#include "automata/SymbolicStateDfa.h"
#include "game/InputOutputPartition.h"
#include "synthesizer/ObligationLTLfPlusSynthesizer.h"
#include "Actor.h"
#include "game/BuchiReachability_multiagent.hpp"
//#include "/home/stella/LydiaSyft/src/synthesis/source/synthesizer/ObligationLTLfPlusSynthesizer.cpp"

#include "Actor.h"
#include "VarMgr.h"
#include "Utils.h"
#include "Synthesizer.h"

using namespace Syft;
using namespace whitemech::lydia;


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

        InputOutputPartition partition = InputOutputPartition::read_from_file(part_file.string());
        
        auto input_vars = partition.input_variables;
        auto agent_vars_groups = partition.agent_variables;
        size_t num_agents = agent_vars_groups.size(); 

        auto var_mgr = std::make_shared<VarMgr>();
        var_mgr->create_named_variables(input_vars);
        for (const auto& vars : agent_vars_groups) {
            var_mgr->create_named_variables(vars);
        }
        var_mgr->partition_variables(input_vars, agent_vars_groups);

        std::cout << "Input variables: " << std::endl;  
        for (const auto& var : input_vars) {
            std::cout << var << std::endl;
        }
        for (std::size_t i = 0; i < agent_vars_groups.size(); ++i) {
            std::cout << "Agent " << i << " variables: " << std::endl;
            for (const auto& var : agent_vars_groups[i]) {
                std::cout << var << std::endl;
            }
        }
 
        std::vector<Actor> actors;
        actors.push_back(Actor::Environment()); 
        for (size_t i = 0; i < num_agents; ++i) {
            actors.push_back((i == 0) ? Actor::MainAgent() : Actor::PeerAgent(i));
        }

        //Formulas
        std::vector<std::filesystem::path> formula_files = { root_dir / "env.ltlf" };
        for (size_t i = 0; i < num_agents; ++i) {
            formula_files.push_back(root_dir / ("agent" + std::to_string(i) + ".ltlf"));
        }

        // LTLf+ driver
        std::shared_ptr<whitemech::lydia::parsers::ltlfplus::LTLfPlusDriver> driver = 
                std::make_shared<whitemech::lydia::parsers::ltlfplus::LTLfPlusDriver>();

        std::vector<SymbolicStateDfa> dfas;
        std::vector<ExplicitStateDfa> explicit_dfas;
        std::size_t i = 0;
        for (const auto& filepath : formula_files) {
            if (!std::filesystem::exists(filepath)) {
                throw std::runtime_error("Missing formula file: " + filepath.string());
            }

            std::ifstream file(filepath);
            std::stringstream buffer; buffer << file.rdbuf();
            std::stringstream formula_stream(buffer.str());
            driver->parse(formula_stream);
            auto result = driver->get_result();

            // cast ast_ptr into ltlf_plus_ptr. Necessary since AbstractDriver is not template anymore
            auto ptr_ltlf_plus_formula =
                std::static_pointer_cast<const whitemech::lydia::LTLfPlusFormula>(result);

            // transform formula in PNF
            auto pnf = whitemech::lydia::get_pnf_result(*ptr_ltlf_plus_formula);
            Syft::LTLfPlus ltlf_plus_formula;
            ltlf_plus_formula.color_formula_ = pnf.color_formula_;
            ltlf_plus_formula.formula_to_color_= pnf.subformula_to_color_;
            ltlf_plus_formula.formula_to_quantification_= pnf.subformula_to_quantifier_;
            const auto& current_actor = actors[i];
            ObligationLTLfPlusSynthesizer synthesizer(
                ltlf_plus_formula,
                partition,
                Actor::MainAgent(),  // starting actor
                current_actor,   // protagonist actor
                var_mgr
            );

            //we have to convert  the ltlf plus formula into a dwa through function convert_to_symbolic
            auto [final_dfa, color_to_final_states] = synthesizer.convert_to_symbolic_dfa();
            dfas.push_back(std::move(final_dfa));
           
            ++i;
        }

        // Build Arena
        SymbolicStateDfa arena = SymbolicStateDfa::product_OR(dfas);
        //save in a dot file the arena
        /* std::filesystem::path arena_out = root_dir / "arena.dot";
        arena.dump_dot(arena_out.string());
        std::cout << "Arena saved to: " << arena_out.string() << std::endl;
        */
        /* for (size_t i = 0; i < actors.size(); ++i) {  


            // Goal is based on protagonist's formula only
            size_t actor_dfa_index;
            if (actors[i].is_environment()) {
                actor_dfa_index = 0;  // ENV formula
            } else {
                actor_dfa_index = 1 + actors[i].id();  // Agent i formula
            }
            
            CUDD::BDD buchi_condition = dfas[actor_dfa_index].final_states();
            //std::cout << "Buchi condition for " << (actors[actor_dfa_index].is_environment() ? "ENV" : "Agent " + std::to_string(actors[actor_dfa_index].id())) << ": " << buchi_condition << std::endl;
            CUDD::BDD space = var_mgr->cudd_mgr()->bddOne();

            
            const Actor& protagonist_actor = actors[i];
            //we can change the starting actor: now is the MainAgent(), but we can set as starting one also Enviroment() or PeerAgent(id)
            Actor starting_actor = Actor::MainAgent(); 

            std::cout << "\n--------------------------------------------" << std::endl;
            std::cout << "Starting actor: " << (starting_actor.is_environment() ? "ENV" : "Agent " + std::to_string(starting_actor.id())) << std::endl;
            std::cout << "Protagonist actor: " << (protagonist_actor.is_environment() ? "ENV" : "Agent " + std::to_string(protagonist_actor.id())) << std::endl;

            BuchiReachability_multiagent game(arena, starting_actor, protagonist_actor, buchi_condition, space, num_agents);
            SynthesisResult result = game.run();

            if (result.realizability) {
                std::cout << "Result: TRUE " << std::endl;
                if(result.transducer_multiagent != nullptr){
                    std::string filename = "winning_strategy_" + (protagonist_actor.is_environment() ? "env" : "agent" + std::to_string(protagonist_actor.id())) + ".dot";
                    result.transducer_multiagent->dump_dot(filename);
                    std::cout << "Winning strategy saved to " << filename << std::endl;
                }
            } else {
                std::cout << "Result: FALSE " << std::endl;
            }
        } */

    } catch (const std::exception& e) {
        std::cerr << "\n[FATAL ERROR] " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
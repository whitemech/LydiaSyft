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

#include "Actor.h"
#include "VarMgr.h"
#include "Utils.h"
#include "Synthesizer.h"


using namespace Syft;
using namespace whitemech::lydia;

int main(int argc, char ** argv) {
    if (argc < 2) {
        std::cerr << "[ERROR] You must specify the test case directory.\n";
        std::cerr << "Usage: " << argv[0] << " <test_directory_path>\n";
        return 1;
    }
    try {
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

        
        //Step 1) For each actor, construct the DWA of its safety/guarantuee specification 
        std::vector<SymbolicStateDfa> dwas;
        std::size_t i = 0;
        for (const auto& filename : formula_files) {
            std::ifstream file(filename);
            if (!file.is_open()) throw std::runtime_error("Impossible to open file " + filename.string());
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

            //we have to convert the ltlf plus formula into a symbolic dwa through function convert_to_symbolic
            auto [final_dwa, color_to_final_states] = synthesizer.convert_to_symbolic_dfa();
            dwas.push_back(std::move(final_dwa));
           
            ++i;
        }
        
        //Step 2) From the environment's automata, construct the dwa that accepts CORE_env(phi_env)
        
        SymbolicStateDfa env_ne = SymbolicStateDfa::get_NE(dwas[0], actors[0]);
        SymbolicStateDfa R_env = SymbolicStateDfa::get_CORE(env_ne, actors[0], Actor::MainAgent());
        SymbolicStateDfa R_env_comp = SymbolicStateDfa::complement(R_env);
        //Step 3) For each peer agent w
        // step 3.1) costruct a new automaton obtained as follows: comp(R_env) U dwa_w
        // step 3.2) construct a new automaton that accepts CORE_w( CORE_env(phi_env) -> phi_w)

        std::vector<SymbolicStateDfa> R_peers;
        std::vector<SymbolicStateDfa> R_peers_comp;
        for (size_t i = 2; i < actors.size() ; ++i){
            std::vector<SymbolicStateDfa> vector;
            vector.push_back(dwas[i]);
            vector.push_back(R_env_comp);
            SymbolicStateDfa dwa_prime = SymbolicStateDfa::product_OR(vector);
            SymbolicStateDfa dwa_prime_ne = SymbolicStateDfa::get_NE(dwa_prime, actors[i]);
            SymbolicStateDfa R_peer = SymbolicStateDfa::get_CORE(dwa_prime_ne, actors[i], Actor::MainAgent());            R_peers.push_back(R_peer);
            R_peers.push_back(R_peer);

            SymbolicStateDfa R_peer_comp = SymbolicStateDfa::complement(R_peer);
            R_peers_comp.push_back(R_peer_comp);
        }

        //Step 4) Costruct a new dwa for the main agent obtained as follows:  R_env_comp U R_peers_comp U dwa_0
        std::vector<SymbolicStateDfa> vector;
        vector.push_back(R_env_comp);
        vector.insert(   
            vector.end(),
            R_peers_comp.begin(),
            R_peers_comp.end()
        );
        vector.push_back(dwas[1]);
        SymbolicStateDfa product_arena = SymbolicStateDfa::product_OR(vector);
        SymbolicStateDfa arena = SymbolicStateDfa::get_NE(product_arena, Actor::MainAgent());
        //var_mgr->print_mgr();    
        
        CUDD::BDD buchi_condition = arena.final_states();
        CUDD::BDD space = var_mgr->cudd_mgr()->bddOne();

        //protagonist actor is Main Agent
        Actor protagonist_actor = Actor::MainAgent();
        //we can change the starting actor: now is the MainAgent(), but we can set as starting one also Enviroment() or PeerAgent(id)
        Actor starting_actor = Actor::MainAgent(); 
             
            
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

    } catch (const std::exception& e) {
        std::cerr << "[FATAL ERROR] " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
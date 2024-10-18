#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include <lydia/parser/ltlf/driver.hpp>

#include "automata/ExplicitStateDfa.h"
#include "automata/ExplicitStateDfaAdd.h"
#include "automata/SymbolicStateDfa.h"
#include "game/InputOutputPartition.h"
#include "Player.h"
#include "VarMgr.h"
#include "synthesizer/LTLfSynthesizer.h"


int main(int argc, char ** argv) {

    // define the formula and the input/output variables
    std::string formula_str = "F(a | b)";
    std::vector<std::string> input_vars{"a"};
    std::vector<std::string> output_vars{"b"};
    
    // parse the formula
    auto driver = std::make_shared<whitemech::lydia::parsers::ltlf::LTLfDriver>();
    std::stringstream formula_stream(formula_str);
    driver->parse(formula_stream);
    whitemech::lydia::ltlf_ptr formula = driver->get_result();

    // initialize the variables
    Syft::InputOutputPartition partition = Syft::InputOutputPartition::construct_from_input(input_vars, output_vars);
    std::shared_ptr<Syft::VarMgr> var_mgr = std::make_shared<Syft::VarMgr>();
    var_mgr->create_named_variables(partition.input_variables);
    var_mgr->create_named_variables(partition.output_variables);

    // build the explicit-state DFA
    Syft::ExplicitStateDfa explicit_dfa = Syft::ExplicitStateDfa::dfa_of_formula(*formula);
    Syft::ExplicitStateDfaAdd explicit_dfa_add = Syft::ExplicitStateDfaAdd::from_dfa_mona(var_mgr, explicit_dfa);

    // build the symbolic-state DFA from the explicit-state DFA
    Syft::SymbolicStateDfa symbolic_dfa = Syft::SymbolicStateDfa::from_explicit(
            std::move(explicit_dfa_add));
    
    
    // do synthesis
    var_mgr->partition_variables(partition.input_variables, partition.output_variables);
    Syft::Player starting_player = Syft::Player::Agent;
    Syft::Player protagonist_player = Syft::Player::Agent;
    Syft::LTLfSynthesizer synthesizer(symbolic_dfa, starting_player,
                                        protagonist_player, symbolic_dfa.final_states(),
                                        var_mgr->cudd_mgr()->bddOne());
    Syft::SynthesisResult result = synthesizer.run();


    std::cout << (result.realizability? "" : "NOT ") << "REALIZABLE" << std::endl;
    return 0;
}
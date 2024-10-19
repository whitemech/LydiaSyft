#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include <lydia/parser/ltlf/driver.hpp>

#include "automata/ExplicitStateDfa.h"
#include "automata/ExplicitStateDfaAdd.h"
#include "automata/SymbolicStateDfa.h"
#include "VarMgr.h"
#include "lydia/mona_ext/mona_ext_base.hpp"


int main(int argc, char ** argv) {

    // explicit DFA
    std::string request = "request";
    std::string response = "response";
    std::string formula_str = "G(request -> F(response))";
    std::cout << "Input formula: " << formula_str << std::endl;

    // parse the formula
    auto driver = std::make_shared<whitemech::lydia::parsers::ltlf::LTLfDriver>();
    std::stringstream formula_stream(formula_str);
    driver->parse(formula_stream);
    whitemech::lydia::ltlf_ptr formula = driver->get_result();

    // build the explicit-state DFA
    Syft::ExplicitStateDfa dfa = Syft::ExplicitStateDfa::dfa_of_formula(*formula);

    std::cout << "Printing the DFA in textual form: " << std::endl;
    dfa.dfa_print();

    // export the DFA in MONA format
    std::cout << "Printing the DFA in MONA format..." << std::endl;
    dfa.export_dfa("main.mona");

    // export the DFA in DOT and SVG formats:
    std::cout << "Exporting the explicit-state MONA DFA in DOT and SVG to files 'main.dot' and 'main.svg'..." << std::endl;
    whitemech::lydia::print_mona_dfa(
        dfa.dfa_,
        "main",
        dfa.get_nb_variables()
    );

    // transform in explicit state form with ADD
    std::shared_ptr<Syft::VarMgr> var_mgr = std::make_shared<Syft::VarMgr>();
    var_mgr->create_named_variables({request, response});
    Syft::ExplicitStateDfaAdd explicit_dfa_add = Syft::ExplicitStateDfaAdd::from_dfa_mona(var_mgr, dfa);
    std::cout << "Number of states: " << explicit_dfa_add.state_count() << std::endl;
    std::cout << "Exporting the explicit-state ADD DFA in DOT format to file 'main_add.dot'..." << std::endl;
    explicit_dfa_add.dump_dot("main_add.dot");

    // build the symbolic-state DFA from the explicit-state DFA
    Syft::SymbolicStateDfa symbolic_dfa = Syft::SymbolicStateDfa::from_explicit(std::move(explicit_dfa_add));
    std::cout << "Exporting the symbolic-state DFA in DOT format to file 'main_symbolic.dot'..." << std::endl;
    symbolic_dfa.dump_dot("main_symbolic.dot");

    return 0;
}
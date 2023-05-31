#include "utils.hpp"
#include "InputOutputPartition.h"
#include "VarMgr.h"
#include "ExplicitStateDfaMona.h"
#include "ExplicitStateDfa.h"
#include "SymbolicStateDfa.h"
#include "Player.h"
#include "ReachabilitySynthesizer.h"

namespace Syft {
namespace Test{

    whitemech::lydia::ltlf_ptr parse_formula(const std::string& formula, whitemech::lydia::parsers::ltlf::LTLfDriver& driver) {
        // Parsing the formula
        std::stringstream formula_stream(formula);
        driver.parse(formula_stream);
        whitemech::lydia::ltlf_ptr parsed_formula = driver.get_result();
        // Apply no-empty semantics
        auto context = driver.context;
        auto not_end = context->makeLtlfNotEnd();
        parsed_formula = context->makeLtlfAnd({parsed_formula, not_end});

        return parsed_formula;
    }

    bool get_realizability_from_input(const std::string& formula, const std::vector<std::string>& input_variables, const std::vector<std::string>& output_variables) {
      auto driver = std::make_shared<whitemech::lydia::parsers::ltlf::LTLfDriver>();
      auto parsed_formula = Syft::Test::parse_formula(formula, *driver);

      Syft::InputOutputPartition partition = Syft::InputOutputPartition::construct_from_input(input_variables, output_variables);
      std::shared_ptr<Syft::VarMgr> var_mgr = std::make_shared<Syft::VarMgr>();
      var_mgr->create_named_variables(partition.input_variables);
      var_mgr->create_named_variables(partition.output_variables);
      Syft::ExplicitStateDfaMona explicit_dfa_mona = Syft::ExplicitStateDfaMona::dfa_of_formula(*parsed_formula);
      Syft::ExplicitStateDfa explicit_dfa =  Syft::ExplicitStateDfa::from_dfa_mona(var_mgr, explicit_dfa_mona);

      Syft::SymbolicStateDfa symbolic_dfa = Syft::SymbolicStateDfa::from_explicit(
          std::move(explicit_dfa));
      var_mgr->partition_variables(partition.input_variables,
                                   partition.output_variables);

      Syft::Player starting_player = Syft::Player::Agent;
      Syft::Player protagonist_player = Syft::Player::Agent;
      Syft::ReachabilitySynthesizer synthesizer(symbolic_dfa, starting_player,
                                                protagonist_player, symbolic_dfa.final_states(),
                                                var_mgr->cudd_mgr()->bddOne());
      Syft::SynthesisResult result = synthesizer.run();

      return result.realizability;
    }
}
}
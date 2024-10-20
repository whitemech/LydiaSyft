#include "Parser.h"
#include "misc.h"
#include "Synthesizer.h"
#include "game/InputOutputPartition.h"
#include "lydia/parser/ltlf/driver.hpp"
#include "Utils.h"



namespace Syft {

    std::string find_syfco_path(const std::optional<std::string> & syfco_path_opt) {
        return find_binary_path(syfco_path_opt, Syft::SYFCO_EXECUTABLE_NAME, Syft::DEFAULT_SYFCO_PATH_);
    }

    std::string find_binary_path(const std::optional<std::string> & binary_path_opt, const std::string& executable_name, const std::string& default_value) {
        if (binary_path_opt.has_value()) {
            if (!std::filesystem::exists(binary_path_opt.value())) {
                throw std::runtime_error("Path does not exist: '" + binary_path_opt.value() + "'");
            }
            return binary_path_opt.value();
        }

        auto result = Syft::find_executable_using_which(executable_name);

        if (result.has_value()) {
            return result.value();
        }

        if (std::filesystem::exists(default_value)) {
            return default_value;
        }

        throw std::runtime_error("Could not find '" + executable_name + "' executable neither in system path, nor in current working directory");

    }


    TLSFArgs parse_tlsf(const std::shared_ptr<whitemech::lydia::parsers::ltlf::LTLfDriver> &driver,
                        const std::string &formula_file, const std::optional<std::string> &path_to_syfco_opt) {

        std::string path_to_syfco = Syft::find_syfco_path(path_to_syfco_opt);
        Syft::Parser parser;
        // the parser assumes "syfco" is in the current working directory
        parser = Syft::Parser::read_from_file(path_to_syfco, formula_file);
        bool sys_first = parser.get_sys_first();

        // get starting player and protagonist player
        Syft::Player starting_player = sys_first ? Syft::Player::Agent : Syft::Player::Environment;
        Syft::Player protagonist_player = Syft::Player::Agent;

        Syft::InputOutputPartition partition =
                Syft::InputOutputPartition::construct_from_input(parser.get_input_variables(),
                                                                 parser.get_output_variables());

        // Parsing the formula
        whitemech::lydia::ltlf_ptr parsed_formula = parse_formula(driver, parser.get_formula());

        return {starting_player, protagonist_player, partition, parsed_formula};
    }



    whitemech::lydia::ltlf_ptr parse_formula(const std::shared_ptr<whitemech::lydia::parsers::ltlf::LTLfDriver> &driver,
                                             const std::string &formula) {
        std::stringstream formula_stream(formula);
        driver->parse(formula_stream);
        whitemech::lydia::ltlf_ptr parsed_formula = driver->get_result();
        // Apply no-empty semantics
        auto context = driver->context;
        auto not_end = context->makeLtlfNotEnd();
        parsed_formula = context->makeLtlfAnd({parsed_formula, not_end});
        return parsed_formula;
    }

    std::shared_ptr<Syft::VarMgr> build_var_mgr(const Syft::InputOutputPartition &partition) {
        std::shared_ptr<Syft::VarMgr> var_mgr = std::make_shared<Syft::VarMgr>();
        var_mgr->create_named_variables(partition.input_variables);
        var_mgr->create_named_variables(partition.output_variables);
        return var_mgr;
    }

    Syft::SymbolicStateDfa
    do_dfa_construction(const whitemech::lydia::LTLfFormula &formula, const std::shared_ptr<Syft::VarMgr> &var_mgr) {
        Syft::ExplicitStateDfa explicit_dfa_mona = Syft::ExplicitStateDfa::dfa_of_formula(formula);
        Syft::ExplicitStateDfaAdd explicit_dfa = Syft::ExplicitStateDfaAdd::from_dfa_mona(var_mgr, explicit_dfa_mona);
        Syft::SymbolicStateDfa symbolic_dfa = Syft::SymbolicStateDfa::from_explicit(explicit_dfa);
        return symbolic_dfa;
    }
}
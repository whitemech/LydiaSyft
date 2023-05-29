#include <catch2/catch_test_macros.hpp>

#include "OneStepRealizability.h"
#include "utils.hpp"


std::optional<CUDD::BDD> get_one_step_realizability(const std::string& formula, const std::vector<std::string>& input_variables, const std::vector<std::string>& output_variables, Syft::VarMgr& var_mgr, whitemech::lydia::parsers::ltlf::LTLfDriver& driver) {
    auto parsed_formula = Syft::Test::parse_formula(formula, driver);
    auto partition = Syft::InputOutputPartition::construct_from_input(input_variables, output_variables);
    var_mgr.create_named_variables(partition.input_variables);
    var_mgr.create_named_variables(partition.output_variables);
    return Syft::one_step_realizable(*parsed_formula, partition, var_mgr);
}


TEST_CASE("One-step realizability check of true", "[one-step-realizability-check]") {
    auto driver = std::make_shared<whitemech::lydia::parsers::ltlf::LTLfDriver>();
    auto var_mgr = std::make_shared<Syft::VarMgr>();
    auto actual = get_one_step_realizability("true", vars{}, vars{}, *var_mgr, *driver);
    auto expected = var_mgr->cudd_mgr()->bddOne();
    REQUIRE(actual == expected);
}

TEST_CASE("One-step realizability check of false", "[one-step-realizability-check]") {
    auto driver = std::make_shared<whitemech::lydia::parsers::ltlf::LTLfDriver>();
    auto var_mgr = std::make_shared<Syft::VarMgr>();
    auto actual = get_one_step_realizability("false", vars{}, vars{}, *var_mgr, *driver);
    auto expected = std::nullopt;
    REQUIRE(actual == expected);
}

TEST_CASE("One-step realizability check of a", "[one-step-realizability-check]") {
    auto driver = std::make_shared<whitemech::lydia::parsers::ltlf::LTLfDriver>();
    auto var_mgr = std::make_shared<Syft::VarMgr>();

    SECTION("a controllable"){
        auto actual = get_one_step_realizability("a", vars{}, vars{"a"}, *var_mgr, *driver);
        auto expected = var_mgr->name_to_variable("a");
        REQUIRE(actual == expected);
    }

    SECTION("a uncontrollable"){
        auto actual = get_one_step_realizability("a", vars{"a"}, vars{}, *var_mgr, *driver);
        auto expected = std::nullopt;
        REQUIRE(actual == expected);
    }

}

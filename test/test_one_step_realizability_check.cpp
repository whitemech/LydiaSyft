#include <catch2/catch_test_macros.hpp>

#include "OneStepRealizability.h"
#include "utils.hpp"

TEST_CASE("One-step realizability check of true", "[one-step-realizability-check]") {
    auto driver = std::make_shared<whitemech::lydia::parsers::ltlf::LTLfDriver>();
    auto formula = Syft::Test::parse_formula("true", *driver);
    auto partition = Syft::InputOutputPartition::construct_from_input({}, {});
    std::shared_ptr<Syft::VarMgr> var_mgr = std::make_shared<Syft::VarMgr>();
    var_mgr->create_named_variables(partition.input_variables);
    var_mgr->create_named_variables(partition.output_variables);

    auto actual = Syft::one_step_realizable(*formula, partition, *var_mgr);
    auto expected = var_mgr->cudd_mgr()->bddOne();

    REQUIRE(actual == expected);
}
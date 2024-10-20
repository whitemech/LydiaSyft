#include <catch2/catch_test_macros.hpp>

#include "OneStepUnrealizability.h"
#include "utils.hpp"
#include "Parser.h"


bool get_one_step_unrealizability(const std::string& formula, const std::vector<std::string>& input_variables, const std::vector<std::string>& output_variables, Syft::VarMgr& var_mgr, whitemech::lydia::parsers::ltlf::LTLfDriver& driver) {
    auto parsed_formula = Syft::Test::parse_formula(formula, driver);
    auto partition = Syft::InputOutputPartition::construct_from_input(input_variables, output_variables);
    var_mgr.create_named_variables(partition.input_variables);
    var_mgr.create_named_variables(partition.output_variables);
    return Syft::one_step_unrealizable(*parsed_formula, partition, var_mgr, Syft::Player::Agent);
}


TEST_CASE("One-step unrealizability check of true", "[one-step-unrealizability-check]") {
    auto driver = std::make_shared<whitemech::lydia::parsers::ltlf::LTLfDriver>();
    auto var_mgr = std::make_shared<Syft::VarMgr>();
    auto actual = get_one_step_unrealizability("true", vars{}, vars{}, *var_mgr, *driver);
    auto expected = false;
    REQUIRE(actual == expected);
}

TEST_CASE("One-step unrealizability check of false", "[one-step-unrealizability-check]") {
    auto driver = std::make_shared<whitemech::lydia::parsers::ltlf::LTLfDriver>();
    auto var_mgr = std::make_shared<Syft::VarMgr>();
    auto actual = get_one_step_unrealizability("false", vars{}, vars{}, *var_mgr, *driver);
    auto expected = true;
    REQUIRE(actual == expected);
}

TEST_CASE("One-step unrealizability check of a", "[one-step-unrealizability-check]") {
    auto driver = std::make_shared<whitemech::lydia::parsers::ltlf::LTLfDriver>();
    auto var_mgr = std::make_shared<Syft::VarMgr>();

    std::string formula = "a";

    SECTION("a controllable"){
        auto actual = get_one_step_unrealizability(formula, vars{}, vars{"a"}, *var_mgr, *driver);
        auto expected = false;
        REQUIRE(actual == expected);
    }

    SECTION("a uncontrollable"){
        auto actual = get_one_step_unrealizability(formula, vars{"a"}, vars{}, *var_mgr, *driver);
        auto expected = true;
        REQUIRE(actual == expected);
    }

}

TEST_CASE("One-step unrealizability check of strongX a", "[one-step-unrealizability-check]") {
    auto driver = std::make_shared<whitemech::lydia::parsers::ltlf::LTLfDriver>();
    auto var_mgr = std::make_shared<Syft::VarMgr>();

    std::string formula = "X[!]a";

    SECTION("a controllable"){
        auto actual = get_one_step_unrealizability(formula, vars{}, vars{"a"}, *var_mgr, *driver);
        auto expected = false;
        REQUIRE(actual == expected);
    }

    SECTION("a uncontrollable"){
        auto actual = get_one_step_unrealizability(formula, vars{"a"}, vars{}, *var_mgr, *driver);
        auto expected = false;
        REQUIRE(actual == expected);
    }

}

TEST_CASE("One-step unrealizability check of X a", "[one-step-unrealizability-check]") {
    auto driver = std::make_shared<whitemech::lydia::parsers::ltlf::LTLfDriver>();
    auto var_mgr = std::make_shared<Syft::VarMgr>();

    std::string formula = "X a";

    SECTION("a controllable"){
        auto actual = get_one_step_unrealizability(formula, vars{}, vars{"a"}, *var_mgr, *driver);
        auto expected = false;
        REQUIRE(actual == expected);
    }

    SECTION("a uncontrollable"){
        auto actual = get_one_step_unrealizability(formula, vars{"a"}, vars{}, *var_mgr, *driver);
        auto expected = false;
        REQUIRE(actual == expected);
    }
}

TEST_CASE("One-step unrealizability check of a U b", "[one-step-unrealizability-check]") {
    auto driver = std::make_shared<whitemech::lydia::parsers::ltlf::LTLfDriver>();
    auto var_mgr = std::make_shared<Syft::VarMgr>();

    std::string formula = "a U b";

    SECTION("a controllable, b controllable (inconclusive)") {
        auto actual = get_one_step_unrealizability(formula, vars{}, vars{"a", "b"}, *var_mgr, *driver);
        auto expected = false;
        REQUIRE(actual == expected);
    }
    SECTION("a controllable, b uncontrollable (inconclusive)") {
        auto actual = get_one_step_unrealizability(formula, vars{"b"}, vars{"a"}, *var_mgr, *driver);
        auto expected = false;
        REQUIRE(actual == expected);
    }
    SECTION("a uncontrollable, b controllable (inconclusive)") {
        auto actual = get_one_step_unrealizability(formula, vars{"a"}, vars{"b"}, *var_mgr, *driver);
        auto expected = false;
        REQUIRE(actual == expected);
    }
    SECTION("a uncontrollable, b uncontrollable (unrealizable)") {
        auto actual = get_one_step_unrealizability(formula, vars{"a", "b"}, vars{}, *var_mgr, *driver);
        auto expected = true;
        REQUIRE(actual == expected);
    }
}

TEST_CASE("One-step unrealizability check of a R b", "[one-step-unrealizability-check]") {
    auto driver = std::make_shared<whitemech::lydia::parsers::ltlf::LTLfDriver>();
    auto var_mgr = std::make_shared<Syft::VarMgr>();

    std::string formula = "a R b";

    SECTION("a controllable, b controllable") {
        auto actual = get_one_step_unrealizability(formula, vars{}, vars{"a", "b"}, *var_mgr, *driver);
        auto expected = false;
        REQUIRE(actual == expected);
    }
    SECTION("a controllable, b uncontrollable") {
        auto actual = get_one_step_unrealizability(formula, vars{"b"}, vars{"a"}, *var_mgr, *driver);
        auto expected = true;
        REQUIRE(actual == expected);
    }
    SECTION("a uncontrollable, b controllable") {
        auto actual = get_one_step_unrealizability(formula, vars{"a"}, vars{"b"}, *var_mgr, *driver);
        auto expected = false;
        REQUIRE(actual == expected);
    }
    SECTION("a uncontrollable, b uncontrollable") {
        auto actual = get_one_step_unrealizability(formula, vars{"a", "b"}, vars{}, *var_mgr, *driver);
        auto expected = true;
        REQUIRE(actual == expected);
    }
}

TEST_CASE("One-step unrealizability check of F a", "[one-step-unrealizability-check]") {
    auto driver = std::make_shared<whitemech::lydia::parsers::ltlf::LTLfDriver>();
    auto var_mgr = std::make_shared<Syft::VarMgr>();

    std::string formula = "F a";

    SECTION("a controllable"){
        auto actual = get_one_step_unrealizability(formula, vars{}, vars{"a"}, *var_mgr, *driver);
        auto expected = false;
        REQUIRE(actual == expected);
    }

    SECTION("a uncontrollable"){
        auto actual = get_one_step_unrealizability(formula, vars{"a"}, vars{}, *var_mgr, *driver);
        auto expected = false;
        REQUIRE(actual == expected);
    }
}

TEST_CASE("One-step unrealizability check of F \\! a", "[one-step-unrealizability-check]") {
    auto driver = std::make_shared<whitemech::lydia::parsers::ltlf::LTLfDriver>();
    auto var_mgr = std::make_shared<Syft::VarMgr>();

    std::string formula = "F ~a";

    SECTION("a controllable (realizable)"){
        auto actual = get_one_step_unrealizability(formula, vars{}, vars{"a"}, *var_mgr, *driver);
        auto expected = false;
        REQUIRE(actual == expected);
    }

    SECTION("a uncontrollable (inconclusive)"){
        auto actual = get_one_step_unrealizability(formula, vars{"a"}, vars{}, *var_mgr, *driver);
        auto expected = false;
        REQUIRE(actual == expected);
    }
}

TEST_CASE("One-step unrealizability check of G a", "[one-step-unrealizability-check]") {
    auto driver = std::make_shared<whitemech::lydia::parsers::ltlf::LTLfDriver>();
    auto var_mgr = std::make_shared<Syft::VarMgr>();

    std::string formula = "G a";

    SECTION("a controllable"){
        auto actual = get_one_step_unrealizability(formula, vars{}, vars{"a"}, *var_mgr, *driver);
        auto expected = false;
        REQUIRE(actual == expected);
    }

    SECTION("a uncontrollable"){
        auto actual = get_one_step_unrealizability(formula, vars{"a"}, vars{}, *var_mgr, *driver);
        auto expected = true;
        REQUIRE(actual == expected);
    }
}

TEST_CASE("One-step unrealizability check of G F a", "[one-step-unrealizability-check]") {
  auto driver = std::make_shared<whitemech::lydia::parsers::ltlf::LTLfDriver>();
  auto var_mgr = std::make_shared<Syft::VarMgr>();

  std::string formula = "G(F(a))";

  SECTION("a controllable"){
    auto actual = get_one_step_unrealizability(formula, vars{}, vars{"a"}, *var_mgr, *driver);
    auto expected = false;
    REQUIRE(actual == expected);
  }

  SECTION("a uncontrollable (inconclusive)"){
    auto actual = get_one_step_unrealizability(formula, vars{"a"}, vars{}, *var_mgr, *driver);
    auto expected = false;
    REQUIRE(actual == expected);
  }
}

TEST_CASE("One-step unrealizability check of random formula 1", "[one-step-unrealizability-check]") {
  auto driver = std::make_shared<whitemech::lydia::parsers::ltlf::LTLfDriver>();
  auto var_mgr = std::make_shared<Syft::VarMgr>();

  std::string formula = "(FG !b || ((G(a || (!a U b))) && !a && (G(!b || F(a)) && GF a))) && F(true)";

  SECTION("a controllable, b uncontrollable"){
    auto actual = get_one_step_unrealizability(formula, vars{"b"}, vars{"a"}, *var_mgr, *driver);
    auto expected = false;
    REQUIRE(actual == expected);
  }
}

TEST_CASE("One-step unrealizability check of a U strongXb", "[one-step-unrealizability-check]") {
  auto driver = std::make_shared<whitemech::lydia::parsers::ltlf::LTLfDriver>();
  auto var_mgr = std::make_shared<Syft::VarMgr>();

  std::string formula = "a U X[!](b)";

  SECTION("a controllable, b controllable") {
    auto actual = get_one_step_unrealizability(formula, vars{}, vars{"a", "b"}, *var_mgr, *driver);
    auto expected = false;
    REQUIRE(actual == expected);
  }
  SECTION("a controllable, b uncontrollable") {
    auto actual = get_one_step_unrealizability(formula, vars{"b"}, vars{"a"}, *var_mgr, *driver);
    auto expected = false;
    REQUIRE(actual == expected);
  }
  SECTION("a uncontrollable, b controllable") {
    auto actual = get_one_step_unrealizability(formula, vars{"a"}, vars{"b"}, *var_mgr, *driver);
    auto expected = false;
    REQUIRE(actual == expected);
  }
  SECTION("a uncontrollable, b uncontrollable") {
    auto actual = get_one_step_unrealizability(formula, vars{"a", "b"}, vars{}, *var_mgr, *driver);
    auto expected = false;
    REQUIRE(actual == expected);
  }
}

TEST_CASE("One-step unrealizability check of examples/001.tlsf", "[one-step-unrealizability-check]") {
    auto driver = std::make_shared<whitemech::lydia::parsers::ltlf::LTLfDriver>();
    auto var_mgr = std::make_shared<Syft::VarMgr>();
    Syft::Parser parser;
    parser = Syft::Parser::read_from_file(Syft::Test::SYFCO_LOCATION, Syft::Test::EXAMPLE_001_TLSF);

    auto formula = parser.get_formula();
    auto input_vars = parser.get_input_variables();
    auto output_vars = parser.get_output_variables();

    auto actual = get_one_step_unrealizability(formula, input_vars, output_vars, *var_mgr, *driver);
    REQUIRE(!actual);
}


TEST_CASE("One-step unrealizability check of Uright pattern", "[one-step-unrealizability-check]") {
    auto driver = std::make_shared<whitemech::lydia::parsers::ltlf::LTLfDriver>();
    auto var_mgr = std::make_shared<Syft::VarMgr>();

    std::string formula("p1 U (p2 U (p3 U (p4 U (p5 U (p6 U (p7 U (p8 U (p9 U (p10 U (p11 U (p12 U (p13 U (p14 U (p15 U (p16 U (p17 U (p18 U (p19 U p20))))))))))))))))))");

    auto set1 = std::vector<std::string>{"p1", "p3", "p5", "p6", "p8", "p2", "p4", "p7", "p9", "p10"};
    auto set2 = std::vector<std::string>{"p12", "p14", "p17", "p19", "p11", "p13", "p15", "p16", "p18", "p20"};
    SECTION("realizable") {
        const auto& input_vars = set1;
        const auto& output_vars = set2;
        bool unrealizability_result = get_one_step_unrealizability(formula, input_vars, output_vars, *var_mgr, *driver);
        REQUIRE(!unrealizability_result);
    }

    SECTION("inconclusive") {
        const auto& input_vars = set2;
        const auto& output_vars = set1;
        auto unrealizability_result = get_one_step_unrealizability(formula, input_vars, output_vars, *var_mgr, *driver);
        REQUIRE(!unrealizability_result);
    }

}

TEST_CASE("One-step unrealizability check of GF-pattern", "[one-step-unrealizability-check]") {
    auto driver = std::make_shared<whitemech::lydia::parsers::ltlf::LTLfDriver>();
    auto var_mgr = std::make_shared<Syft::VarMgr>();

    std::string formula("(G(p1)) & (F(p2)) & (F(p3)) & (F(p4)) & (F(p5)) & (F(p6)) & (F(p7)) & (F(p8)) & (F(p9)) & (F(p10)) & (F(p11)) & (F(p12)) & (F(p13)) & (F(p14)) & (F(p15)) & (F(p16)) & (F(p17)) & (F(p18)) & (F(p19)) & (F(p20))");

    auto all_vars_set = std::vector<std::string>(20);
    std::generate(all_vars_set.begin(), all_vars_set.end(), [n = 0] () mutable { return "p" + std::to_string(++n); });
    auto empty_set = std::vector<std::string>(0);

    auto all_vars_but_first_set = std::vector<std::string>(19);
    auto only_first_var_set = std::vector<std::string>({all_vars_set[0]});
    std::copy(all_vars_set.begin() + 1, all_vars_set.end(), all_vars_but_first_set.begin());
    REQUIRE(all_vars_but_first_set.size() == 19);

    SECTION("realizable with all vars") {
        const auto& input_vars = empty_set;
        const auto& output_vars = all_vars_set;
        bool unrealizability_result = get_one_step_unrealizability(formula, input_vars, output_vars, *var_mgr, *driver);
        REQUIRE(!unrealizability_result);
    }

    SECTION("unrealizable with no vars") {
        const auto& input_vars = all_vars_set;
        const auto& output_vars = empty_set;
        bool unrealizability_result = get_one_step_unrealizability(formula, input_vars, output_vars, *var_mgr, *driver);
        REQUIRE(unrealizability_result);
    }

    SECTION("unrealizable with all vars but first") {
        const auto& input_vars = only_first_var_set;
        const auto& output_vars = all_vars_but_first_set;
        bool unrealizability_result = get_one_step_unrealizability(formula, input_vars, output_vars, *var_mgr, *driver);
        REQUIRE(unrealizability_result);
    }

    SECTION("inconclusive with only first var") {
        const auto& input_vars = all_vars_but_first_set;
        const auto& output_vars = only_first_var_set;
        bool unrealizability_result = get_one_step_unrealizability(formula, input_vars, output_vars, *var_mgr, *driver);
        REQUIRE(!unrealizability_result);
    }

}

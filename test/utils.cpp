#include "utils.hpp"

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
}
}
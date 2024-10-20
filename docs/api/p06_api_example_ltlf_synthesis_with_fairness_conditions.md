# LTLf Synthesis with Fairness Constraints {#p06_api_example_ltlf_synthesis_with_fairness_conditions}

In this example, we will show how to use the LydiaSyft C++ APIs to solve LTLf synthesis with different kinds of fairness constraints.

## Fairness Constraints

The code for this example can be found in `examples/06_ltlf_synthesis_with_fairness_conditions/ltlf_synthesis_with_fairness_constraints.cpp`.
To build this example, you can run `make ltlf_synthesis_with_fairness_constraints_example`.

Follows the full code:
```cpp
#include <filesystem>
#include <memory>
#include <string>

#include <lydia/parser/ltlf/driver.hpp>

#include "automata/SymbolicStateDfa.h"
#include "VarMgr.h"
#include "Utils.h"
#include "Preprocessing.h"
#include "synthesizer/FairnessLtlfSynthesizer.h"


int main(int argc, char ** argv) {

    // parse TLSF file
    std::filesystem::path ROOT_DIRECTORY = __ROOT_DIRECTORY;
    std::filesystem::path tlsf_file_test = ROOT_DIRECTORY / "examples" / "fair_stable_test.tlsf";
    auto driver = std::make_shared<whitemech::lydia::parsers::ltlf::LTLfDriver>();
    Syft::TLSFArgs args = Syft::parse_tlsf(driver, tlsf_file_test.string());

    std::filesystem::path assumption_file = ROOT_DIRECTORY / "examples" / "fair_stable_test_assumption.txt";

    // build variable manager
    auto var_mgr = Syft::build_var_mgr(args.partition);

    std::cout << "Building DFA of the formula..." << std::endl;
    Syft::SymbolicStateDfa dfa = Syft::do_dfa_construction(*args.formula, var_mgr);

    std::cout << "Solving the DFA game with fairness constraints..." << std::endl;
    var_mgr->partition_variables(args.partition.input_variables, args.partition.output_variables);
    Syft::FairnessLtlfSynthesizer synthesizer(dfa, args.starting_player,
                                              args.protagonist_player, dfa.final_states(),
                                              var_mgr->cudd_mgr()->bddOne(), assumption_file.c_str());
    Syft::SynthesisResult result = synthesizer.run();

    if (result.realizability) {
        std::cout << "Specification is realizable!" << std::endl;
    }
    else {
        std::cout << "Specification is unrealizable!" << std::endl;
    }

}
```

Differently from the classical setting, now we can provide a set of _fairness conditions_.
In the code above, `assumption_file` contains the path to the fairness assumption file
`examples/fair_stable_test_assumption.txt`:

```
add
```

More in general, the assumption file is expected to be in CNF form, where each row, a space-separated list of propositions,
is a clause of the assumption constraint.
The declaration of the class `Syft::FairnessLtlfSynthesizer` can be found in the module `synthesize/FairnessLtlfSynthesizer.h`.



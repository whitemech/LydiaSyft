# LTLf Synthesis with Environment Constraints {#p06_07_08_api_example_ltlf_synthesis_with_environment_conditions}

In this example, we will show how to use the LydiaSyft C++ APIs to solve LTLf synthesis with different kinds of environment constraints.

## Fairness Assumptions

The code for this example can be found in `examples/06_ltlf_synthesis_with_fairness_conditions/ltlf_synthesis_with_fairness_constraints.cpp`.
To build this example, you can run `make ltlf_synthesis_with_fairness_conditions_example`.

Follows the full code of the example:
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


## Stability Assumptions

The code for this example can be found in `examples/07_ltlf_synthesis_with_stability_conditions/ltlf_synthesis_with_stability_conditions.cpp`.
To build this example, you can run `make ltlf_synthesis_with_stability_conditions_example`.

Follows the full code of the example:

```cpp
#include <filesystem>
#include <memory>
#include <string>

#include <lydia/parser/ltlf/driver.hpp>

#include "automata/SymbolicStateDfa.h"
#include "synthesizer/StabilityLtlfSynthesizer.h"
#include "VarMgr.h"
#include "Utils.h"
#include "Preprocessing.h"


int main(int argc, char ** argv) {

    // parse TLSF file
    std::filesystem::path ROOT_DIRECTORY = __ROOT_DIRECTORY;
    std::filesystem::path tlsf_file_test = ROOT_DIRECTORY / "examples" / "fair_stable_counter_test.tlsf";
    auto driver = std::make_shared<whitemech::lydia::parsers::ltlf::LTLfDriver>();
    Syft::TLSFArgs args = Syft::parse_tlsf(driver, tlsf_file_test.string());

    std::filesystem::path assumption_file = ROOT_DIRECTORY / "examples" / "fair_stable_test_assumption.txt";

    // build variable manager
    auto var_mgr = Syft::build_var_mgr(args.partition);

    std::cout << "Building DFA of the formula..." << std::endl;
    Syft::SymbolicStateDfa dfa = Syft::do_dfa_construction(*args.formula, var_mgr);

    std::cout << "Solving the DFA game with stability constraints..." << std::endl;
    var_mgr->partition_variables(args.partition.input_variables, args.partition.output_variables);
    Syft::StabilityLtlfSynthesizer synthesizer(dfa, args.starting_player,
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

The C++ API interface of `Syft::StabilityLtlfSynthesizer` is similar to `Syft::FairnessLtlfSynthesizer`,
and it is declared in the header `synthesizer/StabilityLtlfSynthesizer.h`.

## GR(1) Environment Specifications

The code for this example can be found in `examples/08_ltlf_synthesis_with_gr1_env_spec/ltlf_synthesis_with_gr1_env_spec.cpp`.
To build this example, you can run `make ltlf_synthesis_with_gr1_env_spec_example`.
This example also requires the installation of the `slugs` submodule (see the README for more information.)

Follows the full code of the example:

```cpp
#include <filesystem>
#include <memory>

#include <lydia/parser/ltlf/driver.hpp>

#include "automata/SymbolicStateDfa.h"
#include "Utils.h"
#include "Preprocessing.h"


int main(int argc, char ** argv) {

    // parse TLSF file
    std::filesystem::path ROOT_DIRECTORY = __ROOT_DIRECTORY;
    std::filesystem::path finding_nemo_dir = ROOT_DIRECTORY / "examples" / "TLSF" / "GR1benchmarks" / "finding_nemo";
    std::filesystem::path finding_nemo_1 = finding_nemo_dir / "finding_nemo_1.tlsf";
    std::filesystem::path finding_nemo_1_env_gr1 = finding_nemo_dir / "finding_nemo_1_env_gr1.txt";
    std::filesystem::path finding_nemo_1_env_safety = finding_nemo_dir / "finding_nemo_1_env_safety.ltlf";
    std::filesystem::path finding_nemo_1_agent_safety = finding_nemo_dir / "finding_nemo_1_agn_safety.ltlf";
    std::filesystem::path path_to_slugs_dir = ROOT_DIRECTORY / "submodules" / "slugs";

    auto driver = std::make_shared<whitemech::lydia::parsers::ltlf::LTLfDriver>();
    Syft::TLSFArgs args = Syft::parse_tlsf(driver, finding_nemo_1.string());
    auto var_mgr = Syft::build_var_mgr(args.partition);
    whitemech::lydia::ltlf_ptr agent_safety_formula = Syft::parse_formula(
        driver,
        Syft::read_assumption_file_if_file_specified(finding_nemo_1_agent_safety)
    );
    whitemech::lydia::ltlf_ptr env_safety_formula = Syft::parse_formula(
        driver,
        Syft::read_assumption_file_if_file_specified(finding_nemo_1_env_safety)
    );

    std::cout << "Building the DFA of the formula..." << std::endl;
    Syft::SymbolicStateDfa dfa = Syft::do_dfa_construction(*args.formula, var_mgr);
    std::cout << "Building the DFA of the agent safety formula..." << std::endl;
    Syft::SymbolicStateDfa dfa_agn = Syft::do_dfa_construction(*agent_safety_formula, var_mgr);
    std::cout << "Building the DFA of the environment safety formula..." << std::endl;
    Syft::SymbolicStateDfa dfa_env = Syft::do_dfa_construction(*env_safety_formula, var_mgr);

    std::cout << "Solving the DFA game with GR(1) env assumptions..." << std::endl;
    Syft::GR1 gr1 = Syft::GR1::read_from_gr1_file(var_mgr, finding_nemo_1_env_gr1.string());
    var_mgr->partition_variables(args.partition.input_variables, args.partition.output_variables);
    Syft::GR1LTLfSynthesizer synthesizer(var_mgr, gr1, dfa_env,
                                         dfa, dfa_agn, path_to_slugs_dir.string(), "problem");
    Syft::SynthesisResult result = synthesizer.run();

    if (result.realizability) {
        std::cout << "Specification is realizable!" << std::endl;
    }
    else {
        std::cout << "Specification is unrealizable!" << std::endl;
    }

}
```

In order to use the class `Syft::GR1LTLfSynthesizer` that implements LTLf synthesis with GR(1) environment specifications,
besides the TLSF specification file, the following inputs are accepted:

- the GR(1) condition file
- the agent safety condition (optional)
- the environment safety condition (optional)

The GR(1) file format is similar to the regular CNF format, such that every justice is represented as a conjunction over disjunctive clauses. 
Each justice is labeled with `env` or `agn` to refer to environment or  agent justice respectively, and ended with `end`.

Consider GR(1) formula:

```
GF(p_1) & GF(p_1 & q_1) & ... & GF(!p_1 & (q_1 | q_2)) -> GF(q_1) & GF(p_1 & q_1) & ... & GF(!q_1 & (p_1 | p_2))
```

This formula is written as follows (variables in uppercase):

```
--Env-Justice--
P_1
--Justice-End--

--Env-Justice--
P_1
Q_1
--Justice-End--

--Env-Justice--
!P_1
Q_1 Q_2
--Justice-End--

--Agn-Justice--
Q_1
--Justice-End--

--Agn-Justice--
P_1
Q_1
--Justice-End--

--Agn-Justice--
!Q_1
P_1 Q_1
--Justice-End--
End
```

The agent and environment safety condition are simply LTLf formulas.
Both can be transformed into DFAs and provided as input to the `Syft::GR1LTLfSynthesizer` class.
Its declaration can be found in the module `GR1LTLfSynthesizer.h`.
# LTLf Synthesis {#p04_api_example_ltlf_synthesis}

In this example, we will show how to solve a LTLf synthesis problem in the classical setting using the LydiaSyft C++ APIs.

The code for this example can be found in `examples/04_ltlf_synthesis/ltlf_synthesis.cpp`.
To build this example, you can run `make ltlf_synthesis_example`.

```cpp
#include <memory>
#include <sstream>
#include <string>

#include <lydia/parser/ltlf/driver.hpp>
#include <filesystem>

#include "automata/SymbolicStateDfa.h"
#include "synthesizer/LTLfSynthesizer.h"
#include "Player.h"
#include "VarMgr.h"
#include "Utils.h"
#include "Preprocessing.h"


int main(int argc, char ** argv) {

    // parse TLSF file
    std::filesystem::path ROOT_DIRECTORY = __ROOT_DIRECTORY;
    std::filesystem::path tlsf_file_test = ROOT_DIRECTORY / "examples" / "test.tlsf";
    auto driver = std::make_shared<whitemech::lydia::parsers::ltlf::LTLfDriver>();
    Syft::TLSFArgs args = Syft::parse_tlsf(driver, tlsf_file_test.string());
    std::cout << "TLSF file parsed: " << tlsf_file_test.string() << std::endl;
    std::cout << "Starting Player: " << (args.protagonist_player == Syft::Player::Agent? "Agent" : "Environment") << std::endl;
    std::cout << "Protagonist Player: " << (args.starting_player == Syft::Player::Agent? "Agent" : "Environment") << std::endl;
    std::cout << "Input variables: ";
    for (const auto& var : args.partition.input_variables){
        std::cout << var << ", ";
    }
    std::cout << std::endl;
    std::cout << "Output variables: ";
    for (const auto& var : args.partition.output_variables){
        std::cout << var << ", ";
    }
    std::cout << std::endl;

    // build variable manager
    auto var_mgr = Syft::build_var_mgr(args.partition);


    // do preprocessing before constructing the entire DFA
    auto one_step_result = Syft::preprocessing(*args.formula, args.partition, *var_mgr, args.starting_player);
    bool preprocessing_success = one_step_result.realizability.has_value();
    if (preprocessing_success) {
        std::cout << "Preprocessing successful" << std::endl;
    }
    else {
        std::cout << "Preprocessing not successful, continuing with full synthesis" << std::endl;
    }

    if (preprocessing_success and one_step_result.realizability.value()) {
        std::cout << "Specification is realizable in one step!";
        CUDD::BDD move = one_step_result.winning_move;
        var_mgr->dump_dot(move.Add(), "test_winning_move.dot");
    }

    std::cout << "Building DFA of the formula..." << std::endl;
    Syft::SymbolicStateDfa dfa = Syft::do_dfa_construction(*args.formula, var_mgr);

    std::cout << "Solving the DFA game..." << std::endl;
    var_mgr->partition_variables(args.partition.input_variables, args.partition.output_variables);
    Syft::LTLfSynthesizer synthesizer(dfa, args.starting_player, args.protagonist_player, dfa.final_states(), var_mgr->cudd_mgr()->bddOne());
    Syft::SynthesisResult result = synthesizer.run();
    if (result.realizability) {
        std::cout << "Specification is realizable!" << std::endl;
        std::cout << "Printing the strategy in DOT format..." << std::endl;
        result.transducer->dump_dot("test_winning_strategy.dot");
    }
    else {
        std::cout << "Specification is unrealizable!" << std::endl;
    }


}
```

## Parsing of TLSF files 

In this example we will rely on the `Utils.h` module, which provide handy functions 
that wrap other lower-level function calls to the LydiaSyft library.

We start by using the `Syft::parse_tlsf` function, which uses the tool `syfco` to parse a TLSF input file.

```cpp
std::filesystem::path tlsf_file_test = ROOT_DIRECTORY / "examples" / "test.tlsf";
auto driver = std::make_shared<whitemech::lydia::parsers::ltlf::LTLfDriver>();
Syft::TLSFArgs args = Syft::parse_tlsf(driver, tlsf_file_test.string());
```

`Syft::TLSFArgs` contains the information extraced from the TLSF file in form of C++ objects.
The signature of the `Syft::TLSFArgs` struct is the following:

```
struct TLSFArgs {
    const Player starting_player;
    const Player protagonist_player;
    const InputOutputPartition partition;
    const whitemech::lydia::ltlf_ptr formula;
};
```


## Set up the variable manager

The next step is to initialize the variable manager, needed to build the DFA and to handle the synthesis procedure.

```cpp
// build variable manager
auto var_mgr = Syft::build_var_mgr(args.partition);
```

## Synthesis preprocessing

The function `Syft::preprocessing` implement the checks to determine whether the DFA game associated with the formula can be won in a single move---without building the DFA!

```cpp
// do preprocessing before constructing the entire DFA
auto one_step_result = Syft::preprocessing(*args.formula, args.partition, *var_mgr, args.starting_player);
bool preprocessing_success = one_step_result.realizability.has_value();
if (preprocessing_success) {
    std::cout << "Preprocessing successful" << std::endl;
}
else {
    std::cout << "Preprocessing not successful, continuing with full synthesis" << std::endl;
}

if (preprocessing_success and one_step_result.realizability.value()) {
    std::cout << "Specification is realizable in one step!";
    CUDD::BDD move = one_step_result.winning_move;
    var_mgr->dump_dot(move.Add(), "test_winning_move.dot");
}
```

## DFA Construction

If the preprocessing is successful and returns a positive result, then we already know the specification is realizable.
Otherwise, we must proceed and build the full DFA.
We do so via the function `Syft::do_dfa_construction`:

```cpp
std::cout << "Building DFA of the formula..." << std::endl;
Syft::SymbolicStateDfa dfa = Syft::do_dfa_construction(*args.formula, var_mgr);
```

## Solving the DFA Game

Finally, we are ready to solve the DFA game.
The class `Syft::LTLfSynthesizer` implements the LTLf synthesis procedure in the classical setting.
The return value is an instance of `Syft::SynthesisResult` (see below).

```cpp
std::cout << "Solving the DFA game..." << std::endl;
var_mgr->partition_variables(args.partition.input_variables, args.partition.output_variables);
Syft::LTLfSynthesizer synthesizer(dfa, args.starting_player, args.protagonist_player, dfa.final_states(), var_mgr->cudd_mgr()->bddOne());
Syft::SynthesisResult result = synthesizer.run();
```

The `Syft::SynthesisResult` contains all the information regarding the result of the LTLf synthesis algorithm.
If the boolean attribute `result.realizability` is `true`, then the specification is realizable,
and the `result.transducer` attribute (a pointer to an instance of `Syft::Transducer`, see `Transducer.h`) has been set.
We can print the transducer in visual form using the function `Syft::Transducer::dump_dot`: 

```cpp
if (result.realizability) {
    std::cout << "Specification is realizable!" << std::endl;
    std::cout << "Printing the strategy in DOT format..." << std::endl;
    result.transducer->dump_dot("test_winning_strategy.dot");
}
else {
    std::cout << "Specification is unrealizable!" << std::endl;
}
```

The specification `examples/test.tlsf` is unrealizable, so no output can be produced.
However, you can run the code by changing the variable `tlsf_file_test` to `ROOT_DIRECTORY / "examples" / "test1.tlsf"` and recompile.



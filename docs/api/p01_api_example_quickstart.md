# Quickstart {#p01_api_example_quickstart}

LydiaSyft can be used as a C++ library that integrates with other project.
In this page we will walk through each example and explain the main features supported by the library. 
The examples can be found in the `examples/` folder.

In this example, we will see how to use the LydiaSyft C++ APIs to solve an instance of the LTLf synthesis problem in the classical setting.  
The code for this example can be found in `examples/01_quickstart/quickstart.cpp`.

The content of the file `quickstart.cpp` is:

```cpp
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include <lydia/parser/ltlf/driver.hpp>

#include "automata/ExplicitStateDfa.h"
#include "automata/ExplicitStateDfaAdd.h"
#include "automata/SymbolicStateDfa.h"
#include "game/InputOutputPartition.h"
#include "Player.h"
#include "VarMgr.h"
#include "synthesizer/LTLfSynthesizer.h"


int main(int argc, char ** argv) {

    // define the formula and the input/output variables
    std::string formula_str = "F(a | b)";
    std::vector<std::string> input_vars{"a"};
    std::vector<std::string> output_vars{"b"};
    
    // parse the formula
    auto driver = std::make_shared<whitemech::lydia::parsers::ltlf::LTLfDriver>();
    std::stringstream formula_stream(formula_str);
    driver->parse(formula_stream);
    whitemech::lydia::ltlf_ptr formula = driver->get_result();

    // initialize the variables
    Syft::InputOutputPartition partition = Syft::InputOutputPartition::construct_from_input(input_vars, output_vars);
    std::shared_ptr<Syft::VarMgr> var_mgr = std::make_shared<Syft::VarMgr>();
    var_mgr->create_named_variables(partition.input_variables);
    var_mgr->create_named_variables(partition.output_variables);

    // build the explicit-state DFA
    Syft::ExplicitStateDfa explicit_dfa = Syft::ExplicitStateDfa::dfa_of_formula(*formula);
    Syft::ExplicitStateDfaAdd explicit_dfa_add = Syft::ExplicitStateDfaAdd::from_dfa_mona(var_mgr, explicit_dfa);

    // build the symbolic-state DFA from the explicit-state DFA
    Syft::SymbolicStateDfa symbolic_dfa = Syft::SymbolicStateDfa::from_explicit(
            std::move(explicit_dfa_add));
    
    
    // do synthesis
    var_mgr->partition_variables(partition.input_variables, partition.output_variables);
    Syft::Player starting_player = Syft::Player::Agent;
    Syft::Player protagonist_player = Syft::Player::Agent;
    Syft::LTLfSynthesizer synthesizer(symbolic_dfa, starting_player,
                                        protagonist_player, symbolic_dfa.final_states(),
                                        var_mgr->cudd_mgr()->bddOne());
    Syft::SynthesisResult result = synthesizer.run();


    // expected: REALIZABLE
    std::cout << (result.realizability? "" : "NOT ") << "REALIZABLE" << std::endl;
    return 0;
}
```

The expected output of the program is:

```
REALIZABLE
```

Below we break down each step of the program. 
The first step is to define the _LTLf formula_, the set of _controllable_ (or _output_) _variables_, 
and the set of _uncontrollable_ (or _input_) _variables_:

```cpp
std::string formula_str = "F(a | b)";
std::vector<std::string> input_vars{"a"};
std::vector<std::string> output_vars{"b"};
```

Next, we need to parse the formula string and build the syntax tree of the formula. 
To do so, we rely on Lydia library that provides a parser for LTLf formulas.
In particular, we instantiate the parser `whitemech::lydia::parsers::ltlf::LTLfDriver`, 
which can be found in the header `<lydia/parser/ltlf/driver.hpp>`. 

```cpp
// parse the formula
auto driver = std::make_shared<whitemech::lydia::parsers::ltlf::LTLfDriver>();
std::stringstream formula_stream(formula_str);
driver->parse(formula_stream);
whitemech::lydia::ltlf_ptr formula = driver->get_result();
```

We also instantiate a _variable manager_, i.e. an instance of the class `Syft::VarMgr`. 
The definition of such class can be found in the header file `VarMgr.h`.
Essentially, `Syft::VarMgr` is a wrapper of the CUDD BDD manager, and provides auxiliary fields and methods specific for LTLf synthesis.
The class `Syft::InputOutputPartition` is simply a container class for the input and output variables, 
and can be found in the header file `game/InputOutputPartition.h`.

```cpp
// initialize the variables
Syft::InputOutputPartition partition = Syft::InputOutputPartition::construct_from_input(input_vars, output_vars);
std::shared_ptr<Syft::VarMgr> var_mgr = std::make_shared<Syft::VarMgr>();
var_mgr->create_named_variables(partition.input_variables);
var_mgr->create_named_variables(partition.output_variables);
```

Before calling the LTLf synthesis procedure, we have to build the DFA of the input LTLf formula.
To do so, we rely on the static function `Syft::ExplicitStateDfa::dfa_of_formula()`,
which can be found in the header file `automata/ExplicitStateDfa.h`:

```cpp
// build the explicit-state DFA
Syft::ExplicitStateDfa explicit_dfa = Syft::ExplicitStateDfa::dfa_of_formula(*formula);
```

The output of the Lydia tool is a MONA DFA (see the [MONA manual](https://www.brics.dk/mona/mona14.pdf) for more information).
To proceed, we require to translate the MONA DFA into a DFA based on Algebraic Decision Diagrams (ADD), 
i.e. a DFA with explicit states (as the MONA DFA), but with the symbolic transitions represented in ADDs (instead of Shared Multi-terminal BDDs).
The class that implements this DFA representation is `ExplicitStateDfaAdd`,
and the function `Syft::ExplicitStateDfaAdd::from_dfa_mona` performs such construction. 
The declaration of the class `Syft::ExplicitStateDfaAdd` can be found in the header file `automata/ExplicitStateDfaAdd.h`:

```cpp
Syft::ExplicitStateDfaAdd explicit_dfa_add = Syft::ExplicitStateDfaAdd::from_dfa_mona(var_mgr, explicit_dfa);
```

LydiaSyft requires the DFA to be _(fully) symbolic_, i.e. with symbolic states and transitions.
The class that implements the symbolic DFA representation is `SymbolicStateDfa`.
To build a `SymbolicStateDfa` from an `ExplicitStateDfaAdd`, we use the function 
`Syft::SymbolicStateDfa::from_explicit`.
The declaration of the class `Syft::SymbolicStateDfa` can be found in the header file `automata/SymbolicStateDfa.h`:

```cpp
Syft::SymbolicStateDfa symbolic_dfa = Syft::SymbolicStateDfa::from_explicit(std::move(explicit_dfa_add));
```

Finally, we are ready to perform the synthesis step.
First, we setup the variable manager to partition the variables in the input and output sets: 

```cpp
var_mgr->partition_variables(partition.input_variables, partition.output_variables);
```

Then, we create an instance of `Syft::LTLfSynthesizer` (found in `synthesizer/LTLfSynthesizer.h`), 
a functor class that wraps the initialization and the execution  of the synthesis procedure.

```cpp
Syft::Player starting_player = Syft::Player::Agent;
Syft::Player protagonist_player = Syft::Player::Agent;
Syft::LTLfSynthesizer synthesizer(symbolic_dfa, starting_player,
                                    protagonist_player, symbolic_dfa.final_states(),
                                    var_mgr->cudd_mgr()->bddOne());
```

The constructor of `Syft::LTLfSynthesizer` takes the following parameters:

* `spec`: A symbolic-state DFA representing the LTLf formula.
* `starting_player`: The player that moves first each turn.
* `protagonist_player`: The player for which we aim to find the winning strategy.
* `goal_states`: The set of states that the agent must reach to win.
* `state_space`: The state space.

In our case, we have that the player `Agent` is both the starting player and the protagonist agent.
The goal states are the final states of the DFA, and the states space is the entire DFA (hence we use 
the CUDD expression for "true": `var_mgr->cudd_mgr()->bddOne()`).

To run the synthesis procedure, we use:

```
Syft::SynthesisResult result = synthesizer.run();
```

The `Syft::SynthesisResult` object contains all the details of the solution found, if any:

```
struct SynthesisResult {
    bool realizability;
    CUDD::BDD winning_states;
    CUDD::BDD winning_moves;
    std::unique_ptr<Transducer> transducer;
    CUDD::BDD safe_states;
};
```

The field `realizability` is set to `true` if a solution is found, `false` otherwise.
Then, if `realizability` is `true`:

- `winning_states` is a BDD representing the set of states that are winning;
- `winning_moves` is a BDD representing the set of winning moves;
- `transducer` is an instance of the class `Transducer`, which represents the winning strategy;
- `safe_states` is set only in case of `GR(1)` synthesis (we can ignore it now).

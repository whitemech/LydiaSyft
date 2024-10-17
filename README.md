# LydiaSyft: A Compositional Symbolic Synthesis Framework for LTLf Specification

LydiaSyft is an open-source software framework for reasoning and synthesis of Linear Tempora Logic formulas interpreted on finite traces (LTLf), integrating efficient data structures and techniques focused on LTLf specifications. 

This project can be used either as a standalone CLI tool or as a C++ library that integrates with other projects. Among the implemented Data Structures and algorithms, we have:

- DFA representation and manipulation:
  - Explicit-state DFA (à la MONA): [(N. Klarlund et al., 2002)](https://www.worldscientific.com/doi/abs/10.1142/S012905410200128X), [(De Giacomo and Favorito, 2021)](https://ojs.aaai.org/index.php/ICAPS/article/view/15954)
  - Symbolic-state DFA: [(Zhu et al., 2017)](https://www.ijcai.org/proceedings/2017/0189)

- LTLf synthesis settings:
  - Classical synthesis: [(Zhu et al., 2017)](https://www.ijcai.org/proceedings/2017/0189)
  - MaxSet synthesis: [(Zhu and De Giacomo, 2022)](https://www.ijcai.org/proceedings/2022/386)
  - Synthesis with fairness assumptions: [(Zhu et al., 2020)](https://ojs.aaai.org/index.php/AAAI/article/view/5704)
  - Synthesis with stability assumptions: [(Zhu et al., 2020)](https://ojs.aaai.org/index.php/AAAI/article/view/5704)
  - Synthesis with environment GR(1) assumptions: [(De Giacomo et al., 2022)](https://link.springer.com/article/10.1007/s10703-023-00413-2)

  
Currently, the system has been tested on Ubuntu 24.04 LTS, and should work on other Linux systems. We plan to fully support also MacOS and Windows systems.


## Dependencies

The software depends on the following projects:

- CUDD: CU Decision Diagram package: https://github.com/KavrakiLab/cudd
- MONA (WhiteMech's fork): https://github.com/whitemech/MONA
- Lydia: https://github.com/whitemech/lydia
- Flex & [Bison](https://www.gnu.org/software/bison/)
- [Graphviz](https://graphviz.org/)
- Syfco: the Synthesis Format Conversion Tool: https://github.com/reactive-systems/syfco
- Slugs: SmalL bUt Complete GROne Synthesizer: https://github.com/VerifiableRobotics/slugs/


## Compilation Instructions using CMake

### System-wide dependencies

We assume the software will be built on a Ubuntu 22.04 machine.

First, install the following system-wide dependencies:

```
sudo apt install -y \
   automake \
   cmake    \
   gcc      \
   g++      \
   libtool  \
   wget     \
   unzip
```

### Install CUDD

0.1 Make sure CUDD is installed. CUDD can be found at:

    https://github.com/KavrakiLab/cudd.git

0.2 Install CUDD:

    autoreconf -f -i
    ./configure --enable-silent-rules --enable-obj --enable-dddmp --prefix=/usr/local
    sudo make install

### Install MONA

To install MONA system-wide:

```
git clone --recursive https://github.com/whitemech/MONA.git
cd MONA
git checkout v1.4-19.dev0
./configure && make -j && sudo make -j install
# copy headers manually
sudo mkdir -p /usr/local/include/mona
sudo cp Mem/mem.h Mem/gnuc.h Mem/dlmalloc.h BDD/bdd_external.h BDD/bdd_dump.h BDD/bdd_internal.h BDD/bdd.h BDD/hash.h DFA/dfa.h GTA/gta.h config.h /usr/local/include/mona
```


### Install FLEX, BISON

0.3 Install flex and bison:

    sudo apt-get install flex bison

### Install LYDIA

The tool requires the installation of Lydia, which will be triggered by the CMake configuration.

However, if you want to install Lydia manually, you can co into `submodules/lydia` and follow the installation
instructions in the `README.md`.

### Install Z3

By default, the CMake configuration will fetch z3 automatically from the GitHub repository.
In order to disable this behaviour, you can configure the project by setting -DZ3_FETCH=OFF.

In that case, you have to have the library installed on your system.
To link the static library of z3, you have to install z3 manually:

```
wget https://github.com/Z3Prover/z3/releases/download/z3-4.8.12/z3-4.8.12-x64-glibc-2.31.zip
unzip z3-4.8.12-x64-glibc-2.31.zip
cd z3-4.8.12-x64-glibc-2.31
cp bin/libz3.a /usr/local/lib
cp include/*.h /usr/local/include
```

### Graphviz

For the graphical features (automata and strategy visualization), graphviz need to be installed:

```
sudo apt install graphviz libgraphviz-dev
```

## Build LYDIASYFT

1. Make build folder so your directory is not flooded with build files:

```
mkdir build && cd build
```

3. Run CMake to generate the makefile:

```
cmake -DCMAKE_BUILD_TYPE=Release ..
```

4. Compile using the generated makefile:

```
make -j$(nproc --ignore=1) LydiaSyft
```

4.1. For solving LTLf synthesis with GR(1) conditions, please install `slugs` following submodules/slugs/README.md

5. Compile and Run tests:

```
make -j$(nproc --ignore=1) tests
./bin/tests
```

## Run LYDIASYFT

Usage:

```
LydiaSyft: A compositional synthesizer for Linear Temporal Logic on finite traces (LTLf)
Usage: ./cmake-build-debug/bin/LydiaSyft [OPTIONS] SUBCOMMAND

Options:
  -h,--help                   Print this help message and exit
  --help-all                  Expand all help
  -p,--print-strategy         Print out the synthesized strategy (default: false)
  -t,--print-times            Print out running times of each step (default: false)

Subcommands:
  synthesis                   solve a classical LTLf synthesis problem
  maxset                      solve LTLf synthesis with maximally permissive strategies
  fairness                    solve LTLf synthesis with fairness assumptions
  stability                   solve LTLf synthesis with stability assumptions
  gr1                         Solve LTLf synthesis with GR(1) conditions
```

To see the options of each subcommand, run:

```
LydiaSyft [SUBCOMMAND] --help
```

Examples (run commands from the root directory of the project):

- Classical synthesis:

```
./build/bin/LydiaSyft synthesis -f example/test.tlsf   # UNREALIZABLE
./build/bin/LydiaSyft synthesis -f example/test1.tlsf  # REALIZABLE
```

- Maxset synthesis:

```
./build/bin/LydiaSyft maxset -f example/test1.tlsf  # REALIZABLE
```

- Fairness synthesis:

```
./build/bin/LydiaSyft fairness -f example/fair_stable_test.tlsf -a example/fair_stable_test_assumption.txt  # REALIZABLE
```

- Stability synthesis:

```
./build/bin/LydiaSyft stability -f example/fair_stable_counter_test.tlsf -a example/fair_stable_test_assumption.txt  # REALIZABLE
```

- GR(1) synthesis:

```
./build/bin/LydiaSyft gr1 \
    -f example/TLSF/GR1benchmarks/finding_nemo/finding_nemo_1.tlsf \
    -g example/TLSF/GR1benchmarks/finding_nemo/finding_nemo_1_env_gr1.txt \
    -e example/TLSF/GR1benchmarks/finding_nemo/finding_nemo_1_env_safety.ltlf \
    -a example/TLSF/GR1benchmarks/finding_nemo/finding_nemo_1_agn_safety.ltlf \
    --slugs-path ./submodules/slugs/   # REALIZABLE
```

## Quickstart

The software also provides C++ APIs. Here there is an example:

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


    std::cout << (result.realizability? "" : "NOT ") << "REALIZABLE" << std::endl;
    return 0;
}
```


## Documentation

The documentation is built using Doxygen. First, install `doxygen`:

```
sudo apt install doxygen
```

Then:

```
doxygen Doxyfile
```

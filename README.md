# LydiaSyft: A Compositional Symbolic Synthesis Framework for LTLf Specification

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

    ./configure --enable-silent-rules --enable-obj --enable-dddmp --prefix=[install location]
    sudo make install

    If you get an error about aclocal, this might be due to either
    a. Not having automake:
        sudo apt-get install automake
    b. Needing to reconfigure, do this before configuring:
        autoreconf -i


### Install FLEX, BISON

0.3 Install flex and bison:

    sudo apt-get install flex bison


### Install LYDIA

The tool requires the installation of Lydia, which will be triggered by the CMake configuration.

However, if you want to install Lydia manually, you can co into `submodules/lydia` and follow the installation instructions in the `README.md`.

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

    ```mkdir build && cd build```

3. Run CMake to generate the makefile:

    ```cmake -DCMAKE_BUILD_TYPE=Release ..```

4. Compile using the generated makefile:

    ```make -j$(nproc --ignore=1) LydiaSyft```

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
./build/bin/LydiaSyft maxset -f example/test.tlsf
```

- Fairness synthesis:

```
./build/bin/LydiaSyft .fairness -f example/fair_stable_test.tlsf -a example/fair_stable_test_assumption.txt  # REALIZABLE
```

- Stability synthesis:

```
./build/bin/LydiaSyft stability -f example/fair_stable_counter_test.tlsf -a example/fair_stable_test_assumption.txt  # REALIZABLE
```

- GR(1) synthesis:

```
./build/bin/LydiaSyft gr1 -f example/GR1benchmarks/finding_nemo_agn_goal.tlsf -g example/GR1benchmarks/finding_nemo_env_gr1.txt -e example/GR1benchmarks/finding_nemo_env_safety.ltlf -a example/GR1benchmarks/finding_nemo_agn_safety.ltlf --slugs-path ./submodules/slugs/src/slugs
```

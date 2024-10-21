# CLI Tool

This documentation page shows the usage of the `LydiaSyft` CLI tool.
Currently, it supports five subcommands:

```
> LydiaSyft --help
LydiaSyft: A compositional synthesizer for Linear Temporal Logic on finite traces (LTLf)
Usage: LydiaSyft [OPTIONS] SUBCOMMAND

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

## LTLf Synthesis

Usage:
```
> LydiaSyft synthesis --help
solve a classical LTLf synthesis problem
Usage: LydiaSyft synthesis [OPTIONS]

Options:
  -h,--help                   Print this help message and exit
  --help-all                  Expand all help
  -f,--spec-file TEXT:FILE REQUIRED
                              Specification file
  -s,--syfco-path TEXT:FILE   Path to Syfco binary
```

Example of usage:

```
LydiaSyft synthesis -f examples/test.tlsf   # UNREALIZABLE
LydiaSyft synthesis -f examples/test1.tlsf  # REALIZABLE
```

## LTLf MaxSet Synthesis

Usage:
```
> LydiaSyft maxset --help
solve LTLf synthesis with maximally permissive strategies
Usage: LydiaSyft maxset [OPTIONS]

Options:
  -h,--help                   Print this help message and exit
  --help-all                  Expand all help
  -f,--spec-file TEXT:FILE REQUIRED
                              Specification file
  -s,--syfco-path TEXT:FILE   Path to Syfco binary
```

Example of usage:
```
LydiaSyft maxset -f examples/test1.tlsf  # REALIZABLE
```

## LTLf Synthesis with Fairness Assumptions:

Usage:
```
> LydiaSyft fairness --help
solve LTLf synthesis with fairness assumptions
Usage: LydiaSyft fairness [OPTIONS]

Options:
  -h,--help                   Print this help message and exit
  --help-all                  Expand all help
  -f,--spec-file TEXT:FILE REQUIRED
                              Specification file
  -s,--syfco-path TEXT:FILE   Path to Syfco binary
  -a,--assumption-file TEXT:FILE REQUIRED
                              Assumption file

```

Example of usage:

```
LydiaSyft fairness -f examples/fair_stable_test.tlsf -a examples/fair_stable_test_assumption.txt  # REALIZABLE
```


## LTLf Synthesis with Stability Assumptions

Usage:
```
solve LTLf synthesis with stability assumptions
Usage: LydiaSyft stability [OPTIONS]

Options:
  -h,--help                   Print this help message and exit
  --help-all                  Expand all help
  -f,--spec-file TEXT:FILE REQUIRED
                              Specification file
  -s,--syfco-path TEXT:FILE   Path to Syfco binary
  -a,--assumption-file TEXT:FILE REQUIRED
                              Assumption file
```


Example of usage:
```
LydiaSyft stability -f examples/fair_stable_counter_test.tlsf -a examples/fair_stable_test_assumption.txt  # REALIZABLE
```

## LTLf Synthesis with GR(1) Environment Assumptions

Usage:
```
Solve LTLf synthesis with GR(1) conditions
Usage: LydiaSyft gr1 [OPTIONS]

Options:
  -h,--help                   Print this help message and exit
  --help-all                  Expand all help
  -f,--spec-file TEXT:FILE REQUIRED
                              Specification file
  -s,--syfco-path TEXT:FILE   Path to Syfco binary
  -S,--slugs-path TEXT:DIR    Path to Slugs root directory
  -g,--gr1-file TEXT:FILE REQUIRED
                              GR(1) specification file
  -e,--env-safety-file TEXT:FILE REQUIRED
                              Formula file for the environment safety assumptions
  -a,--agent-safety-file TEXT:FILE REQUIRED
                              Formula file for the agent safety assumptions
```

Example of usage:
```
LydiaSyft gr1 \
    -f examples/TLSF/GR1benchmarks/finding_nemo/finding_nemo_1.tlsf \
    -g examples/TLSF/GR1benchmarks/finding_nemo/finding_nemo_1_env_gr1.txt \
    -e examples/TLSF/GR1benchmarks/finding_nemo/finding_nemo_1_env_safety.ltlf \
    -a examples/TLSF/GR1benchmarks/finding_nemo/finding_nemo_1_agn_safety.ltlf \
    --slugs-path ./submodules/slugs/   # REALIZABLE
```

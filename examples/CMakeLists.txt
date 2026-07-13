
add_subdirectory(01_quickstart)
add_subdirectory(02_dfa_representation)
add_subdirectory(03_dfa_creation_and_manipulation)
add_subdirectory(04_ltlf_synthesis)
add_subdirectory(05_ltlf_synthesis_maximally_permissive)
add_subdirectory(06_ltlf_synthesis_with_fairness_conditions)
add_subdirectory(07_ltlf_synthesis_with_stability_conditions)
add_subdirectory(08_ltlf_synthesis_with_gr1_env_spec)
add_subdirectory(09_reachability)
add_subdirectory(10_buchi_reachability)
add_subdirectory(11_obligation)
add_subdirectory(12_algorithm)
add_subdirectory(12_algorithm_interactive)

add_custom_target(examples)

add_dependencies(examples
    quickstart_example
    dfa_representation_example
    dfa_creation_and_manipulation_example
    ltlf_synthesis_example
    ltlf_synthesis_maximally_permissive_example
    ltlf_synthesis_with_fairness_conditions_example
    ltlf_synthesis_with_stability_conditions_example
    ltlf_synthesis_with_gr1_env_spec_example
    reachability_example
    buchi_reachability_example
    algorithm_example
    algorithm_interactive_example
)

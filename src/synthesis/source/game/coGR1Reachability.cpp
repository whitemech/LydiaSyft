//
// Created by shuzhu on 18/04/24.
//

#include "game/coGR1Reachability.hpp"
#include "string_utilities.h"
#include "Stopwatch.h"

namespace Syft {

    coGR1Reachability::coGR1Reachability(const std::shared_ptr<VarMgr> &var_mgr, const GR1 &gr1,
                                         const SymbolicStateDfa &arena,
                                         const CUDD::BDD &state_space,
                                         const CUDD::BDD &initial_condition,
                                         const std::string &slugs_dir,
                                         const std::string &benchmark_name)
            : var_mgr_(var_mgr), gr1_(gr1), arena_(arena), state_space_(state_space),
              initial_condition_(initial_condition), slugs_dir_{slugs_dir}, benchmark_name_{benchmark_name} {}

    void coGR1Reachability::print_variables(const std::string &filename) const {
        std::ofstream to_slugs;
        to_slugs.open(filename);
        if (to_slugs.is_open()) {

            std::vector<std::string> input_variable_labels = var_mgr_->input_variable_labels();
            std::vector<std::string> output_variable_labels = var_mgr_->output_variable_labels();

            to_slugs << "[INPUT]\n";
            for (std::string output_variable_label: output_variable_labels) {
                to_slugs << to_lower_copy(output_variable_label) << "\n";
            }
            to_slugs << "\n";

            to_slugs << "[OUTPUT]\n";
            for (std::string input_variable_label: input_variable_labels) {
                to_slugs << to_lower_copy(input_variable_label) << "\n";
            }
            size_t state_variable_count = var_mgr_->total_state_variable_count();
            for (size_t i = 0; i < state_variable_count; i++) {
                to_slugs << "z" + std::to_string(i) << "\n";
            }

            to_slugs << "\n";
            to_slugs.close();
        } else {
            std::cout << "Unable to open file " + filename;
        }

    }

    void coGR1Reachability::print_initial_conditions(const CUDD::BDD &arena_initial_state_bdd,
                                                     const std::string &filename) const {
        std::ofstream to_slugs;
        to_slugs.open(filename, std::ios::app);
        if (to_slugs.is_open()) {

            to_slugs << "[ENV_INIT]\n";
            to_slugs << "TRUE\n";


            to_slugs << "[SYS_INIT]\n";

            to_slugs << to_lower_copy(var_mgr_->bdd_to_string(arena_initial_state_bdd));
            to_slugs << "\n\n";

            to_slugs << "\n";
            to_slugs.close();
        } else {
            std::cout << "Unable to open file " + filename;
        }
    }


    void coGR1Reachability::print_transitions(const SymbolicStateDfa &arena, const CUDD::BDD &safe_states,
                                              const std::string &filename) const {
        std::ofstream to_slugs;
        to_slugs.open(filename, std::ios::app);
        if (to_slugs.is_open()) {

            to_slugs << "[ENV_TRANS]\n";
            to_slugs << "TRUE\n\n";

            std::vector<std::string> variable_labels = var_mgr_->variable_labels();
            std::unordered_map<std::string, std::string> rename_map;

            std::vector<std::string> output_variable_labels = var_mgr_->output_variable_labels();
            for (const std::string &output_variable_label: output_variable_labels) {
                std::string output_variable_new_label = output_variable_label + '\'';
                rename_map["(" + output_variable_label + ")"] = "(" + output_variable_new_label + ")";
            }

            std::vector<std::string> input_variable_labels = var_mgr_->input_variable_labels();
            for (const std::string &input_variable_label: input_variable_labels) {
                std::string input_variable_new_label = input_variable_label + '\'';
                rename_map["(" + input_variable_label + ")"] = "(" + input_variable_new_label + ")";
            }

            std::vector<CUDD::BDD> transition_function = arena.transition_function();

            std::unordered_map<std::string, std::string> rename_state_vars_map;
            for (size_t i = 0; i < transition_function.size(); i++) {
                std::string state_variable_new_label = "Z" + std::to_string(i) + "\'";
                rename_state_vars_map["(Z" + std::to_string(i) + ")"] = "(" + state_variable_new_label + ")";
            }

            to_slugs << "[SYS_TRANS]\n";
            for (size_t i = 0; i < transition_function.size(); i++) {
                std::string tran = var_mgr_->bdd_to_string(transition_function[i]);
                for (auto &item: rename_map) {
                    size_t index = 0;
                    while (true) {
                        /* Locate the substring to replace. */
                        index = tran.find(item.first, index);
                        if (index == std::string::npos) break;

                        /* Make the replacement. */
                        tran.replace(index, item.first.size(), item.second);

                        /* Advance index forward so the next iteration doesn't pick it up as well. */
                        index += item.first.size();
                    }
                }
                tran = tran + " <-> " + "Z" + std::to_string(i) + "\'";

                to_slugs << to_lower_copy(tran) << "\n";
            }

            std::string safe_states_string = var_mgr_->bdd_to_string(safe_states);
            to_slugs << to_lower_copy(safe_states_string) << "\n";
            for (auto &item: rename_state_vars_map) {
                size_t index = 0;
                while (true) {
                    /* Locate the substring to replace. */
                    index = safe_states_string.find(item.first, index);
                    if (index == std::string::npos) break;

                    /* Make the replacement. */
                    safe_states_string.replace(index, item.first.size(), item.second);

                    /* Advance index forward so the next iteration doesn't pick it up as well. */
                    index += item.first.size();
                }
            }
            to_slugs << to_lower_copy(safe_states_string) << "\n";
            to_slugs << "\n";

            to_slugs << "\n";
            to_slugs.close();
        } else {
            std::cout << "Unable to open file " + filename;
        }
    }


    void coGR1Reachability::print_liveness_constraints(const std::string &filename) const {
        std::ofstream to_slugs;
        to_slugs.open(filename, std::ios::app);
        if (to_slugs.is_open()) {

            to_slugs << "[ENV_LIVENESS]\n";
            for (const CUDD::BDD &justice: gr1_.env_justices) {
                if (justice == var_mgr_->cudd_mgr()->bddOne() || justice == var_mgr_->cudd_mgr()->bddZero()) {
                    to_slugs << to_upper_copy(var_mgr_->bdd_to_string(justice)) << "\n";
                } else {
                    to_slugs << to_lower_copy(var_mgr_->bdd_to_string(justice)) << "\n";
                }
            }
            to_slugs << "\n\n";

            to_slugs << "[SYS_LIVENESS]\n";
            for (const CUDD::BDD &justice: gr1_.agn_justices) {
                if (justice == var_mgr_->cudd_mgr()->bddOne() || justice == var_mgr_->cudd_mgr()->bddZero()) {
                    to_slugs << to_upper_copy(var_mgr_->bdd_to_string(justice)) << "\n";
                } else {
                    to_slugs << to_lower_copy(var_mgr_->bdd_to_string(justice)) << "\n";
                }
            }
            to_slugs.close();
        } else {
            std::cout << "Unable to open file " + filename;
        }
    }

    const std::string
    coGR1Reachability::exec_slugs(const std::string &slugs, const std::string &slugs_input_file,
                                  const std::string &slugs_res_file,
                                  const std::string &slugs_strategy_file) const {
        std::string result;

        std::string run_slugs =
                slugs + " --counterStrategy " + slugs_input_file + " > " + slugs_strategy_file + " 2> " +
                slugs_res_file;

        const char *cmd = run_slugs.c_str();
        system(cmd);
        std::ifstream in(slugs_res_file);
        std::string line;
        std::vector<std::string> substr;
        std::getline(in, line);
        substr = split(line, ":");
        assert (substr[0] == "SLUGS");

        std::getline(in, line);
        substr = split(line, ":");
        assert (substr[0] == "RESULT");
        trim(substr[1]); // remove leading and trailing whitespace
        substr = split(substr[1], " ");
        if (substr[3] == "unrealizable.") {
            result = "unrealizable";
            std::ifstream in(slugs_strategy_file);
            std::string line;
            std::vector<std::string> substr;
            std::getline(in, line);
            if (line == "") {
                result = "realizable";
            }
        } else {
            result = "realizable";
        }
        return result;
    }


    std::string coGR1Reachability::get_slugs_path() const {
        return slugs_dir_ + "/src/slugs";
    }

    SynthesisResult coGR1Reachability::run() const {
        std::string to_slugs_parser = benchmark_name_ + ".parser";
        print_variables(to_slugs_parser);
        print_initial_conditions(initial_condition_, to_slugs_parser);
        print_transitions(arena_, state_space_, to_slugs_parser);
        print_liveness_constraints(to_slugs_parser);

        Syft::Stopwatch gr1_game_stopwatch; // stopwatch for GR1 game solver Slugs
        gr1_game_stopwatch.start();
        std::cout << "* Start calling GR1 game solver Slugs...\n";

        std::string slugs_input_file = benchmark_name_ + ".slugsin";
        std::string slugs_parser = slugs_dir_ + "/tools/StructuredSlugsParser/compiler.py";
        std::string run_slugs_parser = "python3 " + slugs_parser + " " + to_slugs_parser + " > " + slugs_input_file;

//        std::cout<<run_slugs_parser;
        system(run_slugs_parser.c_str());

        std::string detele_to_slugs_parser = "rm " + to_slugs_parser;
        system(detele_to_slugs_parser.c_str());

        std::string slugs = get_slugs_path();
        std::string slugs_strategy_file = benchmark_name_ + ".strategy";
        std::string slugs_res_file = benchmark_name_ + ".res";
        std::string slugs_result = exec_slugs(slugs, slugs_input_file, slugs_res_file, slugs_strategy_file);

        std::string detele_slugs_input_file = "rm " + slugs_input_file;
        system(detele_slugs_input_file.c_str());

        auto gr1_game_time = gr1_game_stopwatch.stop();
        std::cout << "* Finish calling GR1 game solver, took time: "
                  << gr1_game_time.count() << " ms" << std::endl;

        SynthesisResult result;
        if (slugs_result == "realizable") {
            result.realizability = false;
        } else {
            result.realizability = true;
            result.safe_states = state_space_;
        }
        return result;
    }
}

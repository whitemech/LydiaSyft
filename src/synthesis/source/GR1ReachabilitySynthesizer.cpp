//
// Created by shuzhu on 21/01/24.
//
#include "GR1ReachabilitySynthesizer.h"
#include "string_utilities.h"
#include <iostream>


namespace Syft {

    GR1ReachabilitySynthesizer::GR1ReachabilitySynthesizer(std::shared_ptr<VarMgr> var_mgr, GR1 gr1, SymbolicStateDfa env_safety,
                                           SymbolicStateDfa agn_reach, SymbolicStateDfa agn_safety, std::string slugs_dir)
            : var_mgr_(var_mgr), gr1_(gr1), env_safety_(env_safety), agn_reach_(agn_reach),
              agn_safety_(agn_safety), slugs_dir_{slugs_dir}
    {}

    void GR1ReachabilitySynthesizer::print_variables(const SymbolicStateDfa arena, const std::string& filename) const{
        std::ofstream to_slugs;
        to_slugs.open(filename);
        if(to_slugs.is_open()) {

            std::vector<std::string> input_variable_labels = var_mgr_->input_variable_labels();
            std::vector<std::string> output_variable_labels = var_mgr_->output_variable_labels();

            to_slugs << "[INPUT]\n";
            for (std::string output_variable_label : output_variable_labels) {
                to_slugs << to_lower_copy(output_variable_label) << "\n";
            }
            to_slugs << "\n";

            to_slugs << "[OUTPUT]\n";
            for (std::string input_variable_label : input_variable_labels) {
                to_slugs << to_lower_copy(input_variable_label) << "\n";
            }
            size_t state_variable_count = var_mgr_->total_state_variable_count();
            for (size_t i = 0; i < state_variable_count; i++) {
                to_slugs << "z" + std::to_string(i) << "\n";
            }

            to_slugs << "\n";
            to_slugs.close();
        }
        else{
            std::cout << "Unable to open file "+filename;
        }

    }

    void GR1ReachabilitySynthesizer::print_initial_conditions(const CUDD::BDD& arena_initial_state_bdd, const std::string& filename) const{
        std::ofstream to_slugs;
        to_slugs.open(filename, std::ios::app);
        if(to_slugs.is_open()) {

            to_slugs<<"[ENV_INIT]\n";
            to_slugs<<"TRUE\n";


            to_slugs<<"[SYS_INIT]\n";

            to_slugs<<to_lower_copy(var_mgr_->bdd_to_string(arena_initial_state_bdd));
            to_slugs<<"\n\n";

            to_slugs<<"\n";
            to_slugs.close();
        }
        else{
            std::cout << "Unable to open file "+filename;
        }
    }


    void GR1ReachabilitySynthesizer::print_transitions(const SymbolicStateDfa arena, const std::string& filename) const {
        std::ofstream to_slugs;
        to_slugs.open(filename, std::ios::app);
        if(to_slugs.is_open()) {

            to_slugs<<"[ENV_TRANS]\n";
            to_slugs<<"TRUE\n\n";

            std::vector<std::string> variable_labels = var_mgr_->variable_labels();
            std::unordered_map<std::string, std::string> rename_map;

            std::vector<std::string> output_variable_labels = var_mgr_->output_variable_labels();
            for (std::string output_variable_label : output_variable_labels){
                std::string output_variable_new_label = output_variable_label+'\'';
                rename_map["("+output_variable_label+")"] = "("+output_variable_new_label+")";
            }

            std::vector<std::string> input_variable_labels = var_mgr_->input_variable_labels();
            for (std::string input_variable_label : input_variable_labels){
                std::string input_variable_new_label = input_variable_label+'\'';
                rename_map["("+input_variable_label+")"] = "("+input_variable_new_label+")";
            }

            std::vector<CUDD::BDD> transition_function = arena.transition_function();

            std::unordered_map<std::string, std::string> rename_state_vars_map;
            for (size_t i = 0; i < transition_function.size(); i++){
                std::string state_variable_new_label = "Z" + std::to_string(i) + "\'";
                rename_state_vars_map["(Z" + std::to_string(i)+")"] = "("+state_variable_new_label+")";
            }




            to_slugs<<"[SYS_TRANS]\n";
            for (size_t i = 0; i < transition_function.size(); i++){
                std::string tran = var_mgr_->bdd_to_string(transition_function[i]);
                for (auto& item : rename_map) {
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

                to_slugs<<to_lower_copy(tran)<<"\n";
            }

            std::string safe_states_string = var_mgr_->bdd_to_string(safe_states_);
            to_slugs<<to_lower_copy(safe_states_string)<<"\n";
            for (auto& item : rename_state_vars_map) {
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
            to_slugs<<to_lower_copy(safe_states_string)<<"\n";
            to_slugs<<"\n";

            to_slugs<<"\n";
            to_slugs.close();
        }
        else{
            std::cout << "Unable to open file "+filename;
        }
    }


    void GR1ReachabilitySynthesizer::print_liveness_constraints(const SymbolicStateDfa arena, const std::string& filename) const {
        std::ofstream to_slugs;
        to_slugs.open(filename, std::ios::app);
        if(to_slugs.is_open()) {

            to_slugs<<"[ENV_LIVENESS]\n";
            for (CUDD::BDD justice : gr1_.env_justices){
                if (justice == var_mgr_->cudd_mgr()->bddOne() || justice == var_mgr_->cudd_mgr()->bddZero()) {
                    to_slugs<<to_upper_copy(var_mgr_->bdd_to_string(justice))<<"\n";
                }
                else{
                    to_slugs<<to_lower_copy(var_mgr_->bdd_to_string(justice))<<"\n";
                }
            }
            to_slugs<<"\n\n";

            to_slugs<<"[SYS_LIVENESS]\n";
            for (CUDD::BDD justice : gr1_.agn_justices){
                if (justice == var_mgr_->cudd_mgr()->bddOne() || justice == var_mgr_->cudd_mgr()->bddZero()) {
                    to_slugs<<to_upper_copy(var_mgr_->bdd_to_string(justice))<<"\n";
                }
                else {
                    to_slugs <<to_lower_copy(var_mgr_->bdd_to_string(justice)) << "\n";
                }
            }
            to_slugs.close();
        }
        else{
            std::cout << "Unable to open file "+filename;
        }
    }

    const std::string GR1ReachabilitySynthesizer::exec_slugs(const std::string& slugs, const std::string& slugs_input_file, const std::string& slugs_res_file, const std::string& slugs_strategy_file) {
        std::string result;

        std::string run_slugs = slugs + " --counterStrategy " + slugs_input_file + " > " + slugs_strategy_file + " 2> "+slugs_res_file;

        const char* cmd = run_slugs.c_str();
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
        if (substr[3] == "unrealizable."){
            result = "unrealizable";
            std::ifstream in(slugs_strategy_file);
            std::string line;
            std::vector<std::string> substr;
            std::getline(in, line);
            if (line == ""){
                result = "realizable";
            }
        }
        else{
            result = "realizable";
        }
        return result;
    }

    SynthesisResult GR1ReachabilitySynthesizer::run(std::string& benchmark_name) {
        Syft::Stopwatch reduction_to_gr1_stopwatch; // stopwatch for reducing to GR1
        reduction_to_gr1_stopwatch.start();
        std::cout << "* Start reducing to GR1 game...\n";

        SymbolicStateDfa reach_safe = SymbolicStateDfa::product({agn_reach_, agn_safety_});

        CUDD::BDD reach_goal_t1 = agn_reach_.final_states();
        CUDD::BDD safe_region_t2 = agn_safety_.final_states() | agn_safety_.initial_state_bdd();

        reach_safe.new_sink_states(!safe_region_t2);

        SymbolicStateDfa arena = SymbolicStateDfa::product({env_safety_, reach_safe});
        CUDD::BDD arena_initial_state_bdd = agn_reach_.initial_state_bdd() & agn_safety_.initial_state_bdd() & env_safety_.initial_state_bdd();

        safe_states_ = (env_safety_.final_states() & !(reach_safe.final_states())) | arena_initial_state_bdd;

        std::string to_slugs_parser = benchmark_name + ".parser";
        print_variables(arena, to_slugs_parser);
        print_initial_conditions(arena_initial_state_bdd, to_slugs_parser);
        print_transitions(arena, to_slugs_parser);
        print_liveness_constraints(arena, to_slugs_parser);

        auto reduction_to_gr1_time = reduction_to_gr1_stopwatch.stop();
        std::cout << "* Finish reducing to GR1 game, took time: "
                  << reduction_to_gr1_time.count() << " ms" << std::endl;

        Syft::Stopwatch gr1_game_stopwatch; // stopwatch for GR1 game solver Slugs
        gr1_game_stopwatch.start();
        std::cout << "* Start calling GR1 game solver Slugs...\n";

        std::string slugs_input_file = benchmark_name+".slugsin";
        std::string slugs_parser = slugs_dir_ + "/tools/StructuredSlugsParser/compiler.py";
        std::string run_slugs_parser = "python3 "+ slugs_parser + " " + to_slugs_parser + " > " + slugs_input_file;

//        std::cout<<run_slugs_parser;
        system(run_slugs_parser.c_str());

        std::string detele_to_slugs_parser = "rm "+to_slugs_parser;
        system(detele_to_slugs_parser.c_str());

        std::string slugs= get_slugs_path();
        std::string slugs_strategy_file= benchmark_name+".strategy";
        std::string slugs_res_file= benchmark_name+".res";
        std::string slugs_result = exec_slugs(slugs, slugs_input_file, slugs_res_file, slugs_strategy_file);

        std::string detele_slugs_input_file = "rm "+slugs_input_file;
        system(detele_slugs_input_file.c_str());

        auto gr1_game_time = gr1_game_stopwatch.stop();
        std::cout << "* Finish calling GR1 game solver, took time: "
                  << gr1_game_time.count() << " ms" << std::endl;

        SynthesisResult result;
        if (slugs_result == "realizable"){
            result.realizability = false;
        }
        else{
            result.realizability = true;
        }
        return result;
    }

    std::string GR1ReachabilitySynthesizer::get_slugs_path() {
        return slugs_dir_ + "/src/slugs";
    }


}

#include "ExplicitStateDfa.h"

#include <iostream>
#include <istream>
#include <queue>
#include <set>
#include <bits/stdc++.h>
#include <tuple>
#include <unordered_map>
#include <lydia/mona_ext/mona_ext_base.hpp>
#include <lydia/dfa/mona_dfa.hpp>
#include <lydia/logic/to_ldlf.hpp>
#include <lydia/parser/ldlf/driver.cpp>
#include <lydia/parser/ltlf/driver.cpp>
#include <lydia/to_dfa/core.hpp>
#include <lydia/to_dfa/strategies/compositional/base.hpp>
#include <lydia/utils/print.hpp>

#include "cudd.h"

namespace Syft{

    void ExplicitStateDfa::dfa_print() {
        std::cout<< "Number of states " +
                    std::to_string(get_nb_states())<<"\n";



        std::cout<<"Computed automaton: ";
        whitemech::lydia::dfaPrint(get_dfa(),
                                   get_nb_variables(),
                                   names, indices.data());
    }


    ExplicitStateDfa ExplicitStateDfa::dfa_of_formula(const whitemech::lydia::LTLfFormula& formula) {
//        std::string formula_formatted = boost::algorithm::replace_all_copy(formula, "X", "X[!]");

        auto dfa_strategy = whitemech::lydia::CompositionalStrategy();
        auto translator = whitemech::lydia::Translator(dfa_strategy);


        auto t_start = std::chrono::high_resolution_clock::now();

//        logger.info("Transforming to DFA...");
        auto t_dfa_start = std::chrono::high_resolution_clock::now();

        auto ldlf_formula = whitemech::lydia::to_ldlf(formula);
        auto my_dfa = translator.to_dfa(*ldlf_formula);

        auto my_mona_dfa =
                std::dynamic_pointer_cast<whitemech::lydia::mona_dfa>(my_dfa);

        DFA* d = dfaCopy(my_mona_dfa->dfa_);

        ExplicitStateDfa exp_dfa(d, my_mona_dfa->names);


//        std::cout<< "Number of states " +
//                    std::to_string(exp_dfa.get_nb_variables())<<"\n";


        return exp_dfa;
    }

    // the input d is a bad prefix DFA and agent_winning is the set of winning states that should be kept
    ExplicitStateDfa ExplicitStateDfa::prune_dfa_with_states(ExplicitStateDfa& d, std::vector<size_t> agent_winning) {
//		std::cout << "Before prunning:\n";
//		d.dfa_print();
//        std::cout << "here ....\n";
        int d_ns = d.get_nb_states();
        int new_ns = agent_winning.size();
        int n = d.get_nb_variables();
        int new_len = d.names.size();

//         std::cout << "here ...." << d_ns << ", " << new_ns << ", " << n << ", "<< (n == new_len) << "\n";

        bool safe_states[d_ns];
        int state_map[d_ns];
        memset(safe_states, false, sizeof(safe_states));
        memset(state_map, -1, sizeof(state_map));

        for(auto s : agent_winning) {
            safe_states[s] = true;
        }

        int index = 0;
        for(int i = 0; i < d_ns; i ++) {
            if(! safe_states[i]) continue;
            state_map[i] = index ++;
        }

        DFA* a = d.dfa_;
        DFA* result;
        paths state_paths, pp;
        std::string statuses;

        int indices[new_len];
        for(int i = 0; i < d.indices.size(); i ++) {
            indices[i] = d.indices[i];
        }

        dfaSetup(new_ns + 1, new_len, indices);

        for(int i = 0; i < a->ns; i ++) {
            // ignore non-safe_states
            if(! safe_states[i]) continue;
            int next_state;
            std::string next_guard;

            auto transitions = std::vector<std::pair<int, std::string>>();
            state_paths = pp = make_paths(a->bddm, a->q[i]);
            while(pp) {
                auto guard = whitemech::lydia::get_path_guard(n, pp->trace);
                // ignore non safe_states
                if(safe_states[pp->to]) {
                    transitions.emplace_back(pp->to, guard);
                }
                pp = pp->next;
            }

            statuses += "-";
            // transitions
            int nb_transitions = transitions.size();
            dfaAllocExceptions(nb_transitions);
            for(const auto& p: transitions) {
                std::tie(next_state, next_guard) = p;
                dfaStoreException(state_map[next_state], next_guard.data());
            }
            dfaStoreState(new_ns);
            kill_paths(state_paths);
        }

        statuses += "+";
        dfaAllocExceptions(0);
        dfaStoreState(new_ns);

        DFA* tmp = dfaBuild(statuses.data());

//        std::cout << "Pruned DFA:\n" << statuses << "\n";
        ExplicitStateDfa res1(tmp, d.names);
//        res1.dfa_print();
        result = dfaMinimize(tmp);
        //dfaFree(tmp);

        ExplicitStateDfa res(result, d.names);
        return res;
    }

    std::vector<std::string> ExplicitStateDfa::traverse_bdd(CUDD::BDD curr, std::shared_ptr<VarMgr> var_mgr, std::vector<std::string>& names, std::string guard_str) {
        // guard_str passed by a copy not a reference
        std::vector<std::string> result;
        //    while(!curr.IsZero()){
        if (curr.IsZero()) { // no guard that leads to zero
            return result;
        }

        if (curr.IsOne()) { // return current guard
            result.push_back(guard_str);
            return result;
        }

        //std::cout << "curr.NodeReadIndex: " << curr.NodeReadIndex() << std::endl;
        std::string bdd_var_name = var_mgr->index_to_name(curr.NodeReadIndex());
        std::vector<std::string>::iterator itr = std::find(names.begin(), names.end(), bdd_var_name);
        int var_index;
        if (itr != names.cend()) {
            var_index = std::distance(names.begin(), itr);
        }
        else {
            throw std::runtime_error(
                    "Error: Incorrect winning move.");
        }
        //std::cout << "name: " << bdd_var_name << " index: "<< var_index << std::endl;
        CUDD::BDD bdd_var = var_mgr->name_to_variable(bdd_var_name);
        CUDD::BDD high_cofactor = curr.Cofactor(bdd_var);
        std::vector<std::string> res_high = traverse_bdd(high_cofactor, var_mgr, names, guard_str);
        for (std::string res : res_high)
        {
            res[var_index] = '1';
            result.push_back(res);
        }
        CUDD::BDD low_cofactor = curr.Cofactor(! bdd_var);
        std::vector<std::string> res_low = traverse_bdd(low_cofactor, var_mgr, names,  guard_str);
        for (std::string res : res_low)
        {
            res[var_index] = '0';
            result.push_back(res);
        }
        return result;
    }


    // the input d is a bad prefix DFA and agent_winning is the set of winning transitions that should be kept
    ExplicitStateDfa ExplicitStateDfa::prune_dfa_with_transitions(ExplicitStateDfa &d,
                                                                  std::unordered_map<size_t, CUDD::BDD> winning_moves,
                                                                  std::shared_ptr<VarMgr> var_mgr) {
//        std::cout << "here ....\n";
        int d_ns = d.get_nb_states();
        int new_ns = winning_moves.size();
        int n = d.get_nb_variables();
        int new_len = d.names.size();
        std::vector<std::string> names = d.names;

//        std::cout << "Starting pruning ....\n";
//        d.dfa_print();

        std::vector<bool> safe_states;
        safe_states.resize(d_ns, false);

        std::vector<int> state_map;
        state_map.resize(d_ns, -1);

//        bool safe_states[d_ns];
//        int state_map[d_ns];
//        memset(safe_states, false, sizeof(safe_states));
//        memset(state_map, -1, sizeof(state_map));

        for(auto s : winning_moves) {
            safe_states[s.first] = true;
        }

        int index = 0;
        for(int i = 0; i < d_ns; i ++) {
            if(! safe_states[i]) continue;
            state_map[i] = index ++;
        }

        DFA* a = d.dfa_;
        DFA* result;
        paths state_paths, pp;
        std::string statuses;

        int indices[new_len];
        for(int i = 0; i < d.indices.size(); i ++) {
            indices[i] = d.indices[i];
        }

        dfaSetup(new_ns + 1, new_len, indices);

        for(int i = 0; i < a->ns; i ++) {

            // ignore non-safe_states
            if(! safe_states[i]) continue;
            int next_state;
            std::string next_guard;

            auto transitions = std::vector<std::pair<int, std::string>>();
            state_paths = pp = make_paths(a->bddm, a->q[i]);
            while(pp) {
                auto guard = whitemech::lydia::get_path_guard(n, pp->trace);
                //std::cout<<"current_state: "<<i<<", guard: "<<guard<<", succ_state: "<<pp->to<<"\n";
                CUDD::BDD guard_bdd = var_mgr->cudd_mgr()->bddOne();
                for(int k = 0; k < guard.length(); k++){
                    auto name = d.names[k];
                    auto value = guard.at(k);
                    if(value == '0'){
                        guard_bdd *= !(var_mgr->name_to_variable(name));
                    }
                    else if(value == '1'){
                        guard_bdd *= (var_mgr->name_to_variable(name));
                    }
                    else if(value == 'X'){
                        guard_bdd *= var_mgr->cudd_mgr()->bddOne();
                    }
                    else{
                        throw std::runtime_error(
                                "Error: Incorrect guard.");
                    }
                }
                // ignore non safe_states
                if(safe_states[pp->to]) {
                    //std::cout<<guard_bdd<< ", " << winning_moves[i] <<"\n";
                    CUDD::BDD winning_move = guard_bdd * winning_moves[i];
                    //std::cout<<winning_move<<"\n";
                    auto winning_guard = std::string(n, 'X');
                    //assert(winning_move.IsCube() || winning_move.IsZero() || winning_move.IsOne());
                    if(winning_move.IsZero()){
//                        std::cout<<"current_state: "<<i<<", winning_guard: "<<winning_move<<", succ_state: "<<pp->to<<"\n";
                    } else if(winning_move.IsOne()){
                        transitions.emplace_back(pp->to, winning_guard);
//                        std::cout<<"current_state: "<<i<<", winning_guard: "<<winning_guard<<", succ_state: "<<pp->to<<"\n";
                    } else{
                        CUDD::BDD curr = winning_move;
                        assert (! curr.IsZero() && ! curr.IsOne());

                        std::vector<std::string> result_guards = traverse_bdd(curr, var_mgr, d.names, winning_guard);
                        for (std::string winning_guard: result_guards) {
                            transitions.emplace_back(pp->to, winning_guard);
                            //std::cout << "to: " << (pp->to ) << " guard: " << winning_guard << std::endl;
                        }

                    }
                }
                pp = pp->next;
            }

            statuses += "-";
            // transitions
            int nb_transitions = transitions.size();
            dfaAllocExceptions(nb_transitions);
            for(const auto& p: transitions) {
                std::tie(next_state, next_guard) = p;
                dfaStoreException(state_map[next_state], next_guard.data());
            }
            dfaStoreState(new_ns);
            kill_paths(state_paths);
        }

        statuses += "+";
        dfaAllocExceptions(0);
        dfaStoreState(new_ns);

        DFA* tmp = dfaBuild(statuses.data());

//        std::cout << "Pruned DFA:\n" << statuses << "\n";
        ExplicitStateDfa res1(tmp, d.names);
//        res1.dfa_print();
        result = dfaMinimize(tmp);
        //dfaFree(tmp);

        ExplicitStateDfa res(result, d.names);
        return res;
    }

    // all the names may not be the same, needs a map for right indices
    ExplicitStateDfa ExplicitStateDfa::dfa_product_and(const std::vector<ExplicitStateDfa>& dfa_vector) {
        // first record all variables, as they may not have the same alphabet
        std::unordered_map<std::string, int> name_to_index = {};
        std::vector<std::string> name_vector;
        std::set<std::string> names;
        std::vector<DFA* > renamed_dfa_vector;
        std::vector<std::vector<int>> mappings;

        //1. first collect all names
        for(auto dfa : dfa_vector) {
            // for each DFA, record its names and assign with global indices
            for(int i = 0; i < dfa.names.size(); i ++) {
                if(names.find(dfa.names[i]) == names.end()) {
                    // not found
                    names.insert(dfa.names[i]);
                    name_vector.push_back(dfa.names[i]);
                }
            }
        }
        //2. order the names alphabetically
        // Note: MONA DFA needs the names to be ordered alphabetically
        auto str_cmp = [](const std::string& s1, const std::string& s2) { return s1 > s2; };
        std::priority_queue<std::string, std::vector<std::string>, decltype(str_cmp)>
                str_queue(name_vector.begin(), name_vector.end(), str_cmp);
        int index = 0;
        std::vector<std::string> ordered_name_vector;
        while(str_queue.size()  > 0) {
            std::string name = str_queue.top();
            str_queue.pop();
            name_to_index[name] = index;
            ordered_name_vector.push_back(name);
            index ++;
        }

        for(auto dfa : dfa_vector) {
            // for each DFA, record its names and assign with global indices
            // local index to global index
            int map[ordered_name_vector.size()];
            std::set<int> used_indices;
            std::unordered_map<int, int> bimap = {};
            for(int i = 0; i < dfa.names.size(); i ++) {
                int name_index = name_to_index[dfa.names[i]];
                map[i] = name_index;
                used_indices.insert(name_index);
                bimap[name_index] = i;
            }
            // unused indices
            for(int j = dfa.names.size(); j < ordered_name_vector.size(); j ++) {
                /*if(used_indices.find(j) == used_indices.end()) {
                    // not found
                    map[j] = j;
                }else {
                    // already being used
                    map[j] = bimap[j];
                }*/
                map[j] = -1;
            }
            //4. replace indices
            DFA* copy = dfaCopy(dfa.dfa_);
            dfaReplaceIndices(copy, map);
            renamed_dfa_vector.push_back(copy);

        }

        auto cmp = [](const DFA* d1, const DFA* d2) { return d1->ns > d2->ns; };
        std::priority_queue<DFA*, std::vector<DFA*>, decltype(cmp)>
                queue(renamed_dfa_vector.begin(), renamed_dfa_vector.end(), cmp);
        while(queue.size() > 1) {
            DFA* lhs = queue.top();
            queue.pop();
            DFA* rhs = queue.top();
            queue.pop();
            // Bug: tmp could be a nondeterministic automaton if alphabet is not organized alphabetically
            DFA* tmp = dfaProduct(lhs, rhs, dfaProductType::dfaAND);
            dfaFree(lhs);
            dfaFree(rhs);
            DFA* res = dfaMinimize(tmp);
            dfaFree(tmp);
            queue.push(res);
        }

        ExplicitStateDfa res_dfa(queue.top(), ordered_name_vector);
        /*std::cout << "Product Or: (minimized) \n";
        res_dfa.dfa_print();*/
        return res_dfa;
    }


    // all the names may not be the same, needs a map for right indices
    ExplicitStateDfa ExplicitStateDfa::dfa_product_or(const std::vector<ExplicitStateDfa>& dfa_vector) {
        // first record all variables, as they may not have the same alphabet
        std::unordered_map<std::string, int> name_to_index = {};
        std::vector<std::string> name_vector;
        std::set<std::string> names;
        std::vector<DFA* > renamed_dfa_vector;
        std::vector<std::vector<int>> mappings;

        //1. first collect all names
        for(auto dfa : dfa_vector) {
            // for each DFA, record its names and assign with global indices
            for(int i = 0; i < dfa.names.size(); i ++) {
                if(names.find(dfa.names[i]) == names.end()) {
                    // not found
                    names.insert(dfa.names[i]);
                    name_vector.push_back(dfa.names[i]);
                }
            }
        }
        //2. order the names alphabetically
        // Note: MONA DFA needs the names to be ordered alphabetically
        auto str_cmp = [](const std::string& s1, const std::string& s2) { return s1 > s2; };
        std::priority_queue<std::string, std::vector<std::string>, decltype(str_cmp)>
                str_queue(name_vector.begin(), name_vector.end(), str_cmp);
        int index = 0;
        std::vector<std::string> ordered_name_vector;
        while(str_queue.size()  > 0) {
            std::string name = str_queue.top();
            str_queue.pop();
            name_to_index[name] = index;
            ordered_name_vector.push_back(name);
            index ++;
        }

        for(auto dfa : dfa_vector) {
            // for each DFA, record its names and assign with global indices
            // local index to global index
            int map[ordered_name_vector.size()];
            std::set<int> used_indices;
            std::unordered_map<int, int> bimap = {};
            for(int i = 0; i < dfa.names.size(); i ++) {
                int name_index = name_to_index[dfa.names[i]];
                map[i] = name_index;
                used_indices.insert(name_index);
                bimap[name_index] = i;
            }
            // unused indices
            for(int j = dfa.names.size(); j < ordered_name_vector.size(); j ++) {
                /*if(used_indices.find(j) == used_indices.end()) {
                    // not found
                    map[j] = j;
                }else {
                    // already being used
                    map[j] = bimap[j];
                }*/
                map[j] = -1;
            }
            //4. replace indices
            DFA* copy = dfaCopy(dfa.dfa_);
            dfaReplaceIndices(copy, map);
            renamed_dfa_vector.push_back(copy);

        }

        auto cmp = [](const DFA* d1, const DFA* d2) { return d1->ns > d2->ns; };
        std::priority_queue<DFA*, std::vector<DFA*>, decltype(cmp)>
                queue(renamed_dfa_vector.begin(), renamed_dfa_vector.end(), cmp);
        while(queue.size() > 1) {
            DFA* lhs = queue.top();
            queue.pop();
            DFA* rhs = queue.top();
            queue.pop();
            // Bug: tmp could be a nondeterministic automaton if alphabet is not organized alphabetically
            DFA* tmp = dfaProduct(lhs, rhs, dfaProductType::dfaOR);
            dfaFree(lhs);
            dfaFree(rhs);
            DFA* res = dfaMinimize(tmp);
            dfaFree(tmp);
            queue.push(res);
        }

        ExplicitStateDfa res_dfa(queue.top(), ordered_name_vector);
        /*std::cout << "Product Or: (minimized) \n";
        res_dfa.dfa_print();*/
        return res_dfa;
    }



    ExplicitStateDfa ExplicitStateDfa::dfa_minimize(const ExplicitStateDfa& d) {
        //logger.info("Determinizing DFA...");
        DFA* res = dfaMinimize(d.dfa_);
        ExplicitStateDfa res_dfa(res, d.names);
        return res_dfa;
    }


    ExplicitStateDfa ExplicitStateDfa::dfa_complement(ExplicitStateDfa &d) {
        dfaNegation(d.dfa_);
        ExplicitStateDfa res_dfa(d.dfa_, d.names);
        return res_dfa;
    }



}

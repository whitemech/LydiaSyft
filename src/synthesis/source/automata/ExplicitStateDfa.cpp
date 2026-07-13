#include "automata/ExplicitStateDfa.h"

#include <iostream>
#include <istream>
#include <queue>
#include <set>
#include <bits/stdc++.h>
#include <tuple>
#include <unordered_map>
#include "lydia/mona_ext/mona_ext_base.hpp"
#include "lydia/dfa/mona_dfa.hpp"
#include "lydia/logic/to_ldlf.hpp"
#include "lydia/parser/ldlf/driver.cpp"
#include "lydia/parser/ltlf/driver.cpp"
#include "lydia/to_dfa/core.hpp"
#include "lydia/to_dfa/strategies/compositional/base.hpp"
#include "lydia/utils/print.hpp"



// #include <bits/stdc++.h>

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/strong_components.hpp>
#include <boost/graph/topological_sort.hpp>



#include "cudd.h"

namespace Syft {

    void ExplicitStateDfa::dfa_print() {
        std::cout << "Computed automaton: " << std::endl;
        whitemech::lydia::dfaPrint(get_dfa(),
                                   get_nb_variables(),
                                   names, indices.data());
    }


    ExplicitStateDfa ExplicitStateDfa::dfa_of_formula(const whitemech::lydia::LTLfFormula &formula) {

        auto dfa_strategy = whitemech::lydia::CompositionalStrategy();
        auto translator = whitemech::lydia::Translator(dfa_strategy);


        auto t_start = std::chrono::high_resolution_clock::now();

        auto t_dfa_start = std::chrono::high_resolution_clock::now();

        auto ldlf_formula = whitemech::lydia::to_ldlf(formula);
        auto my_dfa = translator.to_dfa(*ldlf_formula);

        auto my_mona_dfa =
                std::dynamic_pointer_cast<whitemech::lydia::mona_dfa>(my_dfa);

        DFA *d = dfaCopy(my_mona_dfa->dfa_);

        ExplicitStateDfa exp_dfa(d, my_mona_dfa->names);
        return exp_dfa;
    }

    ExplicitStateDfa ExplicitStateDfa::dfa_of_ldlf_formula(const whitemech::lydia::LDLfFormula& ldlf_formula) {

        auto dfa_strategy = whitemech::lydia::CompositionalStrategy();
        auto translator = whitemech::lydia::Translator(dfa_strategy);

        auto t_start = std::chrono::high_resolution_clock::now();

        auto t_dfa_start = std::chrono::high_resolution_clock::now();

        auto my_dfa = translator.to_dfa(ldlf_formula);

        auto my_mona_dfa =
                std::dynamic_pointer_cast<whitemech::lydia::mona_dfa>(my_dfa);

        DFA *d = dfaCopy(my_mona_dfa->dfa_);

        ExplicitStateDfa exp_dfa(d, my_mona_dfa->names);
        return exp_dfa;
    }

    ExplicitStateDfa
    ExplicitStateDfa::restrict_dfa_with_states(ExplicitStateDfa &d, std::vector<size_t> restricted_states) {
        int d_ns = d.get_nb_states();
        int new_ns = restricted_states.size();
        int n = d.get_nb_variables();
        int new_len = d.names.size();

        bool safe_states[d_ns];
        int state_map[d_ns];
        memset(safe_states, false, sizeof(safe_states));
        memset(state_map, -1, sizeof(state_map));

        for (auto s: restricted_states) {
            safe_states[s] = true;
        }

        int index = 0;
        for (int i = 0; i < d_ns; i++) {
            if (!safe_states[i]) continue;
            state_map[i] = index++;
        }

        DFA *a = d.dfa_;
        DFA *result;
        paths state_paths, pp;
        std::string statuses;

        int indices[new_len];
        for (int i = 0; i < d.indices.size(); i++) {
            indices[i] = d.indices[i];
        }

        dfaSetup(new_ns + 1, new_len, indices);

        for (int i = 0; i < a->ns; i++) {
            // ignore non-safe_states
            if (!safe_states[i]) continue;
            int next_state;
            std::string next_guard;

            auto transitions = std::vector<std::pair<int, std::string>>();
            state_paths = pp = make_paths(a->bddm, a->q[i]);
            while (pp) {
                auto guard = whitemech::lydia::get_path_guard(n, pp->trace);
                // ignore non safe_states
                if (safe_states[pp->to]) {
                    transitions.emplace_back(pp->to, guard);
                }
                pp = pp->next;
            }

            statuses += "-";
            // transitions
            int nb_transitions = transitions.size();
            dfaAllocExceptions(nb_transitions);
            for (const auto &p: transitions) {
                std::tie(next_state, next_guard) = p;
                dfaStoreException(state_map[next_state], next_guard.data());
            }
            dfaStoreState(new_ns);
            kill_paths(state_paths);
        }

        statuses += "+";
        dfaAllocExceptions(0);
        dfaStoreState(new_ns);

        DFA *tmp = dfaBuild(statuses.data());
        ExplicitStateDfa res1(tmp, d.names);
        result = dfaMinimize(tmp);
        ExplicitStateDfa res(result, d.names);
        return res;
    }

    std::vector<std::string>
    ExplicitStateDfa::traverse_bdd(CUDD::BDD curr, std::shared_ptr<VarMgr> var_mgr, std::vector<std::string> &names,
                                   std::string guard_str) {

        std::vector<std::string> result;
        if (curr.IsZero()) { // no guard that leads to zero
            return result;
        }

        if (curr.IsOne()) { // return current guard
            result.push_back(guard_str);
            return result;
        }

        std::string bdd_var_name = var_mgr->index_to_name(curr.NodeReadIndex());
        std::vector<std::string>::iterator itr = std::find(names.begin(), names.end(), bdd_var_name);
        int var_index;
        if (itr != names.cend()) {
            var_index = std::distance(names.begin(), itr);
        } else {
            throw std::runtime_error(
                    "Error: Incorrect winning move.");
        }
        CUDD::BDD bdd_var = var_mgr->name_to_variable(bdd_var_name);
        CUDD::BDD high_cofactor = curr.Cofactor(bdd_var);
        std::vector<std::string> res_high = traverse_bdd(high_cofactor, var_mgr, names, guard_str);
        for (std::string res: res_high) {
            res[var_index] = '1';
            result.push_back(res);
        }
        CUDD::BDD low_cofactor = curr.Cofactor(!bdd_var);
        std::vector<std::string> res_low = traverse_bdd(low_cofactor, var_mgr, names, guard_str);
        for (std::string res: res_low) {
            res[var_index] = '0';
            result.push_back(res);
        }
        return result;
    }


    ExplicitStateDfa ExplicitStateDfa::restrict_dfa_with_transitions(ExplicitStateDfa &d,
                                                                     std::unordered_map<size_t, CUDD::BDD> restricted_transitions,
                                                                     std::shared_ptr<VarMgr> var_mgr) {
        int d_ns = d.get_nb_states();
        int new_ns = restricted_transitions.size();
        int n = d.get_nb_variables();
        int new_len = d.names.size();
        std::vector<std::string> names = d.names;

        std::vector<bool> safe_states;
        safe_states.resize(d_ns, false);

        std::vector<int> state_map;
        state_map.resize(d_ns, -1);

        for (auto s: restricted_transitions) {
            safe_states[s.first] = true;
        }

        int index = 0;
        for (int i = 0; i < d_ns; i++) {
            if (!safe_states[i]) continue;
            state_map[i] = index++;
        }

        DFA *a = d.dfa_;
        DFA *result;
        paths state_paths, pp;
        std::string statuses;

        int indices[new_len];
        for (int i = 0; i < d.indices.size(); i++) {
            indices[i] = d.indices[i];
        }

        dfaSetup(new_ns + 1, new_len, indices);

        for (int i = 0; i < a->ns; i++) {

            // ignore non-safe_states
            if (!safe_states[i]) continue;
            int next_state;
            std::string next_guard;

            auto transitions = std::vector<std::pair<int, std::string>>();
            state_paths = pp = make_paths(a->bddm, a->q[i]);
            while (pp) {
                auto guard = whitemech::lydia::get_path_guard(n, pp->trace);

                CUDD::BDD guard_bdd = var_mgr->cudd_mgr()->bddOne();
                for (int k = 0; k < guard.length(); k++) {
                    auto name = d.names[k];
                    auto value = guard.at(k);
                    if (value == '0') {
                        guard_bdd *= !(var_mgr->name_to_variable(name));
                    } else if (value == '1') {
                        guard_bdd *= (var_mgr->name_to_variable(name));
                    } else if (value == 'X') {
                        guard_bdd *= var_mgr->cudd_mgr()->bddOne();
                    } else {
                        throw std::runtime_error(
                                "Error: Incorrect guard.");
                    }
                }
                // ignore non safe_states
                if (safe_states[pp->to]) {
                    CUDD::BDD restricted_move = guard_bdd * restricted_transitions[i];
                    auto restricted_guard = std::string(n, 'X');
                    if (restricted_move.IsZero()) {
                    } else if (restricted_move.IsOne()) {
                        transitions.emplace_back(pp->to, restricted_guard);
                    } else {
                        CUDD::BDD curr = restricted_move;
                        assert (!curr.IsZero() && !curr.IsOne());

                        std::vector<std::string> result_guards = traverse_bdd(curr, var_mgr, d.names, restricted_guard);
                        for (std::string winning_guard: result_guards) {
                            transitions.emplace_back(pp->to, winning_guard);
                        }

                    }
                }
                pp = pp->next;
            }

            statuses += "-";
            // transitions
            int nb_transitions = transitions.size();
            dfaAllocExceptions(nb_transitions);
            for (const auto &p: transitions) {
                std::tie(next_state, next_guard) = p;
                dfaStoreException(state_map[next_state], next_guard.data());
            }
            dfaStoreState(new_ns);
            kill_paths(state_paths);
        }

        statuses += "+";
        dfaAllocExceptions(0);
        dfaStoreState(new_ns);

        DFA *tmp = dfaBuild(statuses.data());
        ExplicitStateDfa res1(tmp, d.names);
        result = dfaMinimize(tmp);
        ExplicitStateDfa res(result, d.names);
        return res;
    }


    ExplicitStateDfa ExplicitStateDfa::dfa_product_and(const std::vector<ExplicitStateDfa> &dfa_vector) {
        // first record all variables, as they may not have the same alphabet
        std::unordered_map<std::string, int> name_to_index = {};
        std::vector<std::string> name_vector;
        std::set<std::string> names;
        std::vector<DFA *> renamed_dfa_vector;
        std::vector<std::vector<int>> mappings;

        //1. first collect all names
        for (auto dfa: dfa_vector) {
            // for each DFA, record its names and assign with global indices
            for (int i = 0; i < dfa.names.size(); i++) {
                if (names.find(dfa.names[i]) == names.end()) {
                    // not found
                    names.insert(dfa.names[i]);
                    name_vector.push_back(dfa.names[i]);
                }
            }
        }
        //2. order the names alphabetically
        // Note: MONA DFA needs the names to be ordered alphabetically
        auto str_cmp = [](const std::string &s1, const std::string &s2) { return s1 > s2; };
        std::priority_queue<std::string, std::vector<std::string>, decltype(str_cmp)>
                str_queue(name_vector.begin(), name_vector.end(), str_cmp);
        int index = 0;
        std::vector<std::string> ordered_name_vector;
        while (str_queue.size() > 0) {
            std::string name = str_queue.top();
            str_queue.pop();
            name_to_index[name] = index;
            ordered_name_vector.push_back(name);
            index++;
        }

        for (auto dfa: dfa_vector) {
            // for each DFA, record its names and assign with global indices
            // local index to global index
            int map[ordered_name_vector.size()];
            std::set<int> used_indices;
            std::unordered_map<int, int> bimap = {};
            for (int i = 0; i < dfa.names.size(); i++) {
                int name_index = name_to_index[dfa.names[i]];
                map[i] = name_index;
                used_indices.insert(name_index);
                bimap[name_index] = i;
            }
            // unused indices
            for (int j = dfa.names.size(); j < ordered_name_vector.size(); j++) {
                map[j] = -1;
            }
            //4. replace indices
            DFA *copy = dfaCopy(dfa.dfa_);
            dfaReplaceIndices(copy, map);
            renamed_dfa_vector.push_back(copy);

        }

        auto cmp = [](const DFA *d1, const DFA *d2) { return d1->ns > d2->ns; };
        std::priority_queue<DFA *, std::vector<DFA *>, decltype(cmp)>
                queue(renamed_dfa_vector.begin(), renamed_dfa_vector.end(), cmp);
        while (queue.size() > 1) {
            DFA *lhs = queue.top();
            queue.pop();
            DFA *rhs = queue.top();
            queue.pop();
            DFA *tmp = dfaProduct(lhs, rhs, dfaProductType::dfaAND);
            dfaFree(lhs);
            dfaFree(rhs);
            DFA *res = dfaMinimize(tmp);
            dfaFree(tmp);
            queue.push(res);
        }

        ExplicitStateDfa res_dfa(queue.top(), ordered_name_vector);
        return res_dfa;
    }

    ExplicitStateDfa ExplicitStateDfa::dfa_product_or(const std::vector<ExplicitStateDfa> &dfa_vector) {
        // first record all variables, as they may not have the same alphabet
        std::unordered_map<std::string, int> name_to_index = {};
        std::vector<std::string> name_vector;
        std::set<std::string> names;
        std::vector<DFA *> renamed_dfa_vector;
        std::vector<std::vector<int>> mappings;

        //1. first collect all names
        for (auto dfa: dfa_vector) {
            // for each DFA, record its names and assign with global indices
            for (int i = 0; i < dfa.names.size(); i++) {
                if (names.find(dfa.names[i]) == names.end()) {
                    // not found
                    names.insert(dfa.names[i]);
                    name_vector.push_back(dfa.names[i]);
                }
            }
        }
        //2. order the names alphabetically
        // Note: MONA DFA needs the names to be ordered alphabetically
        auto str_cmp = [](const std::string &s1, const std::string &s2) { return s1 > s2; };
        std::priority_queue<std::string, std::vector<std::string>, decltype(str_cmp)>
                str_queue(name_vector.begin(), name_vector.end(), str_cmp);
        int index = 0;
        std::vector<std::string> ordered_name_vector;
        while (str_queue.size() > 0) {
            std::string name = str_queue.top();
            str_queue.pop();
            name_to_index[name] = index;
            ordered_name_vector.push_back(name);
            index++;
        }

        for (auto dfa: dfa_vector) {
            // for each DFA, record its names and assign with global indices
            // local index to global index
            int map[ordered_name_vector.size()];
            std::set<int> used_indices;
            std::unordered_map<int, int> bimap = {};
            for (int i = 0; i < dfa.names.size(); i++) {
                int name_index = name_to_index[dfa.names[i]];
                map[i] = name_index;
                used_indices.insert(name_index);
                bimap[name_index] = i;
            }
            // unused indices
            for (int j = dfa.names.size(); j < ordered_name_vector.size(); j++) {
                map[j] = -1;
            }
            //4. replace indices
            DFA *copy = dfaCopy(dfa.dfa_);
            dfaReplaceIndices(copy, map);
            renamed_dfa_vector.push_back(copy);

        }

        auto cmp = [](const DFA *d1, const DFA *d2) { return d1->ns > d2->ns; };
        std::priority_queue<DFA *, std::vector<DFA *>, decltype(cmp)>
                queue(renamed_dfa_vector.begin(), renamed_dfa_vector.end(), cmp);
        while (queue.size() > 1) {
            DFA *lhs = queue.top();
            queue.pop();
            DFA *rhs = queue.top();
            queue.pop();
            DFA *tmp = dfaProduct(lhs, rhs, dfaProductType::dfaOR);
            dfaFree(lhs);
            dfaFree(rhs);
            DFA *res = dfaMinimize(tmp);
            dfaFree(tmp);
            queue.push(res);
        }

        ExplicitStateDfa res_dfa(queue.top(), ordered_name_vector);
        return res_dfa;
    }


    ExplicitStateDfa ExplicitStateDfa::dfa_minimize(const ExplicitStateDfa &d) {
        DFA *res = dfaMinimize(d.dfa_);
        ExplicitStateDfa res_dfa(res, d.names);
        return res_dfa;
    }


    ExplicitStateDfa ExplicitStateDfa::dfa_complement(ExplicitStateDfa &d) {
        DFA* arg = dfaCopy(d.dfa_);
        dfaNegation(arg);
        ExplicitStateDfa res_dfa(arg, d.names);
        return res_dfa;
    }

    //set of functions taken from LydiaSyft Plus tool, to be used for obligation fragment synthesis

    std::vector<size_t> ExplicitStateDfa::get_final() {
        std::vector<size_t> final_states;
        DFA *dfa = get_dfa();
        for (int i = 0; i < dfa->ns; i++){
            if (dfa->f[i] == 1) {
                final_states.push_back(i);
            }
        }
        return final_states;
    }

    std::size_t ExplicitStateDfa::get_initial() {
        DFA *dfa = get_dfa();
        return dfa->s;
    }

    ExplicitStateDfa
    ExplicitStateDfa::dfa_to_Gdfa_obligation(const ExplicitStateDfa &input) {
        ExplicitStateDfa d(input);
        int d_ns = d.get_nb_states();
        int new_ns = d_ns + 1; // add a fresh initial state
        int n = d.get_nb_variables();
        int new_len = d.names.size();

        std::vector<size_t> finals = d.get_final();
        std::vector<bool> is_final(d_ns, false);
        for (auto s : finals) {
            if (s < is_final.size()) {
                is_final[s] = true;
            }
        }

        DFA *a = dfaMinimize(d.dfa_);

        int indices[new_len];
        for (int i = 0; i < d.indices.size(); ++i) {
            indices[i] = d.indices[i];
        }

        dfaSetup(new_ns, new_len, indices);

        std::string statuses;
        statuses.reserve(new_ns + 1);

        auto collect_transitions = [&](int state_idx, int offset) {
            std::vector<std::pair<int, std::string>> transitions;
            paths local_paths = make_paths(a->bddm, a->q[state_idx]);
            paths iter = local_paths;
            while (iter) {
                auto guard = whitemech::lydia::get_path_guard(n, iter->trace);
                transitions.emplace_back(iter->to + offset, guard);
                iter = iter->next;
            }
            kill_paths(local_paths);
            return transitions;
        };

        auto emit_state = [&](const std::vector<std::pair<int, std::string>>& transitions,
                              int default_target) {
            dfaAllocExceptions(static_cast<int>(transitions.size()));
            for (const auto &p : transitions) {
                std::vector<char> guard(p.second.begin(), p.second.end());
                guard.push_back('\0');
                dfaStoreException(p.first, guard.data());
            }
            dfaStoreState(default_target);
        };

        // New non-accepting initial state (index 0) copies behaviour of original initial
        statuses += '-';
        auto initial_transitions = collect_transitions(0, 1);
        int initial_default = initial_transitions.empty() ? 0 : initial_transitions.front().first;
        emit_state(initial_transitions, initial_default);

        // Remaining states correspond to original ones, shifted by +1
        for (int i = 0; i < d_ns; ++i) {
            int new_idx = i + 1;
            if (is_final[i]) {
                statuses += '+';
                auto transitions = collect_transitions(i, 1);
                int default_target = transitions.empty() ? new_idx : transitions.front().first;
                emit_state(transitions, default_target);
            } else {
                statuses += '-';
                dfaAllocExceptions(0);
                dfaStoreState(new_idx);
            }
        }

        statuses.push_back('\0');
        DFA *tmp = dfaBuild(statuses.data());
        ExplicitStateDfa res(tmp, d.names);
        return res;
    }

    ExplicitStateDfa
    ExplicitStateDfa::dfa_to_Fdfa_obligation(const ExplicitStateDfa &input) {
        ExplicitStateDfa d(input);
        int d_ns = d.get_nb_states();
        int new_ns = d_ns + 1; // add a fresh initial state
        int n = d.get_nb_variables();
        int new_len = d.names.size();

        std::vector<size_t> final_states = d.get_final();
        std::vector<bool> is_final(d_ns, false);
        for (auto s : final_states) {
            if (s < is_final.size()) {
                is_final[s] = true;
            }
        }

        DFA *a = dfaMinimize(d.dfa_);

        int indices[new_len];
        for (int i = 0; i < d.indices.size(); ++i) {
            indices[i] = d.indices[i];
        }

        dfaSetup(new_ns, new_len, indices);

        auto collect_transitions = [&](int state_idx, int offset) {
            std::vector<std::pair<int, std::string>> transitions;
            paths local_paths = make_paths(a->bddm, a->q[state_idx]);
            paths iter = local_paths;
            while (iter) {
                auto guard = whitemech::lydia::get_path_guard(n, iter->trace);
                transitions.emplace_back(iter->to + offset, guard);
                iter = iter->next;
            }
            kill_paths(local_paths);
            return transitions;
        };

        auto emit_state = [&](const std::vector<std::pair<int, std::string>>& transitions,
                              int default_target) {
            dfaAllocExceptions(static_cast<int>(transitions.size()));
            for (const auto &p : transitions) {
                std::vector<char> guard(p.second.begin(), p.second.end());
                guard.push_back('\0');
                dfaStoreException(p.first, guard.data());
            }
            dfaStoreState(default_target);
        };

        std::string statuses;
        statuses.reserve(new_ns + 1);

        // New initial state: always rejecting, mimics original initial moves
        statuses += '-';
        auto initial_transitions = collect_transitions(0, 1);
        int initial_default = initial_transitions.empty() ? 0 : initial_transitions.front().first;
        emit_state(initial_transitions, initial_default);

        for (int i = 0; i < d_ns; ++i) {
            int new_idx = i + 1;
            if (is_final[i]) {
                statuses += '+';
                dfaAllocExceptions(0);
                dfaStoreState(new_idx);
            } else {
                statuses += '-';
                auto transitions = collect_transitions(i, 1);
                int default_target = transitions.empty() ? new_idx : transitions.front().first;
                emit_state(transitions, default_target);
            }
        }

        statuses.push_back('\0');
        DFA *tmp = dfaBuild(statuses.data());
        ExplicitStateDfa res(tmp, d.names);
        return res;
    }


    ExplicitStateDfa
    ExplicitStateDfa::dfa_to_Gdfa(ExplicitStateDfa &d) {
        // std::cout << "--------- d:\n";
        // d.dfa_print();
        int d_ns = d.get_nb_states();
        int new_ns = d.get_final().size() + 2; // initial state is "0" and sink state is "new_ns"
        int n = d.get_nb_variables();
        int new_len = d.names.size();

        bool safe_states[d_ns];
        int state_map[d_ns];
        memset(safe_states, false, sizeof(safe_states));
        memset(state_map, -1, sizeof(state_map));

        safe_states[0] = true; // we would like to keep initial state
        for (auto s: d.get_final()) {
            safe_states[s] = true;
        }

        int index = 0;
        for (int i = 0; i < d_ns; i++) {
            if (!safe_states[i]) continue;
            state_map[i] = index++;
        } // relabel all safe states

        DFA *a = d.dfa_;
        DFA *result;
        paths state_paths, pp;
        std::string statuses;

        int indices[new_len];
        for (int i = 0; i < d.indices.size(); i++) {
            indices[i] = d.indices[i];
        }

        dfaSetup(new_ns, new_len, indices);

        for (int i = 0; i < a->ns; i++) {
            // ignore non-safe_states
            if (!safe_states[i]) continue;
            int next_state;
            std::string next_guard;

            auto transitions = std::vector<std::pair<int, std::string>>();
            state_paths = pp = make_paths(a->bddm, a->q[i]);
            while (pp) {
                auto guard = whitemech::lydia::get_path_guard(n, pp->trace);
                // ignore non safe_states
                if (safe_states[pp->to]) {
                    transitions.emplace_back(pp->to, guard);
                }
                pp = pp->next;
            }
            if (i == 0) {
                statuses += "-";
            } else {
                statuses += "+";
            }
//            statuses += "+";
            // transitions
            int nb_transitions = transitions.size();
            dfaAllocExceptions(nb_transitions);
            for (const auto &p: transitions) {
                std::tie(next_state, next_guard) = p;
                dfaStoreException(state_map[next_state], next_guard.data());
            }
            dfaStoreState(new_ns-1);
            kill_paths(state_paths);
        }

        statuses += "-";
        dfaAllocExceptions(0);
        dfaStoreState(new_ns-1);

        DFA *tmp = dfaBuild(statuses.data());
        ExplicitStateDfa res1(tmp, d.names);

        // res1.dfa_print();
        //result = dfaMinimize(tmp);
        ExplicitStateDfa res(tmp, d.names);
        // std::cout << "--------- Gd:\n";
        // res.dfa_print();
        return res;
    }

    ExplicitStateDfa
    ExplicitStateDfa::dfa_to_Fdfa(ExplicitStateDfa &d) {
        int d_ns = d.get_nb_states();
        std::vector<size_t> final_states = d.get_final();
        int n = d.get_nb_variables();
        int new_len = d.names.size();

//        int state_map[d_ns];
//        memset(state_map, -1, sizeof(state_map));
//
//        int index = 0;
//        for (int i = 0; i < d_ns; i++) {
//            state_map[i] = index++;
//        }

        DFA *a = d.dfa_;
        DFA *result;
        paths state_paths, pp;
        std::string statuses;

        int indices[new_len];
        for (int i = 0; i < d.indices.size(); i++) {
            indices[i] = d.indices[i];
        }

        dfaSetup(d_ns, new_len, indices);

        for (int i = 0; i < a->ns; i++) {

            int next_state;
            std::string next_guard;

            auto transitions = std::vector<std::pair<int, std::string>>();
            state_paths = pp = make_paths(a->bddm, a->q[i]);
            auto it = find(final_states.begin(), final_states.end(), i);
            while (pp) {
                auto guard = whitemech::lydia::get_path_guard(n, pp->trace);
                if (it != final_states.end()) {
                    transitions.emplace_back(i, guard);
                } else {
                    transitions.emplace_back(pp->to, guard);
                }

                pp = pp->next;
            }
            if (it != final_states.end()) {
                statuses += "+";
            } else {
                statuses += "-";
            }

            // transitions
            int nb_transitions = transitions.size();
            dfaAllocExceptions(nb_transitions);
            for (const auto &p: transitions) {
                std::tie(next_state, next_guard) = p;
                dfaStoreException(next_state, next_guard.data());
            }
            dfaStoreState(d_ns);
            kill_paths(state_paths);
        }

//        statuses += "+";
//        dfaAllocExceptions(0);
//        dfaStoreState(d_ns);

        DFA *tmp = dfaBuild(statuses.data());
        //result = dfaMinimize(tmp);
        ExplicitStateDfa res(tmp, d.names);
        return res;
    }


    ExplicitStateDfa
    ExplicitStateDfa::dfa_remove_initial_self_loops(ExplicitStateDfa &d) {
        //  std::cout << "--------- remove initial loops:\n";
        // d.dfa_print();
        int d_ns = d.get_nb_states();
        int new_ns = d_ns + 1; // initial state is "0", and new state is new_ns-1
        int n = d.get_nb_variables();
        int new_len = d.names.size();

        std::vector<size_t> final_states = d.get_final();

        DFA *a = d.dfa_;
        DFA *result;
        paths state_paths, pp;
        std::string statuses;

        int indices[new_len];
        for (int i = 0; i < d.indices.size(); i++) {
            indices[i] = d.indices[i];
        }

        dfaSetup(new_ns, new_len, indices);

        int next_state;
        std::string next_guard;

        auto transitions = std::vector<std::pair<int, std::string>>();
        state_paths = pp = make_paths(a->bddm, a->q[0]);
        // auto it = find(final_states.begin(), final_states.end(), i);

        while (pp) {
            auto guard = whitemech::lydia::get_path_guard(n, pp->trace);
            transitions.emplace_back(pp->to+1, guard);
            pp = pp->next;
        }
        statuses += "+";

        // transitions
        int nb_transitions = transitions.size();
        dfaAllocExceptions(nb_transitions);
        for (const auto &p: transitions) {
            std::tie(next_state, next_guard) = p;
            dfaStoreException(next_state, next_guard.data());
        }
        dfaStoreState(new_ns);
        kill_paths(state_paths);

        for (int i = 0; i < a->ns; i++) {
            int next_state;
            std::string next_guard;

            auto transitions = std::vector<std::pair<int, std::string>>();
            state_paths = pp = make_paths(a->bddm, a->q[i]);
            auto it = find(final_states.begin(), final_states.end(), i);

            while (pp) {
                auto guard = whitemech::lydia::get_path_guard(n, pp->trace);
                transitions.emplace_back(pp->to+1, guard);

                pp = pp->next;
            }




            if (it != final_states.end()) {
                statuses += "+";
            } else {
                statuses += "-";
            }

            // transitions
            int nb_transitions = transitions.size();
            dfaAllocExceptions(nb_transitions);
            for (const auto &p: transitions) {
                std::tie(next_state, next_guard) = p;
                dfaStoreException(next_state, next_guard.data());
            }
            dfaStoreState(d_ns);
            kill_paths(state_paths);
        }

//        statuses += "+";
//        dfaAllocExceptions(0);
//        dfaStoreState(d_ns);

        DFA *tmp = dfaBuild(statuses.data());
        // result = dfaMinimize(tmp);
        ExplicitStateDfa res(tmp, d.names);
        return res;
    }

     ExplicitStateDfa ExplicitStateDfa::dfa_minimize_weak(const ExplicitStateDfa &d) {
        DFA* a = d.dfa_;
        int ns = a->ns;
        
        // Build a Boost graph from the DFA
        typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::directedS> Graph;
        Graph g(ns);
        // Store if a vertex has a self-loop
        std::vector<bool> has_self_loop(ns, false);

        // Add edges to the graph
        for (int v = 0; v < ns; v++) {
            paths state_paths = make_paths(a->bddm, a->q[v]);
            paths tp = state_paths;
            std::set<int> successors;
            while (tp) {
                successors.insert(tp->to);
                tp = tp->next;
            }
            kill_paths(state_paths);
            
            for (int succ : successors) {
                boost::add_edge(v, succ, g);
                if (succ == v) {
                    has_self_loop[v] = true;
                }
            }
        }
        
        // Step 1: Compute SCCs using Boost's strong_components
        std::vector<int> scc_id(ns);
        int num_sccs = boost::strong_components(g, &scc_id[0]);
        
        // Analyze SCCs - check if recurrent and accepting
        std::vector<bool> is_recurrent(num_sccs, false);
        std::vector<bool> scc_is_accepting(num_sccs, false);
        std::vector<int> scc_size(num_sccs, 0);
        
        // Count SCC sizes and check if all states in SCC are final
        std::vector<bool> all_final_in_scc(num_sccs, true);
        for (int i = 0; i < ns; i++) {
            scc_size[scc_id[i]]++;
            if (a->f[i] != 1) {
                all_final_in_scc[scc_id[i]] = false;
            }
        }
        
        // Determine which SCCs are recurrent
        for (int scc = 0; scc < num_sccs; scc++) {
            if (scc_size[scc] > 1) {
                // Multi-state SCC is always recurrent
                is_recurrent[scc] = true;
            } else {
                // Single-state SCC is recurrent only if it has a self-loop
                for (int v = 0; v < ns; v++) {
                    if (scc_id[v] == scc) {
                        if (has_self_loop[v]) {
                            is_recurrent[scc] = true;
                            break;
                        }
                    }
                }
            }
            if (is_recurrent[scc]) {
                scc_is_accepting[scc] = all_final_in_scc[scc];
            }
        }
        
        // Step 2: Build SCC graph and compute topological order
        typedef boost::adjacency_list<boost::setS, boost::vecS, boost::directedS> SCCGraph;
        SCCGraph scc_graph(num_sccs);
        
        // Build SCC graph by collapsing edges of g into edges between SCC ids.
        typedef boost::graph_traits<Graph>::edge_iterator edge_iter;
        edge_iter ei, ei_end;
        for (boost::tie(ei, ei_end) = boost::edges(g); ei != ei_end; ++ei) {
            int u = boost::source(*ei, g);
            int v = boost::target(*ei, g);
            int su = scc_id[u];
            int sv = scc_id[v];
            if (su != sv) {
            boost::add_edge(su, sv, scc_graph);
            }
        }
        
        // Compute topological sort
        std::vector<int> topo_order;
        topo_order.reserve(num_sccs);
        try {
            boost::topological_sort(scc_graph, std::back_inserter(topo_order));
        } catch (boost::not_a_dag&) {
            // This shouldn't happen with the SCC graph, but handle it gracefully
            std::cerr << "Warning: SCC graph is not a DAG, using original automaton" << std::endl;
            ExplicitStateDfa result(dfaCopy(a), d.names);
            return result;
        }
            
        // Step 3: Compute maximal coloring following Löding's algorithm (Fig. 1)
        const int k = (num_sccs | 1) + 1;  // Large enough even number
        std::vector<int> scc_color(num_sccs);
        
        for (int vi : topo_order) {
            // Get successor SCCs
            std::set<int> succ_sccs;
            auto edge_range = boost::out_edges(vi, scc_graph);
            for (auto it = edge_range.first; it != edge_range.second; ++it) {
                succ_sccs.insert(boost::target(*it, scc_graph));
            }
            
            if (succ_sccs.empty()) {
                // No successors - terminal SCC
                if (scc_is_accepting[vi]) {
                    scc_color[vi] = k;  // even (accepting)
                } else {
                    scc_color[vi] = k + 1;  // odd (rejecting)
                }
            } else {
                // Has successors - compute minimum successor color
                int min_succ_color = k + 1;
                for (int succ : succ_sccs) {
                    min_succ_color = std::min(min_succ_color, scc_color[succ]);
                }
                
                if (is_recurrent[vi]) {
                    // Recurrent SCC - assign based on acceptance
                    bool is_even = (min_succ_color % 2 == 0);
                    if (is_even && scc_is_accepting[vi]) {
                        scc_color[vi] = min_succ_color;
                    } else if (!is_even && !scc_is_accepting[vi]) {
                        scc_color[vi] = min_succ_color;
                    } else {
                        scc_color[vi] = min_succ_color - 1;
                    }
                } else {
                    // Transient SCC - inherit color from successors
                    scc_color[vi] = min_succ_color;
                }
            }
        }
        // Print out the colouring in SCC order
        spdlog::trace("SCC Coloring Results:");
        for (int scc = 0; scc < num_sccs; scc++) {
                spdlog::trace("SCC {}: Color {}, Recurrent: {}, Accepting: {}",
                              scc, scc_color[scc], is_recurrent[scc], scc_is_accepting[scc]);
        }
        // Step 4: Set final states based on coloring (even = final, odd = non-final)
        DFA* normalized = dfaCopy(a);
        for (int i = 0; i < ns; i++) {
            int color = scc_color[scc_id[i]];
            spdlog::trace("State {} in SCC {} colored {}", i, scc_id[i], color);
            // Log if state was final before
            spdlog::trace("State {} was final before: {}", i, normalized->f[i]);
            normalized->f[i] = (color % 2 == 0) ? 1 : -1;
        }
        
        // Step 5: Apply standard DFA minimization
        DFA* minimized = dfaMinimize(normalized);
        dfaFree(normalized);
        // Log number of states now and before
        // spdlog::info("[ExplicitStateDfa::minise] Number of states before minimization: {}", ns);
        // spdlog::info("[ExplicitStateDfa::minise] Number of states after minimization: {}", minimized->ns);
        ExplicitStateDfa result(minimized, d.names);
        return result;
    }




}

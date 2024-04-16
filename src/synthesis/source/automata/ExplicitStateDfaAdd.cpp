#include "automata/ExplicitStateDfaAdd.h"
#include "string_utilities.h"

#include <algorithm>
#include <fstream>

namespace Syft {

    ExplicitStateDfaAdd::ExplicitStateDfaAdd(std::shared_ptr<VarMgr> var_mgr)
            : var_mgr_(std::move(var_mgr)) {}

    CUDD::ADD ExplicitStateDfaAdd::build_add(
            std::size_t node_index,
            const std::shared_ptr<VarMgr> &var_mgr,
            const std::vector<std::string> &variable_names,
            const std::vector<std::vector<int>> &node_table,
            std::unordered_map<std::size_t, CUDD::ADD> &add_table) {
        auto it = add_table.find(node_index);

        if (it != add_table.end()) {
            return it->second;
        } else {
            int name_index = node_table[node_index][0];
            int low_child = node_table[node_index][1];
            int high_child = node_table[node_index][2];

            if (name_index == -1) {
                add_table[node_index] = var_mgr->cudd_mgr()->constant(low_child);
            } else {
                std::string variable_name = variable_names[name_index];
                CUDD::ADD root_node = var_mgr->name_to_variable(variable_name).Add();
                CUDD::ADD low_node = build_add(low_child, var_mgr, variable_names,
                                               node_table, add_table);
                CUDD::ADD high_node = build_add(high_child, var_mgr, variable_names,
                                                node_table, add_table);

                add_table[node_index] = root_node.Ite(high_node, low_node);
            }

            return add_table[node_index];
        }
    }

    ExplicitStateDfaAdd ExplicitStateDfaAdd::from_dfa_mona(std::shared_ptr<VarMgr> var_mgr,
                                                           const ExplicitStateDfa &explicit_dfa) {
        std::size_t initial_state = explicit_dfa.get_initial_state();

        std::vector<std::string> variable_names = explicit_dfa.names;
        var_mgr->create_named_variables(variable_names);

        std::size_t state_count = explicit_dfa.get_nb_states();
        std::vector<std::size_t> final_states;
        for (int i = 0; i < state_count; i++) {
            std::size_t curr_state = explicit_dfa.get_initial_state() + i;
            if (explicit_dfa.is_final(curr_state)) {
                final_states.push_back(curr_state);
            }
        }
        std::unordered_map<std::size_t, CUDD::ADD> add_table;
        std::vector<CUDD::ADD> transition_function(state_count);

        for (std::size_t i = 0; i < state_count; ++i) {
            transition_function[i] = build_add_from_dfa(explicit_dfa.dfa_->q[i], var_mgr, variable_names,
                                                        explicit_dfa, add_table);
        }

        ExplicitStateDfaAdd dfa(std::move(var_mgr));
        dfa.initial_state_ = initial_state;
        dfa.state_count_ = state_count;
        dfa.final_states_ = std::move(final_states);
        dfa.transition_function_ = std::move(transition_function);

        return dfa;

    }

    CUDD::ADD ExplicitStateDfaAdd::build_add_from_dfa(unsigned node_index, const std::shared_ptr<VarMgr> &var_mgr,
                                                      const std::vector<std::string> &variable_names,
                                                      const ExplicitStateDfa &explicit_dfa,
                                                      std::unordered_map<std::size_t, CUDD::ADD> &add_table) {
        bdd_manager *mgr = explicit_dfa.dfa_->bddm;
        auto it = add_table.find(node_index);

        if (it != add_table.end()) {
            return it->second;
        } else {
            unsigned name_index, low_child, high_child;
            LOAD_lri(&mgr->node_table[node_index], low_child, high_child,
                     name_index);

            if (name_index == BDD_LEAF_INDEX) {
                add_table[node_index] = var_mgr->cudd_mgr()->constant(low_child);
            } else {
                std::string variable_name = variable_names[name_index];
                CUDD::ADD root_node = var_mgr->name_to_variable(variable_name).Add();
                CUDD::ADD low_node = build_add_from_dfa(low_child, var_mgr, variable_names,
                                                        explicit_dfa, add_table);
                CUDD::ADD high_node = build_add_from_dfa(high_child, var_mgr, variable_names,
                                                         explicit_dfa, add_table);

                add_table[node_index] = root_node.Ite(high_node, low_node);
            }

            return add_table[node_index];
        }

    }

    std::shared_ptr<VarMgr> ExplicitStateDfaAdd::var_mgr() const {
        return var_mgr_;
    }

    std::size_t ExplicitStateDfaAdd::initial_state() const {
        return initial_state_;
    }

    std::size_t ExplicitStateDfaAdd::state_count() const {
        return state_count_;
    }

    std::vector<std::size_t> ExplicitStateDfaAdd::final_states() const {
        return final_states_;
    }

    std::vector<CUDD::ADD> ExplicitStateDfaAdd::transition_function() const {
        return transition_function_;
    }

    void ExplicitStateDfaAdd::dump_dot(const std::string &filename) const {
        std::vector<std::string> function_labels(state_count_);

        for (std::size_t i = 0; i < state_count_; ++i) {
            function_labels[i] = "S" + std::to_string(i);
        }

        var_mgr_->dump_dot(transition_function_, function_labels, filename);
    }

};

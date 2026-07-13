#include "automata/SymbolicStateDfa.h"
#include "game/BuchiReachability_multiagent.hpp"
#include <cstdio>

namespace Syft {

    SymbolicStateDfa::SymbolicStateDfa(std::shared_ptr<VarMgr> var_mgr)
            : var_mgr_(std::move(var_mgr)) {}

    SymbolicStateDfa::SymbolicStateDfa(std::shared_ptr<Syft::VarMgr> var_mgr,
        std::size_t automaton_id,
        const std::vector<int>& initial_state,
        const std::vector<CUDD::BDD>& transition_function,
        const CUDD::BDD& final_states) : 
            var_mgr_(std::move(var_mgr)), 
            automaton_id_(automaton_id),
            initial_state_(initial_state),
            transition_function_(transition_function),
            final_states_(final_states) {}

    std::pair<std::size_t, std::size_t> SymbolicStateDfa::create_state_variables(
            std::shared_ptr<VarMgr> &var_mgr,
            std::size_t state_count) {
        // Largest state index that needs to be represented
        std::size_t max_state = state_count - 1;

        std::size_t bit_count = 0;

        // Number of iterations equals the log of the state count
        while (max_state > 0) {
            ++bit_count;
            max_state >>= 1;
        }

        std::size_t automaton_id = var_mgr->create_state_variables(bit_count);

        return std::make_pair(bit_count, automaton_id);
    }

    std::vector<int> SymbolicStateDfa::state_to_binary(std::size_t state,
                                                       std::size_t bit_count) {
        std::vector<int> binary_representation;

        while (state != 0) {
            // Add the least significant bit of the state to the binary representation
            binary_representation.push_back(state & 1);

            // Shift right
            state >>= 1;
        }

        // Fill rest of the vector with zeroes up to bit_count
        binary_representation.resize(bit_count);

        // Note that the binary representation goes from least to most significant bit

        return binary_representation;
    }

    CUDD::BDD SymbolicStateDfa::state_to_bdd(
            const std::shared_ptr<VarMgr> &var_mgr,
            std::size_t automaton_id,
            std::size_t state) {
        std::size_t bit_count = var_mgr->state_variable_count(automaton_id);
        std::vector<int> binary_representation = state_to_binary(state, bit_count);

        return var_mgr->state_vector_to_bdd(automaton_id, binary_representation);
    }

    CUDD::BDD SymbolicStateDfa::state_set_to_bdd(
            const std::shared_ptr<VarMgr> &var_mgr,
            std::size_t automaton_id,
            const std::vector<size_t> &states) {
        CUDD::BDD bdd = var_mgr->cudd_mgr()->bddZero();

        for (std::size_t state: states) {
            bdd |= state_to_bdd(var_mgr, automaton_id, state);
        }

        return bdd;
    }

    std::vector<CUDD::BDD> SymbolicStateDfa::symbolic_transition_function(
            const std::shared_ptr<VarMgr> &var_mgr,
            std::size_t automaton_id,
            const std::vector<CUDD::ADD> &transition_function) {
        std::size_t bit_count = var_mgr->state_variable_count(automaton_id);
        std::vector<CUDD::BDD> symbolic_transition_function(
                bit_count, var_mgr->cudd_mgr()->bddZero());

        for (std::size_t j = 0; j < transition_function.size(); ++j) {
            CUDD::BDD state_bdd = state_to_bdd(var_mgr, automaton_id, j);

            for (std::size_t i = 0; i < bit_count; ++i) {
                // BddIthBit counts from the least-significant bit
                CUDD::BDD jth_component = state_bdd & transition_function[j].BddIthBit(i);

                symbolic_transition_function[i] |= jth_component;
            }
        }

        return symbolic_transition_function;
    }

    SymbolicStateDfa SymbolicStateDfa::from_explicit(
            const ExplicitStateDfaAdd &explicit_dfa) {
        std::shared_ptr<VarMgr> var_mgr = explicit_dfa.var_mgr();

        auto count_and_id = create_state_variables(var_mgr,
                                                   explicit_dfa.state_count());
        std::size_t bit_count = count_and_id.first;
        std::size_t automaton_id = count_and_id.second;

        std::vector<int> initial_state = state_to_binary(explicit_dfa.initial_state(),
                                                         bit_count);

        CUDD::BDD final_states = state_set_to_bdd(var_mgr, automaton_id,
                                                  explicit_dfa.final_states());

        std::vector<CUDD::BDD> transition_function = symbolic_transition_function(
                var_mgr, automaton_id, explicit_dfa.transition_function());

        SymbolicStateDfa symbolic_dfa(var_mgr);
        symbolic_dfa.automaton_id_ = automaton_id;
        symbolic_dfa.initial_state_ = std::move(initial_state);
        symbolic_dfa.final_states_ = std::move(final_states);
        symbolic_dfa.transition_function_ = std::move(transition_function);

        return symbolic_dfa;
    }

    std::shared_ptr<VarMgr> SymbolicStateDfa::var_mgr() const {
        return var_mgr_;
    }

    std::size_t SymbolicStateDfa::automaton_id() const {
        return automaton_id_;
    }

    std::vector<int> SymbolicStateDfa::initial_state() const {
        return initial_state_;
    }

    CUDD::BDD SymbolicStateDfa::initial_state_bdd() const {
        return state_to_bdd(var_mgr_, automaton_id_, 1);
    }

    CUDD::BDD SymbolicStateDfa::final_states() const {
        return final_states_;
    }

    std::vector<CUDD::BDD> SymbolicStateDfa::transition_function() const {
        return transition_function_;
    }

    void SymbolicStateDfa::restrict_dfa_with_states(const CUDD::BDD &valid_states) {
        for (CUDD::BDD &bit_function: transition_function_) {
            // If the current state is not a valid state, send every transition to
            // the sink state 0
            bit_function &= valid_states;
        }

        // Restrict the set of accepting states to valid states
        final_states_ &= valid_states;
    }

    void SymbolicStateDfa::restrict_dfa_with_transitions(const CUDD::BDD &feasible_moves) {
        for (CUDD::BDD &bit_function: transition_function_) {
            // Every transition has to be a feasible move
            bit_function &= feasible_moves;
        }
    }


    void SymbolicStateDfa::dump_dot(const std::string &filename) const {
        std::vector<std::string> function_labels =
                var_mgr_->state_variable_labels(automaton_id_);
        function_labels.push_back("Final");

        std::vector<CUDD::ADD> adds;
        adds.reserve(transition_function_.size() + 1);

        for (const CUDD::BDD &bdd: transition_function_) {
            adds.push_back(bdd.Add());
        }

        adds.push_back(final_states_.Add());

        var_mgr_->dump_dot(adds, function_labels, filename);
    }

    SymbolicStateDfa SymbolicStateDfa::from_predicates(
            std::shared_ptr<VarMgr> var_mgr,
            std::vector<CUDD::BDD> predicates) {
        std::size_t predicate_count = predicates.size();
        std::vector<int> initial_state(predicate_count, 0);
        CUDD::BDD final_states = var_mgr->cudd_mgr()->bddOne();
        std::size_t automaton_id = var_mgr->create_state_variables(predicate_count);

        SymbolicStateDfa dfa(std::move(var_mgr));
        dfa.automaton_id_ = automaton_id;
        dfa.initial_state_ = std::move(initial_state);
        dfa.transition_function_ = std::move(predicates);
        dfa.final_states_ = std::move(final_states);

        return dfa;
    }

    SymbolicStateDfa SymbolicStateDfa::product_AND(const std::vector<SymbolicStateDfa> &dfa_vector) {
        if (dfa_vector.size() < 1) {
            throw std::runtime_error("Incorrect usage of automata product");
        }

        std::shared_ptr<VarMgr> var_mgr = dfa_vector[0].var_mgr();

        std::vector<std::size_t> automaton_ids;

        std::vector<int> initial_state;

        CUDD::BDD final_states = var_mgr->cudd_mgr()->bddOne();
        std::vector<CUDD::BDD> transition_function;

        for (SymbolicStateDfa dfa: dfa_vector) {
            automaton_ids.push_back(dfa.automaton_id());

            std::vector<int> dfa_initial_state = dfa.initial_state();
            initial_state.insert(initial_state.end(), dfa_initial_state.begin(), dfa_initial_state.end());

            final_states = final_states & dfa.final_states();
            std::vector<CUDD::BDD> dfa_transition_function = dfa.transition_function();
            transition_function.insert(transition_function.end(), dfa_transition_function.begin(),
                                       dfa_transition_function.end());
        }

        std::size_t product_automaton_id = var_mgr->create_product_state_space(automaton_ids);

        SymbolicStateDfa product_automaton(var_mgr);
        product_automaton.automaton_id_ = product_automaton_id;
        product_automaton.initial_state_ = std::move(initial_state);
        product_automaton.final_states_ = std::move(final_states);
        product_automaton.transition_function_ = std::move(transition_function);

        return product_automaton;
    }

    void SymbolicStateDfa::new_sink_states(const CUDD::BDD &states) {
        int i = 0;
        while (i < transition_function_.size()) {
            CUDD::BDD bit_function = transition_function_[i];
            CUDD::BDD bit = var_mgr()->state_variable(automaton_id_, i);
//        var_mgr()->dump_dot(bit.Add(), "bit"+std::to_string(i));
            CUDD::BDD new_bit_function = (bit_function & !states) | (states * bit);
            transition_function_[i] = new_bit_function;
            i++;
        }
    }

    SymbolicStateDfa SymbolicStateDfa::product_OR(const std::vector<SymbolicStateDfa> &dfa_vector) {
        if (dfa_vector.size() < 1) {
            throw std::runtime_error("Incorrect usage of automata union");
        }

        std::shared_ptr<VarMgr> var_mgr = dfa_vector[0].var_mgr();

        std::vector<std::size_t> automaton_ids;

        std::vector<int> initial_state;

        CUDD::BDD final_states = var_mgr->cudd_mgr()->bddZero();
        std::vector<CUDD::BDD> transition_function;

        for (SymbolicStateDfa dfa: dfa_vector) {
            automaton_ids.push_back(dfa.automaton_id());

            std::vector<int> dfa_initial_state = dfa.initial_state();
            initial_state.insert(initial_state.end(), dfa_initial_state.begin(), dfa_initial_state.end());

            final_states = final_states | dfa.final_states();
            std::vector<CUDD::BDD> dfa_transition_function = dfa.transition_function();
            transition_function.insert(transition_function.end(), dfa_transition_function.begin(),
                                       dfa_transition_function.end());
        }

        std::size_t union_automaton_id = var_mgr->create_product_state_space(automaton_ids);

        SymbolicStateDfa product_automaton(var_mgr);
        product_automaton.automaton_id_ = union_automaton_id;
        product_automaton.initial_state_ = std::move(initial_state);
        product_automaton.final_states_ = std::move(final_states);
        product_automaton.transition_function_ = std::move(transition_function);

        return product_automaton;
    }


    SymbolicStateDfa SymbolicStateDfa::complement(const SymbolicStateDfa dfa) {
        std::shared_ptr<VarMgr> var_mgr = dfa.var_mgr();

        std::size_t complement_automaton_id = var_mgr->create_complement_state_space(dfa.automaton_id());

        std::vector<int> initial_state = dfa.initial_state();

        CUDD::BDD final_states = !dfa.final_states();


        SymbolicStateDfa complement_automaton(std::move(var_mgr));
        complement_automaton.automaton_id_ = complement_automaton_id;
        complement_automaton.initial_state_ = std::move(initial_state);
        complement_automaton.transition_function_ = std::move(dfa.transition_function());
        complement_automaton.final_states_ = std::move(final_states);

        return complement_automaton;
    }

  
    
SymbolicStateDfa SymbolicStateDfa::get_CORE(const Syft::SymbolicStateDfa &dfa, Actor protagonist_actor, Actor starting_actor) {

    auto var_mgr = dfa.var_mgr();
    //STEP 1: Get winning region for the agent using BuchiReachability
    size_t num_agents = var_mgr->agents_count();
    CUDD::BDD buchi_condition = dfa.final_states();
    CUDD::BDD state_space = var_mgr->cudd_mgr()->bddOne();

    BuchiReachability_multiagent game(dfa, starting_actor, protagonist_actor, buchi_condition, state_space, num_agents);
    SynthesisResult result = game.run();
    std::cout << (protagonist_actor.is_agent() ? "Agent core is realizable: " : "Environment core is realizable: ")
              << result.realizability << std::endl;
    CUDD::BDD winning_region = result.winning_states;

    auto initial_state = dfa.initial_state();
    
    //chech if initial state is in winning region
    //std::cout << "initial state of automata non restricted is in winning region? " << (winning_region.Eval(initial_state.data()).IsOne() ? 1 : 0) << std::endl;
   
    //STEP 2: create a new dfa with the same values of the original Dfa + STEP 3) create new sink
    std::string sink_name = "z_sink_" + (protagonist_actor.is_environment() ? "env" : "agent" + std::to_string(protagonist_actor.id()));
    std::size_t new_id = var_mgr->create_core_state_space(dfa.automaton_id(), sink_name);
    
    
    std::string unique_sink_name = sink_name + "_" + std::to_string(new_id);
    CUDD::BDD z_sink = var_mgr->name_to_variable(unique_sink_name);  
    
    //STEP 4:set BDD for z_sinz when actor is an agent
    //STEP 5:set BDD for z_sink when actor is environment
    std::vector<CUDD::BDD> compose_vector = var_mgr->make_compose_vector(
        dfa.automaton_id(), 
        dfa.transition_function_
    );

    CUDD::BDD next_W = winning_region.VectorCompose(compose_vector);

    CUDD::BDD z_sink_bdd;
    if(protagonist_actor.is_agent()){
        CUDD::BDD external_variables_cube = var_mgr->agent_input_cube(protagonist_actor.id());
        CUDD::BDD forallX_nextW = next_W.UnivAbstract(external_variables_cube);
        CUDD::BDD t_Z_Y = winning_region * forallX_nextW;
        z_sink_bdd = z_sink + (winning_region * !t_Z_Y) + (!winning_region);
    } else {
        CUDD::BDD t_Z_Y_X = winning_region * next_W;
        z_sink_bdd = z_sink + (winning_region * !t_Z_Y_X) + (!winning_region);
    }

    std::vector<int> core_initial_state = dfa.initial_state();
    core_initial_state.push_back(0);
    
    std::vector<CUDD::BDD> core_transition_function = dfa.transition_function();
    core_transition_function.push_back(z_sink_bdd);

    //STEP 6: Modify final states function
    CUDD::BDD core_final_states = dfa.final_states() * winning_region * !z_sink;
    
    SymbolicStateDfa core_dfa(std::move(var_mgr));
    core_dfa.automaton_id_ = new_id;
    core_dfa.initial_state_ = std::move(core_initial_state);
    core_dfa.transition_function_ = std::move(core_transition_function);
    core_dfa.final_states_ = std::move(core_final_states);

    return core_dfa;
}

SymbolicStateDfa SymbolicStateDfa::get_NE(const Syft::SymbolicStateDfa &dfa, Actor protagonist_actor) {
    auto var_mgr = dfa.var_mgr();
    std::string ne_name = "z_ne_" + (protagonist_actor.is_environment() ? "env" : "agent" + std::to_string(protagonist_actor.id()));
    std::size_t new_id = var_mgr->create_nonempty_state_space(dfa.automaton_id(), ne_name);

    std::string unique_ne_name = ne_name + "_" + std::to_string(new_id);
    CUDD::BDD ne_sink = var_mgr->name_to_variable(unique_ne_name);  

    std::vector<int> ne_initial_state = dfa.initial_state();
    ne_initial_state.push_back(0);
    
    std::vector<CUDD::BDD> ne_transition_function = dfa.transition_function();
    ne_transition_function.push_back(var_mgr->cudd_mgr()->bddOne());

    CUDD::BDD ne_final_states = dfa.final_states() * ne_sink;

     
    SymbolicStateDfa ne_dfa(std::move(var_mgr));
    ne_dfa.automaton_id_ = new_id;
    ne_dfa.initial_state_ = std::move(ne_initial_state);
    ne_dfa.transition_function_ = std::move(ne_transition_function);
    ne_dfa.final_states_ = std::move(ne_final_states);

    return ne_dfa;  


}
}


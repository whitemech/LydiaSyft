#include "OneStepUnrealizability.h"
#include "Synthesizer.h"
#include "lydia/logic/nnf.hpp"

#include <lydia/logic/ltlf/base.hpp>

namespace Syft {

    void SmtOneStepUnrealizabilityVisitor::visit(const whitemech::lydia::LTLfTrue &formula) {
        result = z3_context.bool_val(true);
    }

    void SmtOneStepUnrealizabilityVisitor::visit(const whitemech::lydia::LTLfFalse &formula) {
        result = z3_context.bool_val(false);
    }

    void SmtOneStepUnrealizabilityVisitor::visit(const whitemech::lydia::LTLfAtom &formula) {
        assert(whitemech::lydia::is_a<const whitemech::lydia::Symbol>(*formula.symbol));
        std::string varname = formula.symbol->get_name();
        result = z3_context.bool_const(varname.c_str());
    }

    void SmtOneStepUnrealizabilityVisitor::visit(const whitemech::lydia::LTLfNot &formula) {
        if (!whitemech::lydia::is_a<const whitemech::lydia::LTLfAtom>(*formula.get_arg()))
            throw std::logic_error("formula must be in NNF for One-Step Unrealizability Check");
        auto atom_result = apply(*formula.get_arg());
        result = !atom_result;
    }

    void SmtOneStepUnrealizabilityVisitor::visit(const whitemech::lydia::LTLfAnd &formula) {
        z3::expr finalResult = z3_context.bool_val(true);
        for (const auto &subf: formula.get_args()) {
            finalResult = finalResult & apply(*subf);
            finalResult = finalResult.simplify();
            if (z3::eq(finalResult, z3_context.bool_val(false))) {
                break;
            }
        }
        result = finalResult;
    }

    void SmtOneStepUnrealizabilityVisitor::visit(const whitemech::lydia::LTLfOr &formula) {
        z3::expr finalResult = z3_context.bool_val(false);
        for (const auto &subf: formula.get_args()) {
            finalResult = finalResult | apply(*subf);
            finalResult = finalResult.simplify();
            if (z3::eq(finalResult, z3_context.bool_val(true))) {
                break;
            }
        }
        result = finalResult;
    }

    void SmtOneStepUnrealizabilityVisitor::visit(const whitemech::lydia::LTLfNext &formula) {
        result = z3_context.bool_val(true);
    }

    void SmtOneStepUnrealizabilityVisitor::visit(const whitemech::lydia::LTLfWeakNext &formula) {
        result = z3_context.bool_val(true);
    }

    void SmtOneStepUnrealizabilityVisitor::visit(const whitemech::lydia::LTLfUntil &formula) {
        z3::expr finalResult = z3_context.bool_val(false);
        for (const auto &subf: formula.get_args()) {
            finalResult = finalResult | apply(*subf);
            finalResult = finalResult.simplify();
            if (z3::eq(finalResult, z3_context.bool_val(true))) {
                break;
            }
        }
        result = finalResult;
    }

    void SmtOneStepUnrealizabilityVisitor::visit(const whitemech::lydia::LTLfRelease &formula) {
        result = apply(**formula.get_args().rbegin());
    }

    void SmtOneStepUnrealizabilityVisitor::visit(const whitemech::lydia::LTLfEventually &formula) {
        result = z3_context.bool_val(true);
    }

    void SmtOneStepUnrealizabilityVisitor::visit(const whitemech::lydia::LTLfAlways &formula) {
        result = apply(*formula.get_arg());
    }

    z3::expr SmtOneStepUnrealizabilityVisitor::apply(const whitemech::lydia::LTLfFormula &f) {
        f.accept(*this);
        return result;
    }

    z3::expr quantify_agent_vars_(z3::solver &solver, const InputOutputPartition &partition, z3::expr &result) {
        // exist an agent move such that for all env moves...
        z3::expr_vector exist_vars(solver.ctx());
        if (!partition.output_variables.empty()) {
            for (const auto &controllable_var: partition.output_variables) {
                exist_vars.push_back(solver.ctx().bool_const(controllable_var.c_str()));
            }
            result = z3::exists(exist_vars, result);
        }
        return result;
    }

    z3::expr quantify_env_vars_(z3::solver &solver, const InputOutputPartition &partition, z3::expr &result) {
        // for all env moves...
        z3::expr_vector forall_vars(solver.ctx());
        if (!partition.input_variables.empty()) {
            for (const auto &controllable_var: partition.input_variables) {
                forall_vars.push_back(solver.ctx().bool_const(controllable_var.c_str()));
            }
            result = z3::forall(forall_vars, result);
        }
        return result;
    }

    bool one_step_unrealizable(const whitemech::lydia::LTLfFormula &f, const InputOutputPartition &partition,
                               const Syft::VarMgr &var_mgr, Syft::Player starting_player) {

        whitemech::lydia::ltlf_ptr nnf_formula = whitemech::lydia::to_nnf(f);

        z3::context ctx{};
        z3::solver solver{ctx};
        z3::params p(ctx);
        solver.set(p);

        auto visitor = SmtOneStepUnrealizabilityVisitor{partition, var_mgr, ctx, solver, starting_player};
        auto result = visitor.apply(*nnf_formula);

        if (starting_player == Player::Environment) {
            result = quantify_agent_vars_(solver, partition, result);
            result = quantify_env_vars_(solver, partition, result);
        } else {
            result = quantify_env_vars_(solver, partition, result);
            result = quantify_agent_vars_(solver, partition, result);
        }

        solver.reset();
        solver.add(result);

        // if there does not exist Y such that, for all X, the formula is satisfiable,
        // then the formula is unrealizable
        if (!solver.check()) {
            // true means that the check was successful -> the formula is unrealizable
            return true;
        }

        return false;

    }
}
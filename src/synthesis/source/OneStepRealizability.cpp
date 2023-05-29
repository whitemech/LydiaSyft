#include "OneStepRealizability.h"
#include "Synthesizer.h"

#include <lydia/logic/ltlf/base.hpp>

namespace Syft {

  void SmtOneStepRealizabilityVisitor::visit(const whitemech::lydia::LTLfTrue &formula) {
    result = z3_context.bool_val(true);
  }
  void SmtOneStepRealizabilityVisitor::visit(const whitemech::lydia::LTLfFalse &formula) {
    result = z3_context.bool_val(false);
  }

  void SmtOneStepRealizabilityVisitor::visit(const whitemech::lydia::LTLfAtom &formula) {
    assert(whitemech::lydia::is_a<const whitemech::lydia::Symbol>(*formula.symbol));
    std::string varname = formula.symbol->get_name();
    result = z3_context.bool_const(varname.c_str());

    if (partition.is_input(varname)){
      uncontrollableVars.insert(varname);
    }
  }

  void SmtOneStepRealizabilityVisitor::visit(const whitemech::lydia::LTLfNot &formula) {
    result = !apply(*formula.get_arg());
  }

  void SmtOneStepRealizabilityVisitor::visit(const whitemech::lydia::LTLfAnd &formula) {
    z3::expr finalResult = z3_context.bool_val(true);
    for (const auto &subf : formula.get_args()) {
      finalResult = finalResult & apply(*subf);
      finalResult = finalResult.simplify();
      if (z3::eq(finalResult, z3_context.bool_val(false))) {
        break;
      }
    }
    result = finalResult;
  }
  void SmtOneStepRealizabilityVisitor::visit(const whitemech::lydia::LTLfOr &formula) {
    z3::expr finalResult = z3_context.bool_val(false);
    for (const auto &subf : formula.get_args()) {
      finalResult = finalResult | apply(*subf);
      finalResult = finalResult.simplify();
      if (z3::eq(finalResult, z3_context.bool_val(true))) {
        break;
      }
    }
    result = finalResult;
  }

  void SmtOneStepRealizabilityVisitor::visit(const whitemech::lydia::LTLfNext &formula) {
    result = z3_context.bool_val(false);
  }
  void SmtOneStepRealizabilityVisitor::visit(const whitemech::lydia::LTLfWeakNext &formula) {
    result = z3_context.bool_val(true);
  }
  void SmtOneStepRealizabilityVisitor::visit(const whitemech::lydia::LTLfUntil &formula) {
    result = apply(**formula.get_args().rbegin());
  }
  void SmtOneStepRealizabilityVisitor::visit(const whitemech::lydia::LTLfRelease &formula) {
    result = apply(**formula.get_args().rbegin());
  }
  void SmtOneStepRealizabilityVisitor::visit(const whitemech::lydia::LTLfEventually &formula) {
    result = apply(*formula.get_arg());
  }
  void SmtOneStepRealizabilityVisitor::visit(const whitemech::lydia::LTLfAlways &formula) {
    result = apply(*formula.get_arg());
  }

  z3::expr SmtOneStepRealizabilityVisitor::apply(const whitemech::lydia::LTLfFormula &f) {
    f.accept(*this);
    return result;
  }

  std::optional<CUDD::BDD> one_step_realizable(const whitemech::lydia::LTLfFormula &f, const InputOutputPartition &partition, const Syft::VarMgr& var_mgr) {
    z3::context ctx{};
    z3::solver solver{ctx};
    z3::params p(ctx);
    solver.set(p);

    auto visitor = SmtOneStepRealizabilityVisitor{partition, var_mgr, ctx, solver};
    auto result = visitor.apply(f);

    auto quantified = result;
    if (!visitor.uncontrollableVars.empty()){
      z3::expr_vector uncontollables_expr(ctx);
      for (const auto& uncontrollableVar : visitor.uncontrollableVars) {
        uncontollables_expr.push_back(ctx.bool_const(uncontrollableVar.c_str()));
      }
      quantified = z3::forall(uncontollables_expr, result);
    }
    solver.reset();
    solver.add(quantified);

    if (!solver.check()){
      return std::nullopt;
    }

    // build the agent move from Z3 model
    z3::model model = solver.get_model();
    CUDD::BDD move = var_mgr.cudd_mgr()->bddOne();

    for (const auto& controllableVar : visitor.partition.output_variables) {
      auto evaluation = model.eval(ctx.bool_const(controllableVar.c_str()));
      auto bdd_of_var = var_mgr.name_to_variable(controllableVar);
      if (z3::eq(evaluation, ctx.bool_val(true))) {
        move &= bdd_of_var;
      }
      else if (z3::eq(evaluation, ctx.bool_val(false))) {
        move &= !bdd_of_var;
      }
      else{
        // do nothing - variable is a don't care
      }
    }

    return move;
  }
}
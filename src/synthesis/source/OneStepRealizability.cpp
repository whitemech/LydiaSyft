#include "OneStepRealizability.h"

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
    result = !apply(formula);
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

  bool one_step_realizable(const whitemech::lydia::LTLfFormula &f, const InputOutputPartition &partition) {
    z3::context ctx{};
    z3::solver solver{ctx};
    z3::params p(ctx);
    solver.set(p);

    auto visitor = SmtOneStepRealizabilityVisitor{partition, ctx, solver};
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
      return false;
    }

    // TODO extract agent move from model
    return true;
  }
}
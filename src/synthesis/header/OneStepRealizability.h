#ifndef ONE_STEP_REALIZABILITY_HPP
#define ONE_STEP_REALIZABILITY_HPP

#include <z3++.h>
#include <set>
#include <lydia/visitor.hpp>
#include <optional>

#include "game/InputOutputPartition.h"
#include "VarMgr.h"

namespace Syft {


  class SmtOneStepRealizabilityVisitor : public whitemech::lydia::Visitor {
  public:
    InputOutputPartition partition;
    const VarMgr& var_mgr;
    z3::context& z3_context;
    z3::solver& solver;
    z3::expr result;
    std::set<std::string> uncontrollableVars;
    // dummy value for 'result' since z3::expr does not have a default constructor
    explicit SmtOneStepRealizabilityVisitor(const InputOutputPartition &partition, const Syft::VarMgr& var_mgr, z3::context& z3_context, z3::solver& solver)
        : partition{partition}, var_mgr{var_mgr}, z3_context{z3_context}, solver{solver}, result{z3_context.bool_val(true)} {}
    ~SmtOneStepRealizabilityVisitor() {}
    void visit(const whitemech::lydia::LTLfTrue &) override;
    void visit(const whitemech::lydia::LTLfFalse &) override;
    void visit(const whitemech::lydia::LTLfAtom &) override;
    void visit(const whitemech::lydia::LTLfNot &) override;
    void visit(const whitemech::lydia::LTLfAnd &) override;
    void visit(const whitemech::lydia::LTLfOr &) override;
    void visit(const whitemech::lydia::LTLfNext &) override;
    void visit(const whitemech::lydia::LTLfWeakNext &) override;
    void visit(const whitemech::lydia::LTLfUntil &) override;
    void visit(const whitemech::lydia::LTLfRelease &) override;
    void visit(const whitemech::lydia::LTLfEventually &) override;
    void visit(const whitemech::lydia::LTLfAlways &) override;

    z3::expr apply(const whitemech::lydia::LTLfFormula &f);
  };

  std::optional<CUDD::BDD> one_step_realizable(const whitemech::lydia::LTLfFormula &f, const InputOutputPartition &partition, const Syft::VarMgr& var_mgr);

}

#endif //ONE_STEP_REALIZABILITY_HPP

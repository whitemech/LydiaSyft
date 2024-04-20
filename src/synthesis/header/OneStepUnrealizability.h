#ifndef ONE_STEP_UNREALIZABILITY_HPP
#define ONE_STEP_UNREALIZABILITY_HPP

#include <z3++.h>
#include <set>
#include <lydia/visitor.hpp>
#include <optional>

#include "game/InputOutputPartition.h"
#include "Player.h"
#include "VarMgr.h"

namespace Syft {

  /**
  * \brief Implementation of the one-step unrealizability check using Z3.
  *
  * Shengping Xiao, Jianwen Li, Shufang Zhu, Yingying Shi, Geguang Pu, Moshe Y. Vardi. "On-the-fly Synthesis for LTL over Finite Traces". AAAI 2021: 6530-6537
  */
  class SmtOneStepUnrealizabilityVisitor : public whitemech::lydia::Visitor {
  public:
    InputOutputPartition partition;
    const VarMgr& var_mgr;
    z3::context& z3_context;
    z3::solver& solver;
    z3::expr result;
    Syft::Player starting_player;
    // dummy value for 'result' since z3::expr does not have a default constructor
    explicit SmtOneStepUnrealizabilityVisitor(const InputOutputPartition &partition, const Syft::VarMgr& var_mgr, z3::context& z3_context, z3::solver& solver, Syft::Player starting_player)
        : partition{partition}, var_mgr{var_mgr}, z3_context{z3_context}, solver{solver}, result{z3_context.bool_val(true)}, starting_player{starting_player} {}
    ~SmtOneStepUnrealizabilityVisitor() {}
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

  bool one_step_unrealizable(const whitemech::lydia::LTLfFormula &f, const InputOutputPartition &partition, const Syft::VarMgr& var_mgr, Syft::Player starting_player);

}

#endif //ONE_STEP_UNREALIZABILITY_HPP

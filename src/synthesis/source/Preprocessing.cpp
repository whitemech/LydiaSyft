#include "synthesizer/Synthesizer.h"
#include "game/InputOutputPartition.h"
#include "OneStepRealizability.h"
#include "OneStepUnrealizability.h"
#include <lydia/logic/ltlf/base.hpp>

namespace Syft {

    OneStepSynthesisResult
    preprocessing(const whitemech::lydia::LTLfFormula &formula, const InputOutputPartition &partition,
                  const Syft::VarMgr &var_mgr, Syft::Player starting_player) {
        Syft::OneStepSynthesisResult result;

        // one-step realizability check
        auto one_step_realizability_check_result = one_step_realizable(formula, partition, var_mgr);
        if (one_step_realizability_check_result.has_value()) {
            result.realizability = true;
            result.winning_move = one_step_realizability_check_result.value();
            return result;
        }

        // one-step unrealizability check
        bool one_step_unrealizability_check_result = one_step_unrealizable(formula, partition, var_mgr,
                                                                           starting_player);
        if (one_step_unrealizability_check_result) {
            result.realizability = false;
            return result;
        }

        // preprocessing failed
        result.realizability = std::nullopt;
        return result;

    }

}

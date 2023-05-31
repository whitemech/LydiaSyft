#ifndef PREPROCESSING_HPP
#define PREPROCESSING_HPP

#include "Synthesizer.h"
#include "InputOutputPartition.h"
#include <lydia/logic/ltlf/base.hpp>

namespace Syft {

  OneStepSynthesisResult preprocessing(const whitemech::lydia::LTLfFormula& formula, const InputOutputPartition &partition, const Syft::VarMgr& var_mgr);
}

#endif //PREPROCESSING_HPP

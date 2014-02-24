#ifndef _INCLUDED_SYMBOLIC_TYPES_H_
#define _INCLUDED_SYMBOLIC_TYPES_H_

#include "ParseTree.h"

#include "Simulator.h"
#include "DefaultVariable.h"

#include "ValueRange.h"
#include "DefaultParameter.h"
#include "PhaseSimulator.h"

namespace hydla {
namespace simulator {
namespace symbolic {
  typedef std::vector<simulator::PhaseResult>    phase_result_sptrs_t;

  typedef enum Mode_{
    ContinuousMode,
    DiscreteMode,
    ConditionsMode,
  } Mode;
  
} // namespace symbolic
} // namespace simulator
} // namespace hydla

#endif //_INCLUDED_SYMBOLIC_TYPES_H_

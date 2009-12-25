#ifndef _INCLUDED_HYDLA_BP_SIMULATOR_BP_TYPES_H_
#define _INCLUDED_HYDLA_BP_SIMULATOR_BP_TYPES_H_

#include <boost/shared_ptr.hpp>

#include "BPVariable.h"
#include "BPValue.h"
#include "BPTime.h"

// simulator
#include "Simulator.h"

namespace hydla {
namespace bp_simulator {

typedef simulator::VariableMap<BPVariable, BPValue> variable_map_t;
typedef simulator::PhaseState<BPVariable, BPValue, BPTime> phase_state_t;
typedef boost::shared_ptr<phase_state_t> phase_state_sptr;
typedef simulator::Simulator<phase_state_t> simulator_t;

} // namespace bp_simulator
} // namespace hydla

#endif // _INCLUDED_HYDLA_BP_SIMULATOR_BP_TYPES_H_

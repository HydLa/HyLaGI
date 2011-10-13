#ifndef _INCLUDED_HYDLA_BP_SIMULATOR_BP_TYPES_H_
#define _INCLUDED_HYDLA_BP_SIMULATOR_BP_TYPES_H_

#include <boost/shared_ptr.hpp>
#include <boost/bimap/bimap.hpp>
#include <boost/bimap/set_of.hpp>

#include "BPVariable.h"
#include "BPValue.h"
#include "BPTime.h"

// simulator
#include "Simulator.h"

#define BP_PREV_STR "_p"
#define BP_DERIV_STR "_d"
#define BP_INITIAL_STR "_0"

namespace hydla {
namespace bp_simulator {

struct var_property {
public:
  var_property(int c, bool p) : derivative_count(c), prev_flag(p) {} 
  int derivative_count;
  bool prev_flag;
};

typedef boost::bimaps::bimap<
     boost::bimaps::set_of<std::string>,
     boost::bimaps::set_of<int>,
  boost::bimaps::with_info<var_property>
> var_name_map_t;

typedef simulator::VariableMap<BPVariable, BPValue> variable_map_t;
typedef simulator::PhaseState<BPVariable, BPValue, BPTime> phase_state_t;
typedef boost::shared_ptr<phase_state_t> phase_state_sptr;
typedef simulator::Simulator<phase_state_t> simulator_t;

} // namespace bp_simulator
} // namespace hydla

#endif // _INCLUDED_HYDLA_BP_SIMULATOR_BP_TYPES_H_

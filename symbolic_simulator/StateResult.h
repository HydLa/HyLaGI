#ifndef _STATE_RESULT_H_
#define _STATE_RESULT_H_

#include <vector>
#include <boost/shared_ptr.hpp>
#include "SymbolicTypes.h"

namespace hydla {
namespace symbolic_simulator {


struct StateResult {

  typedef enum{
    TIME_LIMIT,
    ERROR,
    INCONSISTENCY,
    ASSERTION,
    NONE
  }CauseOfTermination;
  
  
  variable_map_t variable_map;
  parameter_map_t parameter_map;
  time_t time;
  CauseOfTermination cause_of_termination;
  hydla::simulator::Phase phase_type;
  state_result_sptr_t parent;
  std::vector<state_result_sptr_t> children;
  
  StateResult(){}
  StateResult(const variable_map_t& vm,
              const parameter_map_t& pm,
              const time_t & t,
              const hydla::simulator::Phase phase,
              const state_result_sptr_t& p = state_result_sptr_t(),
              const CauseOfTermination& cause = NONE):
    variable_map(vm), parameter_map(pm), time(t), cause_of_termination(cause), phase_type(phase), parent(p){}
  
};


} // namespace symbolic_simulator
} // namespace hydla 

#endif // _STATE_RESULT_H_

#ifndef _INCLUDED_EPSILON_MODE
#define _INCLUDED_EPSILON_MODE

#include <iostream>
#include <string>
#include <cassert>

#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>

#include "Timer.h"
#include "Logger.h"
#include "PhaseResult.h"
#include "RelationGraph.h"
#include "Simulator.h"
#include "UnsatCoreFinder.h"
#include "AnalysisResultChecker.h"

namespace hydla {
namespace simulator{

  variable_map_t cut_high_order_epsilon(backend::Backend* backend_, phase_result_sptr_t& phase, int diffcnt);
  backend::pp_time_result_t reduce_unsuitable_case(backend::pp_time_result_t time_result, backend::Backend* backend_, phase_result_sptr_t& phase);
  backend::pp_time_result_t pass_specific_case(backend::pp_time_result_t time_result, backend::Backend* backend_, phase_result_sptr_t& phase,
                                               variable_map_t vm_before_time_shift, backend::dc_causes_t dc_causes, value_t time_limit,simulation_todo_sptr_t& current_todo);
  backend::pp_time_result_t zero_case_expansion(backend::pp_time_result_t time_result, backend::Backend* backend_, phase_result_sptr_t& phase);


} //namespace simulator
} //namespace hydla

#endif // _INCLUDED_EPSILON_MODE

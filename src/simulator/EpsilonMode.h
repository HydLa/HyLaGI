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

namespace simulator {

  variable_map_t cut_high_order_epsilon(backend_sptr_t backend_, phase_result_sptr_t phase);
  pp_time_result_t reduce_unsuitable_case(pp_time_result_t time_result, backend_sptr_t backend_, phase_result_sptr_t phase);

} //namespace simulator
} //namespace hydla

#endif // _INCLUDED_EPSILON_MODE

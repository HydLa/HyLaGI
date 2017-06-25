#pragma once

#include <iostream>
#include <string>
#include <cassert>

#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>

#include "Backend.h"
#include "PhaseResult.h"

namespace hydla {
namespace simulator {

  void cut_high_order_epsilon(backend::Backend* backend_, phase_result_sptr_t& phase, int diffcnt);
  pp_time_result_t reduce_unsuitable_case(pp_time_result_t original_result, backend::Backend* backend_, phase_result_sptr_t& phase);
  find_min_time_result_t reduce_unsuitable_case(find_min_time_result_t original_result, backend::Backend* backend_, phase_result_sptr_t& phase);

  find_min_time_result_t find_min_time_epsilon(symbolic_expression::node_sptr trigger, variable_map_t &vm, value_t time_limit, phase_result_sptr_t& phase, backend::Backend* backend);
  find_min_time_result_t calculate_tmp_result(phase_result_sptr_t& phase,value_t time_limit,  symbolic_expression::node_sptr
                                            trigger,backend::Backend* backend_, variable_map_t vm_before_time_shift);

} // namespace simulator
} // namespace hydla

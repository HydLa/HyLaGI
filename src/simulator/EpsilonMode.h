#pragma once

#include <iostream>
#include <string>
#include <cassert>

#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>

#include "Backend.h"
#include "PhaseResult.h"


namespace hydla {
namespace simulator{

variable_map_t cut_high_order_epsilon(backend::Backend* backend_, phase_result_sptr_t& phase);
pp_time_result_t reduce_unsuitable_case(pp_time_result_t time_result, backend::Backend* backend_, phase_result_sptr_t& phase);

} //namespace simulator
} //namespace hydla

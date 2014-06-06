#ifndef _INCLUDED_EPSILON_MODE
#define _INCLUDED_EPSILON_MODE

#include <iostream>
#include <string>
#include <cassert>

#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>


namespace hydla {
namespace simulator{

  variable_map_t cut_high_order_epsilon(backend::Backend* backend_, phase_result_sptr_t& phase);
  backend::pp_time_result_t reduce_unsuitable_case(backend::pp_time_result_t time_result, backend::Backend* backend_, phase_result_sptr_t& phase);

} //namespace simulator
} //namespace hydla

#endif // _INCLUDED_EPSILON_MODE

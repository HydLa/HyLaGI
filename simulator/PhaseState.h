#ifndef _INCLUDED_HYDLA_SIMULATOR_PHASE_STATE_H_
#define _INCLUDED_HYDLA_SIMULATOR_PHASE_STATE_H_

#include <vector>

#include <boost/shared_ptr.hpp>

#include "./VariableMap.h"
#include "./Types.h"

namespace hydla {
namespace simulator {

/**
 * äeèàóùÇÃèÛë‘
 */
template<typename VariableType, 
         typename ValueType, 
         typename TimeType>
struct PhaseState {
  typedef VariableType                                      variable_t;
  typedef ValueType                                         value_t;
  typedef TimeType                                          time_t;
  typedef VariableMap<variable_t, value_t>                  variable_map_t;

  Phase                     phase;
  TimeType                  current_time;
  variable_map_t            variable_map;
  expanded_always_id_t      expanded_always_id;
  changed_asks_t            changed_asks;
  module_set_container_sptr module_set_container;
};


} // namespace simulator
} // namespace hydla 

#endif // _INCLUDED_HYDLA_SIMULATOR_PHASE_STATE_H_


#ifndef _INCLUDED_SYMBOLIC_TYPES_H_
#define _INCLUDED_SYMBOLIC_TYPES_H_

#include "ParseTree.h"

#include "Simulator.h"
#include "DefaultVariable.h"

#include "SymbolicValue.h"
#include "ValueRange.h"
#include "DefaultParameter.h"
#include "PhaseSimulator.h"

namespace hydla {
namespace symbolic_simulator {
  typedef simulator::module_set_sptr             module_set_sptr;
  typedef simulator::module_set_container_sptr   module_set_container_sptr;
  typedef simulator::module_set_list_t           module_set_list_t;

  typedef boost::shared_ptr<simulator::Value>               value_t;
  typedef value_t                                time_t;
  typedef simulator::simulation_todo_sptr_t     simulation_todo_sptr_t;
  typedef simulator::PhaseSimulator::todo_list_t          todo_list_t;
  typedef std::vector<simulator::PhaseResult>    phase_result_sptrs_t;
  typedef simulator::variable_t       variable_t;
  typedef simulator::DefaultParameter            parameter_t;
  typedef simulator::ValueRange                  value_range_t;
  typedef simulator::variable_map_t              variable_map_t;
  typedef simulator::variable_range_map_t        variable_range_map_t;
  typedef simulator::parameter_map_t             parameter_map_t;
  typedef simulator::continuity_map_t            continuity_map_t;
  typedef simulator::constraints_t               constraints_t;
  typedef simulator::tells_t                     tells_t;
  typedef simulator::collected_tells_t           collected_tells_t;
  typedef simulator::expanded_always_t           expanded_always_t;
  typedef simulator::ask_set_t                   ask_set_t;
  typedef simulator::positive_asks_t             positive_asks_t;
  typedef simulator::negative_asks_t             negative_asks_t;
  typedef simulator::not_adopted_tells_list_t    not_adopted_tells_list_t;
  typedef simulator::continuity_map_t            continuity_map_t;
  typedef simulator::variable_set_t   variable_set_t;
  typedef simulator::parameter_set_t  parameter_set_t;
  
  typedef simulator::parse_tree_sptr  parse_tree_sptr;
  
  typedef simulator::PhaseSimulator              simulator_t;

  typedef enum Mode_{
    ContinuousMode,
    DiscreteMode,
    FalseConditionsMode,
  } Mode;
  

} // namespace symbolic_simulator
} // namespace hydla

#endif //_INCLUDED_SYMBOLIC_TYPES_H_

#ifndef _INCLUDED_SYMBOLIC_TYPES_H_
#define _INCLUDED_SYMBOLIC_TYPES_H_

#include "ParseTree.h"

#include "Simulator.h"
#include "DefaultVariable.h"

#include "SymbolicValue.h"
#include "ValueRange.h"
#include "DefaultParameter.h"

#include "Types.h"

namespace hydla {
namespace symbolic_simulator {

  
  typedef simulator::node_id_t                   node_id_t;
  typedef simulator::module_set_sptr             module_set_sptr;
  typedef simulator::module_set_container_sptr   module_set_container_sptr;
  typedef simulator::module_set_list_t           module_set_list_t;

  typedef SymbolicValue                          value_t;
  typedef SymbolicValue                          time_t;
  typedef simulator::PhaseResult<value_t>        phase_result_t;
  typedef phase_result_t::phase_result_sptr_t    phase_result_sptr_t;
  typedef std::vector<phase_result_sptr_t>       phase_result_sptrs_t;
  typedef simulator::DefaultVariable             variable_t;
  typedef simulator::DefaultParameter<value_t>   parameter_t;
  typedef simulator::ValueRange<value_t>         value_range_t;
  typedef simulator::VariableMap<variable_t*, 
                                        value_t> variable_map_t;
  typedef simulator::VariableMap<parameter_t*, 
                                        value_range_t> parameter_map_t;
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
  typedef simulator::Simulator<phase_result_t>::variable_set_t  variable_set_t;
  typedef simulator::Simulator<phase_result_t>::parameter_set_t  parameter_set_t;
  
  typedef simulator::parse_tree_sptr  parse_tree_sptr;
                                        

  typedef boost::shared_ptr<phase_result_t> phase_result_sptr;
  
  typedef simulator::PhaseSimulator<phase_result_t> simulator_t;
  
  typedef simulator_t::Phases                    Phases;

  typedef std::vector<phase_result_sptr> CalculateClosureResult;

  typedef enum Mode_{
    ContinuousMode,
    DiscreteMode,
  } Mode;
  

} // namespace symbolic_simulator
} // namespace hydla

#endif //_INCLUDED_SYMBOLIC_TYPES_H_

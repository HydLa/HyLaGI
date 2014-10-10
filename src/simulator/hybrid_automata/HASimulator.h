#pragma once

#include "HybridAutomata.h"

#include <map>
#include <string>


namespace hydla {
namespace simulator {

class HASimulator: public HybridAutomata{
	
public:
	
	HASimulator(Opts &opts);

  void set_ha_results(const ha_results_t& ha_results);

  virtual ~HASimulator();

  virtual phase_result_sptr_t simulate();
	
	parameter_map_t get_init_vm(phase_result_sptr_t pr);
	ha_result_t get_ha(ha_results_t ha_results);
	void substitute(phase_result_sptr_t pr, parameter_map_t vm);
	parameter_map_t update_vm(phase_result_sptr_t pr, parameter_map_t vm_pre);

private:
  value_t simplify(symbolic_expression::node_sptr exp);
  ha_results_t ha_results;

};//HASimulator

}//namespace hydla
}//namespace simulator 

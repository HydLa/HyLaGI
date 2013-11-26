
#ifndef _INCLUDED_HYDLA_HASimulator_SIMULATOR_H_
#define _INCLUDED_HYDLA_HASimulator_SIMULATOR_H_

#include "HybridAutomata.h"

#include <map>
#include <string>


namespace hydla {
namespace simulator {

class HASimulator: public HybridAutomata{
	
public:
	
	HASimulator(Opts &opts);

  virtual ~HASimulator();

  phase_result_const_sptr_t simulate(ha_results_t ha_results);
  virtual phase_result_const_sptr_t simulate();

	typedef std::map<parameter_t*, value_t>    init_value_map_t;
	
	init_value_map_t get_init_vm(phase_result_sptr_t pr);
	ha_result_t get_ha(ha_results_t ha_results);
	void substitute(phase_result_sptr_t pr, init_value_map_t vm, time_t current_time);
	init_value_map_t update_vm(phase_result_sptr_t pr, init_value_map_t vm_pre);
	

};//HASimulator

}//namespace hydla
}//namespace simulator 

#endif // _INCLUDED_HYDLA_HASimulator_SIMULATOR_H_


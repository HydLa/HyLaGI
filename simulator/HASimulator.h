
#ifndef _INCLUDED_HYDLA_HASimulator_SIMULATOR_H_
#define _INCLUDED_HYDLA_HASimulator_SIMULATOR_H_

#include "HAConverter.h"
#include "BatchSimulator.h"
#include "Node.h"
#include "NodeAccessor.h"
#include "DefaultTreeVisitor.h"
#include "PhaseResult.h"

#include <map>
#include <string>


namespace hydla {
namespace simulator {

class HASimulator: public BatchSimulator{
public:
	
	HASimulator(Opts &opts);

  virtual ~HASimulator();

  phase_result_const_sptr_t simulate(HAConverter::ha_results_t ha_results);
  virtual phase_result_const_sptr_t simulate();

	typedef hydla::ch::module_set_sptr 												module_set_sptr_t;
	typedef std::vector<module_set_sptr_t>					 					module_set_sptrs_t;

	typedef phase_result_sptrs_t              ha_result_t;
	typedef std::deque<ha_result_t> 					ha_results_t;

	typedef std::map<parameter_t*, value_t>    init_value_map_t;
	
	init_value_map_t get_init_vm(phase_result_sptr_t pr);
	ha_result_t get_ha(ha_results_t ha_results);
	void substitute(phase_result_sptr_t pr, init_value_map_t vm, time_t current_time);
	init_value_map_t update_vm(phase_result_sptr_t pr, init_value_map_t vm_pre);
	
protected: 
	
	// phase_result_sptrs_t�̒��g�\��
	void viewPrs(phase_result_sptrs_t results);
	// phase_result_sptr_t�̒��g�\��
	void viewPr(phase_result_sptr_t result);
	// asks�̒��g�\��
	void viewAsks(ask_set_t asks);

};//HASimulator

}//namespace hydla
}//namespace simulator 

#endif // _INCLUDED_HYDLA_HASimulator_SIMULATOR_H_



#ifndef _INCLUDED_HYDLA_HybridAutomata_SIMULATOR_H_
#define _INCLUDED_HYDLA_HybridAutomata_SIMULATOR_H_


#include "BatchSimulator.h"
#include "Node.h"
#include "NodeAccessor.h"
#include "DefaultTreeVisitor.h"
#include "PhaseResult.h"

#include <map>
#include <string>


namespace hydla {
namespace simulator {
    
class HybridAutomata: public BatchSimulator{
public:
      
  HybridAutomata(Opts &opts);
  virtual ~HybridAutomata();

  typedef hydla::ch::module_set_sptr module_set_sptr_t;
  typedef std::vector<module_set_sptr_t> module_set_sptrs_t;

  typedef phase_result_sptrs_t													 current_condition_t;
  typedef std::deque<current_condition_t> 					 		 current_conditions_t;
  typedef phase_result_sptrs_t              ha_result_t;
  typedef std::deque<current_condition_t> 	ha_results_t;

protected:

  // phase_result_sptrs_tの中身表示
  void viewPrs(phase_result_sptrs_t results);
  // phase_result_sptr_tの中身表示
  void viewPr(phase_result_sptr_t result);
  // asksの中身表示
  void viewAsks(ask_set_t asks);
      
};//HybridAutomata

}//namespace hydla
}//namespace simulator 

#endif // _INCLUDED_HYDLA_HybridAutomata_SIMULATOR_H_

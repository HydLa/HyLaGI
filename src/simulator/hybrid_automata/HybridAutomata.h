#pragma once

#include "Simulator.h"
#include "Node.h"
#include "NodeAccessor.h"
#include "DefaultTreeVisitor.h"
#include "PhaseResult.h"

#include <map>
#include <string>


namespace hydla {
namespace simulator {
    
class HybridAutomata: public Simulator{
public:
      
  HybridAutomata(Opts &opts);
  virtual ~HybridAutomata();

  typedef phase_result_sptrs_t              ha_result_t;
  typedef std::deque<ha_result_t> 	        ha_results_t;

protected:

  // phase_result_sptrs_tの中身表示
  void viewPrs(phase_result_sptrs_t results);
  // phase_result_sptr_tの中身表示
  void viewPr(phase_result_sptr_t result);
  // asksの中身表示
  std::string viewAsks(ask_set_t asks);

};//HybridAutomata

}//namespace hydla
}//namespace simulator 

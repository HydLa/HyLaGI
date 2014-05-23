
#include "HybridAutomata.h"
#include "Timer.h"
#include "SymbolicTrajPrinter.h"
#include "PhaseSimulator.h"
#include "Simulator.h"
#include "../common/TimeOutError.h"
#include "../common/Logger.h"
#include <limits.h>
#include <string>
#include <assert.h>

using namespace std;

namespace hydla {
namespace simulator {

HybridAutomata::HybridAutomata(Opts &opts):BatchSimulator(opts){}

HybridAutomata::~HybridAutomata(){}
	
void HybridAutomata::viewPrs(phase_result_sptrs_t results)
{
  phase_result_sptrs_t::iterator it_ls = results.begin();
  while(it_ls != results.end()) {
    viewPr(*it_ls);
    it_ls++;
  }	
}
	
void HybridAutomata::viewPr(phase_result_sptr_t result)
{
  hydla::output::SymbolicTrajPrinter printer(backend);
  printer.output_one_phase(result);
			
  HYDLA_LOGGER_DEBUG("negative ask:");
  HYDLA_LOGGER_DEBUG(viewAsks(result->negative_asks));
  HYDLA_LOGGER_DEBUG("positive ask:");
  HYDLA_LOGGER_DEBUG(viewAsks(result->positive_asks));	
}
	
string HybridAutomata::viewAsks(ask_set_t asks)
{
  ask_set_t::iterator it = asks.begin();
  string str = "";
  while(it != asks.end()){
    str += get_infix_string((*it)->get_guard()) + " ";
    it++;
  }
  return str;
}
	
}//namespace hydla
}//namespace simulator 

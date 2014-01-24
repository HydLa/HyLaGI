
#include "HybridAutomata.h"
#include "Timer.h"
#include "SymbolicTrajPrinter.h"
#include "SymbolicPhaseSimulator.h"
#include "SymbolicValue.h"
#include "PhaseSimulator.h"
#include "../common/TimeOutError.h"
#include "../common/Logger.h"
#include "../parser/TreeInfixPrinter.h"
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
		
		if (logger::Logger::ha_converter_area_) {
	    hydla::output::SymbolicTrajPrinter printer;
			printer.output_one_phase(result);
			
			HYDLA_LOGGER_HA("negative ask:");
			HYDLA_LOGGER_HA(viewAsks(result->negative_asks));
			HYDLA_LOGGER_HA("positive ask:");
			HYDLA_LOGGER_HA(viewAsks(result->positive_asks));
		}else if (logger::Logger::ha_simulator_area_) {
	    hydla::output::SymbolicTrajPrinter printer;
			printer.output_one_phase(result);
			
			HYDLA_LOGGER_HAS("negative ask:");
			HYDLA_LOGGER_HAS(viewAsks(result->negative_asks));
			HYDLA_LOGGER_HAS("positive ask:");
			HYDLA_LOGGER_HAS(viewAsks(result->positive_asks));
		}
	}
	
	string HybridAutomata::viewAsks(ask_set_t asks)
	{
		hydla::parse_tree::TreeInfixPrinter tree_printer;
		ask_set_t::iterator it = asks.begin();
		string str = "";
		while(it != asks.end()){
			str += tree_printer.get_infix_string((*it)->get_guard()) + " ";
			it++;
		}
		return str;
	}
	
}//namespace hydla
}//namespace simulator 



#include "HASimulator.h"
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

	HASimulator::HASimulator(Opts &opts):BatchSimulator(opts){}

	HASimulator::~HASimulator(){}
	
	phase_result_const_sptr_t HASimulator::simulate(){assert(0); return result_root_;}

  phase_result_const_sptr_t HASimulator::simulate(ha_results_t ha_results){assert(0); return result_root_;}
  /*
	{
		HYDLA_LOGGER_HAS("%% simulation start");
		HYDLA_LOGGER_HAS("*** using HA");
		viewPrs(ha_results[0]);

		ha_result_t ha = get_ha(ha_results); //初期値の範囲にあっている記号定数を持つHAを求める。一旦分岐は考えない
		
		time_t max_time;
    
		if(opts_->max_time != ""){
      max_time.reset(new symbolic::SymbolicValue(hydla::parse_tree::node_sptr(new hydla::parse_tree::Number(opts_->max_time))));
    }else{
      max_time.reset(new symbolic::SymbolicValue(hydla::parse_tree::node_sptr(new hydla::parse_tree::Infinity)));
    }
		
		phase_result_sptrs_t simulation_result;

		int i = 0;
		int cnt_phase = 0;
		phase_result_sptr_t pr(new PhaseResult(*ha[i]));
		init_value_map_t vm = get_init_vm(pr);
		time_t current_time = value_t(new hydla::simulator::symbolic::SymbolicValue("0"));
		
		timer::Timer has_timer;
		profile_t profile;
		
		while (true){
	    timer::Timer phase_timer;
			phase_result_sptr_t pr(new PhaseResult(*ha[i]));
			
			HYDLA_LOGGER_HAS("*** now pr : ", i);
			viewPr(pr);
			
			substitute(pr, vm, current_time);

			HYDLA_LOGGER_HAS("*** after substitute pr");
			viewPr(pr);
			
			if(pr->phase == PointPhase){
				vm = update_vm(pr, vm);
				current_time = pr->current_time;
			}
			
			i++;
			cnt_phase++;
			pr->id = cnt_phase;
			simulation_result.push_back(pr);
			
			// profile 
			profile["EntirePhase"] = phase_timer.get_elapsed_us();

			simulation_todo_sptr_t st(new SimulationTodo);
			st->id = pr->id;
			st->phase = pr->phase;
			st->profile = profile;
			profile_vector_->push_back(st);
			
			if(opts_->max_phase > 0 && opts_->max_phase < cnt_phase) {
				HYDLA_LOGGER_HAS("fin : max_phase");					
				break;
			}
			
			if(ha[i]->phase == IntervalPhase){
				if(!ha[i]->end_time.get()){
					// ???だったら最初のノードに戻る(初期のエッジは飛ばす)
					i = 1;
				}else if(ha[i]->end_time.get()->get_string() == "inf"){
					// infだったら終了
					HYDLA_LOGGER_HAS("fin : inf");
					break;
				}else if(max_time < ha[i]->end_time){
					// max_time <= end_timeなら終了
					HYDLA_LOGGER_HAS("fin : max_time");					
				}
			}
			
			//int test;
			//cin >> test;
		}
		
	  HYDLA_LOGGER_HAS("%% simulation end");

		phase_result_sptrs_t::iterator it_ls = simulation_result.begin();
	  hydla::output::SymbolicTrajPrinter printer;
		int phase_num = 1;
		while(it_ls != simulation_result.end()) {
			if((*it_ls)->phase == PointPhase){
			  cout << "#---------" << phase_num++ << "---------\n";
			}
			printer.output_one_phase(*it_ls);
			it_ls++;
		}	
  	has_timer.elapsed("\n\nHASimulator Time");
		return result_root_;
	}
	
  HASimulator::init_value_map_t HASimulator::get_init_vm(phase_result_sptr_t pr){		
		init_value_map_t vm;
		parameter_map_t::iterator it = pr->parameter_map.begin();
	  for(; it != pr->parameter_map.end(); ++it) {
      parameter_t* v;
	  	v = (*it).first;
			string st = "";
	  	cout << "input init value : " << *v << endl;
			cout << ">" ;
			cin >> st;
			vm[v] = value_t(new hydla::simulator::symbolic::SymbolicValue(st));
		}
		
		return vm;
	}
	
	HASimulator::ha_result_t HASimulator::get_ha(ha_results_t ha_results){
		ha_result_t cc = ha_results[0];
		return cc;
	}
	
	void HASimulator::substitute(phase_result_sptr_t pr, init_value_map_t vm, time_t current_time){
		// init_value_map の適用
		phase_simulator_->substitute_values_for_vm(pr, vm);
		// current_time の適用
		phase_simulator_->substitute_current_time_for_vm(pr, current_time);

	}

  HASimulator::init_value_map_t HASimulator::update_vm(phase_result_sptr_t pr, init_value_map_t vm_pre){
		init_value_map_t vm;
		init_value_map_t::const_iterator it_vm  = vm_pre.begin();
	  for(; it_vm != vm_pre.end(); ++it_vm) {
			variable_map_t::iterator it = pr->variable_map.begin();
		  for(; it != pr->variable_map.end(); ++it) {
		  	if(it_vm->first->get_name() == it->first->get_name() && 
		  	   it_vm->first->get_derivative_count() == it->first->get_derivative_count() ){
		  	   	vm[it_vm->first] = value_t(new hydla::simulator::symbolic::SymbolicValue(it->second->get_string()));
		  	}
		  }
	  }
		return vm;
	}
	
	
	void HASimulator::viewPrs(phase_result_sptrs_t results)
	{
		phase_result_sptrs_t::iterator it_ls = results.begin();
		while(it_ls != results.end()) {
			viewPr(*it_ls);	
			it_ls++;
		}	
	}
	
	void HASimulator::viewPr(phase_result_sptr_t result)
	{
		
		if (logger::Logger::ha_simulator_area_) {
	    hydla::output::SymbolicTrajPrinter printer;
			printer.output_one_phase(result);
			
			HYDLA_LOGGER_HAS("negative ask:");
			viewAsks(result->negative_asks);
			HYDLA_LOGGER_HAS("positive ask:");
			viewAsks(result->positive_asks);
		}
	}
	
	void HASimulator::viewAsks(ask_set_t asks)
	{
		hydla::parse_tree::TreeInfixPrinter tree_printer;
		ask_set_t::iterator it = asks.begin();
		string str = "";
		while(it != asks.end()){
			str += tree_printer.get_infix_string((*it)->get_guard()) + " ";
			it++;
		}
		HYDLA_LOGGER_HAS(str);
	}
*/	
}//namespace hydla
}//namespace simulator 


#include "HASimulator.h"
#include "Timer.h"
#include "SymbolicTrajPrinter.h"
#include "SymbolicPhaseSimulator.h"
#include "SymbolicValue.h"
#include "PhaseSimulator.h"
#include "../common/TimeOutError.h"
#include "../common/Logger.h"
#include "../parser/TreeInfixPrinter.h"
#include "Backend.h"
#include <limits.h>
#include <string>
#include <assert.h>

using namespace std;
using namespace hydla::parse_tree;

namespace hydla {
namespace simulator {

HASimulator::HASimulator(Opts &opts):HybridAutomata(opts){}

HASimulator::~HASimulator(){}

void HASimulator::set_ha_results(const ha_results_t& ha)
{
  ha_results = ha;
}
	
phase_result_const_sptr_t HASimulator::simulate()
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

  int i = 0;
  int cnt_phase = 0;
  phase_result_sptr_t pr(new PhaseResult(*ha[i])), parent = result_root_;
  parameter_map_t vm = get_init_vm(pr);
  time_t current_time = value_t(new hydla::simulator::symbolic::SymbolicValue("0"));
		
  timer::Timer has_timer;
  profile_t profile;
  HYDLA_LOGGER_LOCATION(HAS);
		
  while (true){
    timer::Timer phase_timer;
    pr.reset(new PhaseResult(*ha[i]));
    pr->children.clear();
			
    HYDLA_LOGGER_HAS("*** now pr : ", i);
    viewPr(pr);
			
    substitute(pr, vm, current_time);

    HYDLA_LOGGER_HAS("*** after substitute pr");
    viewPr(pr);
			
    if(pr->phase == PointPhase)
    {
      vm = update_vm(pr, vm);
      pr->current_time = current_time;
    }
    else
    {
      pr->current_time = parent->current_time;
      *pr->end_time = *ha[i]->end_time;
      *pr->end_time -= *ha[i]->current_time;
      *pr->end_time += *pr->current_time;      
      pr->end_time = simplify(pr->end_time->get_node());
      current_time = pr->end_time;
      pr->variable_map = phase_simulator_->shift_variable_map_time(pr->variable_map, pr->current_time);     
    }
			
    i++;
    cnt_phase++;
    pr->id = cnt_phase;
    HYDLA_LOGGER_LOCATION(HAS);
    parent->children.push_back(pr);

 
    // profile 
    profile["EntirePhase"] = phase_timer.get_elapsed_us();

    simulation_todo_sptr_t st(new SimulationTodo);
    st->id = pr->id;
    st->phase = pr->phase;
    st->profile = profile;
    profile_vector_->push_back(st);

    if(opts_->max_phase >= 0 && opts_->max_phase < cnt_phase) {
      HYDLA_LOGGER_HAS("fin : max_phase");					
      pr->cause_of_termination = STEP_LIMIT;
      break;
    }
 
    if(ha[i]->phase == IntervalPhase){
      if(!ha[i]->end_time.get()){
        // ???だったら最初のノードに戻る(初期のエッジは飛ばす)
        i = 1;
      }else if(ha[i]->end_time.get()->get_string() == "inf"){
        // infだったら終了
        HYDLA_LOGGER_HAS("fin : inf");
        pr->cause_of_termination = TIME_LIMIT;
        break;
      }else if(max_time < ha[i]->end_time){
/*
        TODO: バックエンドで時刻同士の判定を行う
        // max_time <= end_timeなら終了
        HYDLA_LOGGER_HAS("fin : max_time");
        pr->cause_of_termination = TIME_LIMIT;
        break;
*/
      }
    }
    parent = pr;
  }
		
  HYDLA_LOGGER_HAS("%% simulation end");

  has_timer.elapsed("\n\nHASimulator Time");
  return result_root_;
}
	
parameter_map_t HASimulator::get_init_vm(phase_result_sptr_t pr){		
  parameter_map_t vm;
  parameter_map_t::iterator it = pr->parameter_map.begin();
  for(; it != pr->parameter_map.end(); ++it) {
    parameter_t v = it->first;
    string st = "";
    cout << "input init value : " << v << endl;
    cout << ">" ;
    cin >> st;
    vm[v] = value_t(new hydla::simulator::symbolic::SymbolicValue(st));
  }
		
  return vm;
}

value_t HASimulator::simplify(node_sptr exp)
{
  value_t ret;
  backend_->call("simplify", 1, "en", "vl", &exp, &ret);
  return ret;
}
	
HASimulator::ha_result_t HASimulator::get_ha(ha_results_t ha_results){
  ha_result_t cc = ha_results[0];
  return cc;
}
	
void HASimulator::substitute(phase_result_sptr_t pr, parameter_map_t vm, time_t current_time){
  // parameter_map の適用
  phase_simulator_->substitute_parameter_condition(pr, vm);
}

parameter_map_t HASimulator::update_vm(phase_result_sptr_t pr, parameter_map_t vm_pre){
  parameter_map_t vm;
  parameter_map_t::const_iterator it_vm  = vm_pre.begin();
  for(; it_vm != vm_pre.end(); ++it_vm) {
    variable_map_t::iterator it = pr->variable_map.begin();
    for(; it != pr->variable_map.end(); ++it) {
      if(it_vm->first.get_name() == it->first.get_name() && 
         it_vm->first.get_derivative_count() == it->first.get_derivative_count() ){
        vm[it_vm->first].set_unique(value_t(new hydla::simulator::symbolic::SymbolicValue(it->second.get_string())));
      }
    }
  }
  return vm;
}
	
}//namespace hydla
}//namespace simulator 


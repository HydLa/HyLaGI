#include "HAConverter.h"
#include "Timer.h"
#include "SymbolicTrajPrinter.h"
#include "PhaseSimulator.h"
#include "TimeOutError.h"
#include "Logger.h"
#include "Utility.h"
#include <limits.h>
#include <string>
#include "Backend.h"
#include "ValueModifier.h"

using namespace std;

namespace hydla {
namespace simulator {

HAConverter::HAConverter(boost::shared_ptr<backend::Backend> backend, Opts &opts):HybridAutomata(opts), backend_(backend){}

HAConverter::~HAConverter(){}

phase_result_sptr_t HAConverter::simulate()
{
  std::string error_str;
  simulation_todo_sptr_t init_todo = make_initial_todo();
  int error_sum = 0;
	  
  current_condition_t cc_;
  push_current_condition(cc_);
/* TODO: implement
  while(!todo_stack_->empty()) 
  {
    try
    {
      simulation_todo_sptr_t todo(todo_stack_->pop_todo());
      process_one_todo(todo);
    }
    catch(const std::runtime_error &se)
    {
      error_str += "error ";
      error_str += ('0' + (++error_sum));
      error_str += ": ";
      error_str += se.what();
      error_str += "\n";
      HYDLA_LOGGER_DEBUG(se.what());
    }
  }
*/

  if(!error_str.empty()){
    std::cout << error_str;
  }
	  
  HYDLA_LOGGER_DEBUG("%% simulation end");
	  
  output_ha();

  return result_root_;
}
	
void HAConverter::process_one_todo(simulation_todo_sptr_t& todo)
{
  hydla::io::SymbolicTrajPrinter printer(backend, opts_->output_variables, std::cerr);

  HYDLA_LOGGER_DEBUG("************************\n");

  HYDLA_LOGGER_DEBUG("--- Current Todo ---\n", *todo);
  current_condition_t cc_;
  current_condition_t tmp_cc_;

  try{
    timer::Timer phase_timer;	    
    cc_ = pop_current_condition();
	    
    PhaseSimulator::result_list_t phases = phase_simulator_->calculate_phase_result(todo);
	    
    for(unsigned int i = 0; i < phases.size(); i++)
    {
      phase_result_sptr_t& phase = phases[i];
	    	
      HYDLA_LOGGER_DEBUG("--- Result Phase", i+1 , "/", phases.size(), " ---\n", *phase);
	    	
      tmp_cc_ = cc_;
      tmp_cc_.push_back(phase);
	    	
      if(phase->cause_for_termination == ASSERTION)
      {
        // ASSERTION はそこで変換終了。HAは出力しない（push_resultしない）
        cout << "****** assertion" << endl;
        viewPrs(tmp_cc_);
        continue;
      }
    		
      if (phase->phase_type == IntervalPhase) 
      {
        if (check_already_exec(phase, cc_))
        {
          push_result(tmp_cc_);
          //todoを生成せずに次へ（HA変換完了）
          continue;
        }
      }
      PhaseSimulator::todo_list_t next_todos = phase_simulator_->make_next_todo(phase, todo);
      for(unsigned int j = 0; j < next_todos.size(); j++)
      {
        simulation_todo_sptr_t& n_todo = next_todos[j];
        HYDLA_LOGGER_DEBUG("--- Next Todo", i+1 , "/", phases.size(), ", ", j+1, "/", next_todos.size(), " ---");
        HYDLA_LOGGER_DEBUG(*n_todo);
        // TIME_LIMITの場合
        if(n_todo->parent->cause_for_termination == TIME_LIMIT){
          current_condition_t tmp_tmp_cc_;
          tmp_tmp_cc_ = tmp_cc_;
          tmp_tmp_cc_.pop_back();
          tmp_tmp_cc_.push_back(n_todo->parent);
          HYDLA_LOGGER_DEBUG("*-*-*-*-* termination == TIME_LIMIT");
          push_result(tmp_tmp_cc_);
          continue;
        }
        //todo_stack_->push_todo(n_todo); //TODO: implement
        push_current_condition(tmp_cc_);
      }
    }
	  	
  }
  catch(const hydla::timeout::TimeOutError &te)
  {
    HYDLA_LOGGER_DEBUG(te.what());
    phase_result_sptr_t phase(new PhaseResult(*todo, simulator::TIME_OUT_REACHED));
    todo->parent->children.push_back(phase);
  }
}

bool HAConverter::check_already_exec(phase_result_sptr_t phase, current_condition_t cc)
{
  HYDLA_LOGGER_DEBUG("****** check_already_exec ******");
  phase_result_sptrs_t::iterator it_prs = cc.begin();
  while(it_prs != cc.end()){
    // 同じノードの探索
    if(compare_phase_result(phase, *it_prs)) {
      // 全てのパラメータが実行済みノードのパラメータの部分集合だったらtrue
      if (check_subset(phase, *it_prs)) {
        HYDLA_LOGGER_DEBUG("****** end check_already_exec : true ******");
        return true;
      }
    }
    it_prs++;
  }
  HYDLA_LOGGER_DEBUG("****** end check_already_exec : false ******");
  return false;
}
	
bool HAConverter::check_subset(phase_result_sptr_t phase, phase_result_sptr_t past_phase)
{
  HYDLA_LOGGER_DEBUG("****** check_subset ******");
  viewPr(phase);
  viewPr(past_phase);

  value_t past_time = past_phase->current_time;
  variable_map_t now_vm, past_vm;
  ValueModifier modifier(*backend);
  now_vm = modifier.substitute_time(value_t("0"), phase->variable_map);
  past_vm = modifier.substitute_time(past_time, past_phase->variable_map);

  variable_map_t::const_iterator it_phase_v  = now_vm.begin();
  variable_map_t::const_iterator end_phase_v = now_vm.end();

  for(; it_phase_v!=end_phase_v; ++it_phase_v) {
    range_t tmp_variable_phase, tmp_variable_past;
    variable_map_t::const_iterator it_past_v  = past_vm.begin();
    variable_map_t::const_iterator end_past_v = past_vm.end();
    for(; it_past_v!=end_past_v; ++it_past_v) {
      if ( it_phase_v->first.name == it_past_v->first.name && 
           it_phase_v->first.differential_count == it_past_v->first.differential_count ) {
        tmp_variable_phase = it_phase_v->second;
        HYDLA_LOGGER_DEBUG("Variable: ",it_phase_v->first.name," ",it_phase_v->first.differential_count);
        HYDLA_LOGGER_DEBUG("t         :  0");	
        HYDLA_LOGGER_DEBUG("now       :  ", tmp_variable_phase);
        search_variable_parameter(phase->parameter_map, it_phase_v->first.name, it_phase_v->first.differential_count);
        HYDLA_LOGGER_DEBUG("");	 	
        tmp_variable_past = it_past_v->second;
        HYDLA_LOGGER_DEBUG("t         :  ", past_time);	
        HYDLA_LOGGER_DEBUG("past      :  ", tmp_variable_past);
        search_variable_parameter(past_phase->parameter_map, it_past_v->first.name, it_past_v->first.differential_count);
      }
    }
    // TODO: 幅を持つ場合への対応
    bool isIncludeBound = check_include_bound(tmp_variable_phase.get_unique_value(), tmp_variable_past.get_unique_value(), phase->parameter_map, past_phase->parameter_map);
		  
    if(isIncludeBound){
      HYDLA_LOGGER_DEBUG("****** end check_include_bound : true ******");
    }else{
      HYDLA_LOGGER_DEBUG("****** end check_subset : false ******");
      return false;
    }
  }
  HYDLA_LOGGER_DEBUG("****** end check_subset : true ******");		
  return true;   	
}
	
void HAConverter::search_variable_parameter(parameter_map_t map, std::string name, int diff_cnt)
{
  parameter_map_t::const_iterator it  = map.begin();
  parameter_map_t::const_iterator end = map.end();
  for(; it!=end; ++it) 
  {
    // 途中で導入されたパラメータは見ない
    if ( it->first.get_phase_id() != 1 ) continue;

    HYDLA_LOGGER_DEBUG(it->first, " : " , it->second);		
  }
}

	
bool HAConverter::compare_phase_result(phase_result_sptr_t r1, phase_result_sptr_t r2)
{
  // フェーズ
  if(!(r1->phase_type == r2->phase_type)) return false;
  // モジュール集合
  if(!(r1->module_set.compare(r2->module_set) == 0)) return false;
  // positive_ask
  ask_set_t::iterator it_1 = r1->get_all_positive_asks().begin();
  ask_set_t::iterator it_2 = r2->get_all_positive_asks().begin();
  while(it_1 != r1->get_all_positive_asks().end() && it_2 != r2->get_all_positive_asks().end()) {
    if(!((*it_1)->is_same_struct(**it_2, true))) return false;
    it_1++;
    it_2++;
  }
  // どちらかのイテレータが最後まで達していなかったら等しくない
  if(it_1 != r1->get_all_positive_asks().end() || it_2 != r2->get_all_positive_asks().end()) return false;
		
  return true;
} 

bool HAConverter::check_include_bound(value_t tmp_variable_phase, value_t tmp_variable_past, parameter_map_t pm1, parameter_map_t pm2)
{
  bool ret;
  backend_->call("checkIncludeBound", 4, "vlnvlnmpmp", "b", &tmp_variable_phase, &tmp_variable_past, &pm1, &pm2, &ret);
  return ret;
}

		
void HAConverter::push_result(current_condition_t cc)
{
  phase_result_sptrs_t result;
  for(unsigned int i = 0 ; i < cc.size() ; i++){
    result.push_back(cc[i]);
  }
  HYDLA_LOGGER_DEBUG("・・・・・ ha_result ", ha_results_.size(), " ・・・・・");
  viewPrs(result);
  HYDLA_LOGGER_DEBUG("・・・・・・・・・・・・・・");
  ha_results_.push_back(result);
}
	
void HAConverter::output_ha()
{
  cout << "-・-・-・-・Result Convert・-・-・-・-" << endl;
  ha_results_t::iterator it_ha_res = ha_results_.begin();		
  while(it_ha_res != ha_results_.end()){
    viewPrs((*it_ha_res));
    convert_phase_results_to_ha((*it_ha_res));
    cout << "-・-・-・-・-・-・-・-・-・-・-・-・-" << endl;
    it_ha_res++;
  }
}
	
void HAConverter::convert_phase_results_to_ha(phase_result_sptrs_t result)
{


  // 初期状態のパラメータの出力  id=1のもののみ出力
  parameter_map_t::const_iterator it_pm = result[result.size() - 1]->parameter_map.begin();
  parameter_map_t::const_iterator end_pm = result[result.size() - 1]->parameter_map.end();
  string parameter_str = "";
  for(; it_pm!=end_pm; ++it_pm) {
    if(it_pm->first.get_phase_id() == 1){
      cout << it_pm->first << "\t: " << it_pm->second << "\n";
      parameter_str += it_pm->first.get_name();
      for(int i=0; i < it_pm->first.get_differential_count(); i++) parameter_str +="'";
      parameter_str += "=" + it_pm->second.get_string() + "\\n";
    }
  }
		
  // 状態遷移列の出力
  string stf_str = "";
  for(unsigned int i = 0 ; i < result.size() ; i++){
    // phase_idの出力時に，同じノード，エッジは同じphase_idに合わせる
    if (result[i]->id == subset_id){
      stf_str += "*";
    }
    for(unsigned int j = 1 ; j < i ; j++){
      if(compare_phase_result(result[i],result[j])){
        result[i]->id = result[j]->id;
      }
    }
    if(i == result.size() - 1){
      stf_str += "Phase " + utility::to_string(result[i]->id);
    }else{
      stf_str += "Phase " + utility::to_string(result[i]->id) + " -> ";
    }
  }
  cout << "State Transition Flow" << "\n";
  cout << stf_str << endl;


  vector<string> strs_;
  string str_;
  cout << "digraph g{" << endl;
  cout << "edge [dir=forward];" << endl;
  cout << "labelloc=t;" << endl;
  cout << "label=\"" << stf_str << "\";" << endl;
  cout << "\"start\" [shape=point];" << endl;
  cout << "\"start\"->\"" << "Phase " << result[1]->id << "\\n" << result[1]->module_set.get_name() << "\\n(" << get_asks_str(result[1]->get_all_positive_asks()) 
       << ")\" [label=\"" << "Phase " << result[0]->id << "\\n" << parameter_str << result[0]->module_set.get_name() << "\\n(" << get_asks_str(result[0]->get_all_positive_asks())
       << ")\", labelfloat=false,arrowtail=dot];" << endl;
  for(unsigned int i = 2 ; i < result.size() ; i++){
    if(result[i]->phase_type == IntervalPhase){
      str_ = "\"" + result[i-2]->module_set.get_name() + "\\n(" + get_asks_str(result[i-2]->get_all_positive_asks()) 
        + ")\"->\"" + result[i]->module_set.get_name() + "\\n(" + get_asks_str(result[i]->get_all_positive_asks()) 
        + ")\" [label=\"" + result[i-1]->module_set.get_name() + "\\n(" + get_asks_str(result[i-1]->get_all_positive_asks()) 
        + ")\", labelfloat=false,arrowtail=dot];";
      vector<string>::iterator it_strs = find(strs_.begin(),strs_.end(),str_);
      if(it_strs != strs_.end()) continue;				
      cout << str_ << endl;
      strs_.push_back(str_);
    }
  }
  cout << "}" << endl;
}
	
std::string HAConverter::get_asks_str(ask_set_t asks)
{
  std::string res = "";
  ask_set_t::iterator it = asks.begin();
  while(it != asks.end()){
    res += get_infix_string((*it)->get_guard()) + " ";
    it++;
  }
  return res;
}

}//namespace hydla
}//namespace simulator 


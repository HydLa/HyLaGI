
#include "HAConverter.h"
#include "Timer.h"
#include "SymbolicTrajPrinter.h"
#include "SymbolicValue.h"
#include "PhaseSimulator.h"
#include "../common/TimeOutError.h"
#include "../common/Logger.h"
#include "../parser/TreeInfixPrinter.h"
#include <limits.h>
#include <string>

using namespace std;

namespace hydla {
namespace simulator {

	HAConverter::HAConverter(Opts &opts):BatchSimulator(opts){}

	HAConverter::~HAConverter(){}

	phase_result_const_sptr_t HAConverter::simulate()
	{
	  std::string error_str;
	  simulation_todo_sptr_t init_todo = make_initial_todo();
	  todo_stack_->push_todo(init_todo);
	  int error_sum = 0;
	  
	  current_condition_t cc_;
 		push_current_condition(cc_);

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
	      HYDLA_LOGGER_HA(se.what());
	    }
	  }
	  
	  if(!error_str.empty()){
	    std::cout << error_str;
	  }
	  
	  HYDLA_LOGGER_HA("%% simulation end");
	  
 		output_ha();

	  return result_root_;
	}
	
	void HAConverter::process_one_todo(simulation_todo_sptr_t& todo)
	{
	  hydla::output::SymbolicTrajPrinter printer(opts_->output_variables, std::cerr);

	  HYDLA_LOGGER_HA("************************\n");

	  HYDLA_LOGGER_HA("--- Current Todo ---\n", todo);
    current_condition_t cc_;
    current_condition_t tmp_cc_;

	  try{
	    timer::Timer phase_timer;	    
	    cc_ = pop_current_condition();
	    
	    PhaseSimulator::result_list_t phases = phase_simulator_->calculate_phase_result(todo);
	    
	    for(unsigned int i = 0; i < phases.size(); i++)
	    {
	      phase_result_sptr_t& phase = phases[i];
	    	
	      HYDLA_LOGGER_HA("--- Result Phase", i+1 , "/", phases.size(), " ---\n", phase);
	    	
      	tmp_cc_ = cc_;
      	tmp_cc_.push_back(phase);

	    	if (phase->phase == IntervalPhase) 
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
          HYDLA_LOGGER_HA("--- Next Todo", i+1 , "/", phases.size(), ", ", j+1, "/", next_todos.size(), " ---");
          HYDLA_LOGGER_HA(n_todo);
	      	// TIME_LIMITの場合
	      	if(n_todo->parent->cause_of_termination == TIME_LIMIT){
	      		current_condition_t tmp_tmp_cc_;
	      		tmp_tmp_cc_ = tmp_cc_;
	      		tmp_tmp_cc_.pop_back();
	      		tmp_tmp_cc_.push_back(n_todo->parent);
	      		HYDLA_LOGGER_HA("*-*-*-*-* termination == TIME_LIMIT");
	      		push_result(tmp_tmp_cc_);
	      		continue;
	      	}
	        n_todo->elapsed_time = phase_timer.get_elapsed_us() + todo->elapsed_time;
          todo_stack_->push_todo(n_todo);
	       	push_current_condition(tmp_cc_);
	      }
	    }
	  	
	  }
	  catch(const hydla::timeout::TimeOutError &te)
	  {
	    HYDLA_LOGGER_HA(te.what());
	    phase_result_sptr_t phase(new PhaseResult(*todo, simulator::TIME_OUT_REACHED));
	    todo->parent->children.push_back(phase);
	  }
	}

	bool HAConverter::check_already_exec(phase_result_sptr_t phase, current_condition_t cc)
	{
		HYDLA_LOGGER_HA("****** check_already_exec ******");
		phase_result_sptrs_t::iterator it_prs = cc.begin();
		while(it_prs != cc.end()){
			// 同じノードの探索
			if(compare_phase_result(phase, *it_prs)) {
				// 全てのパラメータが実行済みノードのパラメータの部分集合だったらtrue
				if (check_subset(phase, *it_prs)) {
					HYDLA_LOGGER_HA("****** end check_already_exec : true ******");
					return true;
				}
			}
			it_prs++;
		}
		HYDLA_LOGGER_HA("****** end check_already_exec : false ******");
		return false;
	}
	
	bool HAConverter::check_subset(phase_result_sptr_t phase, phase_result_sptr_t past_phase)
	{
		HYDLA_LOGGER_HA("****** check_subset ******");
		viewPr(phase);
		viewPr(past_phase);
 		
	  variable_map_t::const_iterator it_phase_v  = phase->variable_map.begin();
	  variable_map_t::const_iterator end_phase_v = phase->variable_map.end();
	  for(; it_phase_v!=end_phase_v; ++it_phase_v) {
	  	value_t tmp_variable_phase, tmp_variable_past;
	  	variable_map_t::const_iterator it_past_v  = past_phase->variable_map.begin();
		  variable_map_t::const_iterator end_past_v = past_phase->variable_map.end();
		  for(; it_past_v!=end_past_v; ++it_past_v) {
		 		if ( it_phase_v->first->name == it_past_v->first->name && 
		 				 it_phase_v->first->derivative_count == it_past_v->first->derivative_count ) {
			 		HYDLA_LOGGER_HA("Variable: ",it_phase_v->first->name," ",it_phase_v->first->derivative_count);		 			
			 		HYDLA_LOGGER_HA("t         :  0");		 				 	
			  	tmp_variable_phase = it_phase_v->second;
			  	HYDLA_LOGGER_HA("now       :  ", *tmp_variable_phase);
			  	//HYDLA_LOGGER_HA("c-t         :  ", *phase->current_time);		
		 			search_variable_parameter(phase->parameter_map, it_phase_v->first->name, it_phase_v->first->derivative_count);
					HYDLA_LOGGER_HA("");	 	
			  	tmp_variable_past = it_past_v->second;
			  	HYDLA_LOGGER_HA("past      :  ", *tmp_variable_past);
			  	//HYDLA_LOGGER_HA("c-t         :  ", *past_phase->current_time);		 				 	
			  	search_variable_parameter(past_phase->parameter_map, it_past_v->first->name, it_past_v->first->derivative_count);
		  	}
		  } 

		  bool isIncludeBound = phase_simulator_->check_include_bound(tmp_variable_phase, tmp_variable_past, phase->parameter_map, past_phase->parameter_map);
		  
	  	if(isIncludeBound){
		    HYDLA_LOGGER_HA("****** check_include_bound end : true ******");
		  }else{
		    HYDLA_LOGGER_HA("****** check_include_bound end : false ******");
		  }
	  	if(hydla::logger::Logger::ha_converter_area_){
		  	cout << "please input 0 or 1: if past includes now, input 1, otherwise 0. " << endl;
			  cout << ">";
			  cin >> isIncludeBound;
			  if(isIncludeBound == 0) {
					HYDLA_LOGGER_HA("****** end check_subset : false ******");		
				  return false;
			  }
	  	}else{
	  		return isIncludeBound;
	  	}
	  }
	  
		HYDLA_LOGGER_HA("****** end check_subset : true ******");		
   	return true;
   	
	}
	
	void HAConverter::search_variable_parameter(parameter_map_t map, std::string name, int diff_cnt)
	{
	  parameter_map_t::const_iterator it  = map.begin();
	  parameter_map_t::const_iterator end = map.end();
	  for(; it!=end; ++it) 
	  {
		  // 途中で導入されたパラメータは見ない
	  	if ( (*(it->first)).get_phase_id() != 1 ) continue;

			HYDLA_LOGGER_HA((*it->first), " : " , it->second);		
	  }
	}
	
	bool HAConverter::compare_phase_result(phase_result_sptr_t r1, phase_result_sptr_t r2)
	{
		// フェーズ
		if(!(r1->phase == r2->phase)) return false;
		// モジュール集合
		HYDLA_LOGGER_HA("compare :: id:", r1->id, " ", r1->module_set->get_name(), " <=> id:", r2->id, " ", r2->module_set->get_name());
		if(!(r1->module_set->compare(*r2->module_set) == 0)) return false;
		// positive_ask
		ask_set_t::iterator it_1 = r1->positive_asks.begin();
		ask_set_t::iterator it_2 = r2->positive_asks.begin();
		while(it_1 != r1->positive_asks.end() && it_2 != r2->positive_asks.end()) {
			if(!((*it_1)->is_same_struct(**it_2, true))) return false;
			it_1++;
			it_2++;
		}
		// どちらかのイテレータが最後まで達していなかったら等しくない
		if(it_1 != r1->positive_asks.end() || it_2 != r2->positive_asks.end()) return false;
		
		return true;
	} 
	
	void HAConverter::viewPrs(phase_result_sptrs_t results)
	{
		phase_result_sptrs_t::iterator it_ls = results.begin();
		while(it_ls != results.end()) {
			viewPr(*it_ls);	
			it_ls++;
		}	
	}
	
	void HAConverter::viewPr(phase_result_sptr_t result)
	{
		if (logger::Logger::ha_converter_area_) {
	    hydla::output::SymbolicTrajPrinter printer;
			printer.output_one_phase(result);
			
			HYDLA_LOGGER_HA("negative ask:");
			viewAsks(result->negative_asks);
			HYDLA_LOGGER_HA("positive ask:");
			viewAsks(result->positive_asks);
		}
	}
	
	void HAConverter::viewAsks(ask_set_t asks)
	{
		hydla::parse_tree::TreeInfixPrinter tree_printer;
		ask_set_t::iterator it = asks.begin();
		string str = "";
		while(it != asks.end()){
			str += tree_printer.get_infix_string((*it)->get_guard()) + " ";
			it++;
		}
		HYDLA_LOGGER_HA(str);
	}
	
	void HAConverter::push_result(current_condition_t cc)
	{
		phase_result_sptrs_t result;
		for(unsigned int i = 0 ; i < cc.size() ; i++){
			result.push_back(cc[i]);
		}
		HYDLA_LOGGER_HA("・・・・・ ha_result ", ha_results_.size(), " ・・・・・");
		viewPrs(result);
		HYDLA_LOGGER_HA("・・・・・・・・・・・・・・");
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
		vector<string> strs_;
		string str_;
		cout << "digraph g{" << endl;
		cout << "edge [dir=forward];" << endl;
		cout << "\"start\" [shape=point];" << endl;
			cout << "\"start\"->\"" << result[1]->module_set->get_name() << "\\n(" << get_asks_str(result[1]->positive_asks) 
			<< ")\" [label=\"" << result[0]->module_set->get_name() << "\\n(" << get_asks_str(result[0]->positive_asks)
			   << ")\", labelfloat=false,arrowtail=dot];" << endl;
		for(unsigned int i = 2 ; i < result.size() ; i++){
			if(result[i]->phase == IntervalPhase){
				str_ = "\"" + result[i-2]->module_set->get_name() + "\\n(" + get_asks_str(result[i-2]->positive_asks) 
					+ ")\"->\"" + result[i]->module_set->get_name() + "\\n(" + get_asks_str(result[i]->positive_asks) 
						+ ")\" [label=\"" + result[i-1]->module_set->get_name() + "\\n(" + get_asks_str(result[i-1]->positive_asks) 
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
		hydla::parse_tree::TreeInfixPrinter tree_printer;
		ask_set_t::iterator it = asks.begin();
		while(it != asks.end()){
			res += tree_printer.get_infix_string((*it)->get_guard()) + " ";
			it++;
		}
		return res;
	}
	
}//namespace hydla
}//namespace simulator 


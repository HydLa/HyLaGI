
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
	  
	  HYDLA_LOGGER_HA("%% simulation ended");
	  
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
       	push_current_condition(tmp_cc_);
	    	
	      PhaseSimulator::todo_list_t next_todos = phase_simulator_->make_next_todo(phase, todo);
	      for(unsigned int j = 0; j < next_todos.size(); j++)
	      {
	        simulation_todo_sptr_t& n_todo = next_todos[j];
	        n_todo->elapsed_time = phase_timer.get_elapsed_us() + todo->elapsed_time;
          todo_stack_->push_todo(n_todo);
          HYDLA_LOGGER_HA("--- Next Todo", i+1 , "/", phases.size(), ", ", j+1, "/", next_todos.size(), " ---");
          HYDLA_LOGGER_HA(n_todo);
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
		  bool isIncludeBound;
		  cout << "please input 0 or 1: if past includes now, input 1, otherwise 0. " << endl;
		  cout << ">";
		  cin >> isIncludeBound;
		  if(isIncludeBound == 0) {
				HYDLA_LOGGER_HA("****** end check_subset : false ******");		
			  return false;
		  }
	  }
	  
		HYDLA_LOGGER_HA("****** end check_subset : true ******");		
   	return true;
   	
		/*
	  parameter_map_t::const_iterator it_phase  = phase->parameter_map.begin();
	  parameter_map_t::const_iterator end_phase = phase->parameter_map.end();

	  parameter_map_t::const_iterator it_past  = past_phase->parameter_map.begin();
	  parameter_map_t::const_iterator end_past = past_phase->parameter_map.end();
	
	  for(; it_phase!=end_phase; ++it_phase) {
	  	// 途中で導入されたパラメータは見ない
	  	if ( (*(it_phase->first)).get_phase_id() != 1 ) continue;

		  for(; it_past!=end_past; ++it_past) {
		  	// 途中で導入されたパラメータは見ない
		  	if ( (*(it_past->first)).get_phase_id() != 1 ) continue;

  	    if ( (*(it_phase->first)).get_name() == (*(it_past->first)).get_name() && 
  	    		 (*(it_phase->first)).get_derivative_count() == (*(it_past->first)).get_derivative_count() )
  	    {
  	    	// 対象変数がガード条件に含まれない場合は次へ
  	    	if (!check_guard_variable(phase, (*(it_phase->first)).get_name(), (*(it_phase->first)).get_derivative_count())) continue;
	  	    
	  	    HYDLA_LOGGER_HA(*(it_phase->first));
	  	    HYDLA_LOGGER_HA("now:", it_phase->second);
	  	    HYDLA_LOGGER_HA("past:", it_past->second);
	  	    
	  	    if (compare_parameter_range(it_phase->second, it_past->second)) {
						HYDLA_LOGGER_HA("****** end check_subset : true ******");		  	
	  	    	return true;
	  	    }
	  	  }
		  }//for	  
	  }//for
		
		HYDLA_LOGGER_HA("****** end check_subset : false ******");		
		return false;		
		*/
	}
	
	bool HAConverter::check_guard_variable(phase_result_sptr_t phase, std::string name, int derivative_count)
	{
 		GuardGetter gg;
		phase->module_set->dispatch(&gg);
		
		VaribleGetter vg;
		ask_set_t::iterator it = gg.asks.begin();
		
		// ガード条件に含まれる変数を集める
		while(it != gg.asks.end()){
			vg.tmp_diff_cnt = 0;
			(*it)->get_guard()->accept((*it)->get_guard(), &vg);		
			it++;
		}
		
		vg.it_begin();
   	while(vg.get_iterator() != vg.vec_variable.end()){
   		if ( (*vg.get_iterator()).name == name && (*vg.get_iterator()).diff_cnt == derivative_count ) {
				HYDLA_LOGGER_HA("----- check_guard_variable : true ");		
   			return true;
   		}
   		vg.inc_it();
   	}	
		HYDLA_LOGGER_HA("----- check_guard_variable : false ");		
   	return false;
	}
	
	void HAConverter::search_variable_parameter(parameter_map_t map, std::string name, int diff_cnt)
	{
	  parameter_map_t::const_iterator it  = map.begin();
	  parameter_map_t::const_iterator end = map.end();
	  for(; it!=end; ++it) 
	  {
		  // 途中で導入されたパラメータは見ない
	  	if ( (*(it->first)).get_phase_id() != 1 ) continue;

 	    if ( (*(it->first)).get_name() == name && (*(it->first)).get_derivative_count() == diff_cnt ){
			  HYDLA_LOGGER_HA("parameter :  ", it->second);		
			  break;	  	
 	    }
	  }
	}
	
	// A in B => true
	bool HAConverter::compare_parameter_range(range_t A, range_t B)
	{
		if(B.get_upper_cnt() > 0){
			// Bup != inf && Aup = inf
			if (A.get_upper_cnt() == 0) {
				HYDLA_LOGGER_HA("****** compare_parameter_range : false 1 ******");		
				return false;
			}
			
			HYDLA_LOGGER_HA("Bup:",B.get_upper_bound().value->get_string());
			HYDLA_LOGGER_HA("Aup:",A.get_upper_bound().value->get_string());
			
			// Bup < Aup
			if (B.get_upper_bound().value->get_string() < A.get_upper_bound().value->get_string() ||
					// A=B & Aが閉区間でBが開区間 => false
					( B.get_upper_bound().value->get_string() == A.get_upper_bound().value->get_string() && 
					  !B.get_upper_bound().include_bound && A.get_upper_bound().include_bound ) ) {
				HYDLA_LOGGER_HA("****** compare_parameter_range : false 2 ******");		
				return false;
			}
		}
		
		if(B.get_lower_cnt() > 0){
			// Blow != -inf && Alow = -inf
			if (A.get_lower_cnt() == 0) {
				HYDLA_LOGGER_HA("****** compare_parameter_range : false 3 ******");		
				return false;
			}
			HYDLA_LOGGER_HA("Alow:",A.get_lower_bound().value->get_string());
			HYDLA_LOGGER_HA("Blow:",B.get_lower_bound().value->get_string());
			// Alow < Blow
			if (A.get_lower_bound().value->get_string() < B.get_lower_bound().value->get_string() ||
					// A=B & Aが閉区間でBが開区間 => false
					( B.get_lower_bound().value->get_string() == A.get_lower_bound().value->get_string() && 
					  !B.get_lower_bound().include_bound && A.get_lower_bound().include_bound ) ) {
				HYDLA_LOGGER_HA("****** compare_parameter_range : false 4 ******");		
				return false;
			}
		}
		
		HYDLA_LOGGER_HA("****** compare_parameter_range : true ******");		
		return true;
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
	
	bool HAConverter::compare_phase(phase_t p1, phase_t p2)
	{
		// module set
		if(!(p1.module_set->compare(*p2.module_set) == 0)) return false;
		// positive_ask
		ask_set_t::iterator it_1 = p1.positive_asks.begin();
		ask_set_t::iterator it_2 = p2.positive_asks.begin();
		while(it_1 != p1.positive_asks.end() && it_2 != p2.positive_asks.end()) {
			if(!((**it_1).is_same_struct(**it_2, true))) return false;
			it_1++;
			it_2++;
		}
		// どちらかのイテレータが最後まで達していなかったら等しくない
		if(it_1 != p1.positive_asks.end() || it_2 != p2.positive_asks.end()) return false;
		
		return true;
	}
  
	bool HAConverter::check_contain(phase_result_sptr_t result, current_condition_t cc)
	{
		HYDLA_LOGGER_HA("****** check_contain ******");
		HYDLA_LOGGER_HA("・・・・・phase_result・・・・・");
		viewPrs(cc);
		HYDLA_LOGGER_HA("・・・・・・・・・・・・・・・・");
		phase_result_sptrs_t::iterator it_prs = cc.begin();
		while(it_prs != cc.end()){
			if(compare_phase_result(result, *it_prs)) {
				HYDLA_LOGGER_HA("****** end check_contain : true ******");				
				return true;
			}
			it_prs++;
		}
		HYDLA_LOGGER_HA("****** end check_contain : false ******");				
		return false;
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
			convert_phase_results_to_ha((*it_ha_res));
			cout << "-・-・-・-・-・-・-・-・-・-・-・-・-" << endl;
			it_ha_res++;
		}
	}
	
	void HAConverter::convert_phase_results_to_ha(phase_result_sptrs_t result)
	{
		cout << "digraph g{" << endl;
		cout << "edge [dir=forward];" << endl;
		cout << "\"start\" [shape=point];" << endl;
			cout << "\"start\"->\"" << result[1]->module_set->get_name() << "\\n(" << get_asks_str(result[1]->positive_asks) 
			<< ")\" [label=\"" << result[0]->module_set->get_name() << "\\n(" << get_asks_str(result[0]->positive_asks)
			   << ")\", labelfloat=false,arrowtail=dot];" << endl;
		for(unsigned int i = 2 ; i < result.size() ; i++){
			if(result[i]->phase == IntervalPhase){
				cout << "\"" << result[i-2]->module_set->get_name() << "\\n(" << get_asks_str(result[i-2]->positive_asks) 
					<< ")\"->\"" << result[i]->module_set->get_name() << "\\n(" << get_asks_str(result[i]->positive_asks) 
						<< ")\" [label=\"" << result[i-1]->module_set->get_name() << "\\n(" << get_asks_str(result[i-1]->positive_asks) 
				     << ")\", labelfloat=false,arrowtail=dot];" << endl;
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
		
	GuardGetter::GuardGetter(){}
	GuardGetter::~GuardGetter(){}
	
	void GuardGetter::accept(const boost::shared_ptr<parse_tree::Node>& n)
	{
		n->accept(n, this);
	}
	
	void GuardGetter::visit(boost::shared_ptr<parse_tree::Ask> node)
	{
		asks.insert(node);
	}

	VaribleGetter::VaribleGetter(){}
	VaribleGetter::~VaribleGetter(){}
	
	void VaribleGetter::accept(const boost::shared_ptr<parse_tree::Node>& n)
	{
		n->accept(n, this);
	}
	
	void VaribleGetter::visit(boost::shared_ptr<parse_tree::Differential> node)
	{
			tmp_diff_cnt++;
			cout << "DIFF:" << tmp_diff_cnt << endl;
		  node->get_child()->accept(node->get_child(),this);
		  tmp_diff_cnt--;
	}
	void VaribleGetter::visit(boost::shared_ptr<parse_tree::Variable> node)
	{
		guard_variable_t gv;
		gv.diff_cnt = tmp_diff_cnt;
		tmp_diff_cnt = 0;
		gv.name = node->get_name();
		vec_variable.push_back(gv);
	}
	
}//namespace hydla
}//namespace simulator 


#include "HAConverter.h"
#include "Timer.h"
#include "SymbolicTrajPrinter.h"
#include "SymbolicValue.h"
#include "PhaseSimulator.h"
#include "../common/TimeOutError.h"
#include "../common/Logger.h"
#include "../parser/TreeInfixPrinter.h"

using namespace std;

namespace hydla {
namespace simulator {

  HAConverter::HAConverter(Opts &opts):Simulator(opts){}

  HAConverter::~HAConverter(){}

  HAConverter::phase_result_const_sptr_t HAConverter::simulate()
  {
      hydla::output::SymbolicTrajPrinter printer;
      std::string error_str;
  	  current_condition_t cc_;
  		current_condition_t tmp_cc_;
  		cc_.is_loop_step = false;
  		cc_.loop_start_id = -1;
  		cc_.loop_count = 0;
  		push_current_condition(cc_);
  	HYDLA_LOGGER_HA("mlc:", opts_->max_loop_count);
  		while(!state_stack_.empty()) {
        simulation_phase_sptr_t state(pop_simulation_phase());
      	// current conditionはtodoと同様にstuckに積まれるため、同じようにpopすれば、現phaseのcc_が取得できる
  			if(cc_vec_.empty()) break;
  			// シミュレーション結果が複数ある場合、これをもとにconditionを変更する
      	tmp_cc_ = pop_current_condition();
  			
  			HYDLA_LOGGER_HA("%% Current Condition");
  			HYDLA_LOGGER_HA("%% phase_results.size():", tmp_cc_.phase_results.size());
  			HYDLA_LOGGER_HA("%% ls.size():", tmp_cc_.ls.size());
  			HYDLA_LOGGER_HA("%% loop.size():", tmp_cc_.loop.size());
  			HYDLA_LOGGER_HA("%% is_loop_step:", tmp_cc_.is_loop_step);
  			HYDLA_LOGGER_HA("%% loop_count:", tmp_cc_.loop_count);
  			HYDLA_LOGGER_HA("%% loop_start_id:", tmp_cc_.loop_start_id);
  			HYDLA_LOGGER_HA("");
  			
        bool consistent;
        
        {
          phase_result_sptr_t& pr = state->phase_result;
          if(opts_->max_phase >= 0 && pr->step >= opts_->max_phase){
            pr->parent->cause_of_termination = simulator::STEP_LIMIT;
            continue;
          }
        }
        
        try{
          state->module_set_container->reset(state->visited_module_sets);
		      timer::Timer phase_timer;
		      PhaseSimulator::todo_and_results_t phases = phase_simulator_->simulate_phase(state, consistent);

        	HYDLA_LOGGER_HA("%% Phases size: ", phases.size());
        	HYDLA_LOGGER_HA("%% Result PHASE: ");
        	HYDLA_LOGGER_HA("parent_id : ", state->phase_result->parent->id);
          for(unsigned int i=0;i<phases.size();i++){
		        if(phases[i].result.get() != NULL){
	            phase_result_sptr_t& pr = phases[i].result;
		        	HYDLA_LOGGER_HA("--- Phase", i, " ---");
		        	HYDLA_LOGGER_HA("%% PhaseType: ", pr->phase);
		        	HYDLA_LOGGER_HA("%% id: ", pr->id);
		        	if(logger::Logger::ha_converter_area_) printer.output_one_phase(pr);
		        }
          }
        	HYDLA_LOGGER_HA("");
        	
	       	HYDLA_LOGGER_HA("%% Result TODO:");
          for(unsigned int i=0; i<phases.size();i++){
		        if(phases[i].todo.get() != NULL){
		          phase_result_sptr_t& pr = phases[i].todo->phase_result;
		          HYDLA_LOGGER_HA("--- Phase", i, " ---");
		          HYDLA_LOGGER_HA("%% PhaseType: ", pr->phase);
		          HYDLA_LOGGER_HA("%% id: ", pr->id);
		          HYDLA_LOGGER_HA("%% step: ", pr->step);
		          HYDLA_LOGGER_HA("%% time: ", *pr->current_time);
		          HYDLA_LOGGER_HA("--- parameter map ---\n", pr->parameter_map);
		        }
          }
        	
        	// ******* start HA変換処理 ******* 
          for(unsigned int i=0; i<phases.size();i++){
          	cc_ = tmp_cc_;
	        	phase_result_sptr_t result_ = phases[i].result;
          	// resultがなければ、current_conditionをそのままpushして次へ
          	if(result_ == NULL) {
	           	push_current_condition(cc_);
	         		continue;
	          }
	        	switch (state->phase_result->phase)
	        	{
	          	case PointPhase:
	          	{
	          		HYDLA_LOGGER_HA("～・～・～ PP ～・～・～");
	          		if(cc_.is_loop_step){
		         			cc_.phase_results.push_back(result_);
	          			cc_.loop.push_back(result_);
	          			HYDLA_LOGGER_HA("******* loop *******");
	          			viewPrs(cc_.loop);
	          			HYDLA_LOGGER_HA("* * * * * * * * * *");
	          			if (!check_continue(&cc_)){
	          				// ループ判定ステップを抜ける
	          				HYDLA_LOGGER_HA("-------- end loop step ----------");
							  		cc_.is_loop_step = false;
							  		cc_.loop_start_id = -1;
							  		cc_.loop_count = 0;
	          			}
		         			break;
	          		}else{
		         			cc_.phase_results.push_back(result_);
		         			break;
	          		}
	          	}
	          	case IntervalPhase:
	          	{
	          		HYDLA_LOGGER_HA("～・～・～ IP ～・～・～");
	          		if(cc_.is_loop_step){
		         			cc_.phase_results.push_back(result_);
	          			cc_.loop.push_back(result_);
	          			HYDLA_LOGGER_HA("******* loop *******");
	          			viewPrs(cc_.loop);
	          			HYDLA_LOGGER_HA("* * * * * * * * * *");
	          			if (!check_continue(&cc_)) {
	          				// ループ判定ステップを抜ける
	          				HYDLA_LOGGER_HA("-------- end loop step ----------");
							  		cc_.is_loop_step = false;
							  		cc_.loop_start_id = -1;
							  		cc_.loop_count = 0;
	          				break;
	          			}
	          			if (loop_eq_max_ls(cc_)){
	          				cc_.loop_count++;
	          				cc_.loop.clear();
	          				cc_.loop.push_back(result_);
	          				if(cc_.loop_count >= opts_->max_loop_count){
		          				// エッジ判定ステップへ
		          				check_edge_step();
		          				// ここではls_の要素は１つになっている（loopと等しい最大のもののみ残っている）
		          				push_result(cc_);
		          				continue;  
	          				}
	          				HYDLA_LOGGER_HA("loop_count < max_loop_count");
	          			}
		         			break;	          		
	          		}else{
		          		if(check_contain(result_, cc_)){
		          			HYDLA_LOGGER_HA("-------- change loop step ----------");
		          			cc_.is_loop_step = true;
	          				cc_.loop.clear();
	          				cc_.ls.clear();
		          			cc_.loop_count = 0;
			         			cc_.phase_results.push_back(result_);
		          			set_possible_loops(result_, &cc_);
		          			cc_.loop.push_back(result_);
		          			cc_.loop_start_id = result_->id;
		          			break;
		          		}
		         			cc_.phase_results.push_back(result_);
		         			break;	
	          		}
	          	}
	        	}
	        	
	        	if(result_->cause_of_termination == simulator::TIME_LIMIT) {
	        		// シミュレーション終了時刻は∞を想定しており、TIME_LIMITである場合、現在のphase_resultsがそのままha_results_に入る
	        		HYDLA_LOGGER_HA("TIME_LIMIT : Infinity");
	        		push_result(cc_);
	        		continue;
	        	}
          	
          	push_current_condition(cc_);
          }
        	// ******* end HA変換処理 ******* 
        	
		      if((opts_->max_phase_expanded <= 0 || phase_id_ < opts_->max_phase_expanded) && !phases.empty()){
		        for(unsigned int i = 0;i < phases.size();i++){
		          PhaseSimulator::TodoAndResult& tr = phases[i];
		          if(tr.todo.get() != NULL){
		            if(tr.todo->phase_result->parent != result_root_){
		              tr.todo->module_set_container = msc_no_init_;
		            }
		            else{
		              // TODO これだと，最初のPPで分岐が起きた時のモジュール集合がおかしくなるはず
		              tr.todo->module_set_container = msc_original_;
		            }
		            tr.todo->elapsed_time = phase_timer.get_elapsed_us() + state->elapsed_time;
		            push_simulation_phase(tr.todo);
		          }if(tr.result.get() != NULL){
		            state->phase_result->parent->children.push_back(tr.result);
		          }
		          if(!opts_->nd_mode) break;
		        }
		      }
        	HYDLA_LOGGER_HA("");        	
        	HYDLA_LOGGER_HA("***************************");
        	HYDLA_LOGGER_HA("");        	
        }//try
        catch(const hydla::timeout::TimeOutError &te)
        {
		      // タイムアウト発生
		      phase_result_sptr_t& pr = state->phase_result;
		      HYDLA_LOGGER_PHASE(te.what());
		      pr->cause_of_termination = TIME_OUT_REACHED;
		      pr->parent->children.push_back(pr);
        }
      	catch(const std::runtime_error &se)
        {
          error_str = se.what();
          HYDLA_LOGGER_PHASE(se.what());
        }
      }//while(!state_stack_.empty())
      if(!error_str.empty()){
        std::cout << error_str;
      }

  		output_ha();

      return result_root_;
  }
	
	bool HAConverter::compare_phase_result(phase_result_sptr_t r1, phase_result_sptr_t r2){
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
	
	bool HAConverter::check_continue(current_condition_t *cc)
	{
		HYDLA_LOGGER_HA("****** check_continue ******");
		viewLs(*cc);
		std::vector<phase_result_sptrs_t>::iterator it_ls_vec = cc->ls.begin();		
		std::vector< std::vector<phase_result_sptrs_t>::iterator > remove_itr;
		while(it_ls_vec != cc->ls.end()){
			phase_result_sptrs_t::iterator it_ls = (*it_ls_vec).begin();
			phase_result_sptrs_t::iterator it_loop = cc->loop.begin();
			while(it_ls != (*it_ls_vec).end() && it_loop != cc->loop.end()) {
				// 等しくない状態遷移列だったらlsから削除
				if(!compare_phase_result(*it_ls, *it_loop)){
					HYDLA_LOGGER_HA("delete ls element : not equal loop");
					// whileでイテレート中にls_の中身をeraseできないので、eraseするitrを保存して後でls_の要素を削除する
					remove_itr.push_back(it_ls_vec);
					break;
				}
				it_ls++;
				it_loop++;
				// loopより短い状態遷移列はlsから削除
				if(it_ls == (*it_ls_vec).end() && it_loop != cc->loop.end()){
					HYDLA_LOGGER_HA("delete ls element : less than loop");
					// whileでイテレート中にls_の中身をeraseできないので、eraseするitrを保存して後でls_の要素を削除する
					remove_itr.push_back(it_ls_vec);
				}
			}	
			it_ls_vec++;
			HYDLA_LOGGER_HA("* * * * * * * * * * * * *");
		}
		
		for (unsigned int i = 0 ; i < remove_itr.size() ; i++){
			cc->ls.erase(remove_itr[i]);
		}
		
		viewLs(*cc);
		
		if(cc->ls.empty()) {
			HYDLA_LOGGER_HA("****** end check_continue : false ******");
			return false;
		}
		HYDLA_LOGGER_HA("****** end check_continue : true ******");

		return true;
	}
  	
	void HAConverter::set_possible_loops(phase_result_sptr_t result, current_condition_t *cc)
	{
		HYDLA_LOGGER_HA("****** set_possible_loops ******");
		phase_result_sptrs_t::iterator it_prs = cc->phase_results.begin();
		while(it_prs != cc->phase_results.end()){
			if(compare_phase_result(result, *it_prs)){
				phase_result_sptrs_t candidate_loop;
				copy(it_prs, cc->phase_results.end(), back_inserter(candidate_loop));
				cc->ls.push_back(candidate_loop);
			}
			it_prs++;
		}
		
		// lsの中身を表示
		viewLs(*cc);
		HYDLA_LOGGER_HA("****** end set_possible_loops ******");
	}
  	
	bool HAConverter::loop_eq_max_ls(current_condition_t cc)
	{
		HYDLA_LOGGER_HA("****** loop_eq_max_ls ******");
		// とりあえずlsの最初の要素が最大のものと仮定←たぶんこれで正しい
		// loopより短い状態遷移列と、loopと等しくない状態遷移列はcheck_continueでls_から削除しているため
		phase_result_sptrs_t::iterator it_ls = cc.ls[0].begin();
		phase_result_sptrs_t::iterator it_loop = cc.loop.begin();
		while(it_ls != cc.ls[0].end() && it_loop != cc.loop.end()) {
			if(!compare_phase_result(*it_ls, *it_loop)) {
				HYDLA_LOGGER_HA("****** end loop_eq_max_ls : false ******");
				return false;
			}
			it_ls++;
			it_loop++;
		}
		// どちらかのイテレータが最後まで達していなかったら等しくない
		if(it_ls != cc.ls[0].end() || it_loop != cc.loop.end()) {
			HYDLA_LOGGER_HA("****** end loop_eq_max_ls : false ******");
			return false;
		}
		HYDLA_LOGGER_HA("****** end loop_eq_max_ls : true ******");
		return true;
	}
  	
	void HAConverter::check_edge_step()
	{
		//TODO エッジ判定ステップの実装
		HYDLA_LOGGER_HA("****** check_edge_step ******");
		HYDLA_LOGGER_HA("****** end check_edge_step ******");
	}
  	
	bool HAConverter::check_contain(phase_result_sptr_t result, current_condition_t cc)
	{
		HYDLA_LOGGER_HA("****** check_contain ******");
		HYDLA_LOGGER_HA("・・・・・phase_result・・・・・");
		viewPrs(cc.phase_results);
		HYDLA_LOGGER_HA("・・・・・・・・・・・・・・・・");
		phase_result_sptrs_t::iterator it_prs = cc.phase_results.begin();
		while(it_prs != cc.phase_results.end()){
			if(compare_phase_result(result, *it_prs)) {
				HYDLA_LOGGER_HA("****** end check_contain : true ******");				
				return true;
			}
			it_prs++;
		}
		HYDLA_LOGGER_HA("****** end check_contain : false ******");				
		return false;
	}
	
	void HAConverter::viewLs(current_condition_t cc)
	{
		HYDLA_LOGGER_HA("・・・・・ ls ・・・・・");
		std::vector<phase_result_sptrs_t>::iterator it_ls_vec = cc.ls.begin();		
		while(it_ls_vec != cc.ls.end()){
			viewPrs(*it_ls_vec);
			HYDLA_LOGGER_HA("・・・・・・・・・・・・");
			it_ls_vec++;
		}
	}

	void HAConverter::viewPrs(phase_result_sptrs_t results)
	{
    hydla::output::SymbolicTrajPrinter printer;
		phase_result_sptrs_t::iterator it_ls = results.begin();
		while(it_ls != results.end()) {
			if(logger::Logger::ha_converter_area_) printer.output_one_phase(*it_ls);
			it_ls++;
		}	
	}
	
	void HAConverter::push_result(current_condition_t cc)
	{
		phase_result_sptrs_t result;
		for(unsigned int i = 0 ; i < cc.phase_results.size() ; i++){
			result.push_back(cc.phase_results[i]);
			if(cc.phase_results[i]->id == cc.loop_start_id) break;
		}
		HYDLA_LOGGER_HA("・・・・・ ha_result ", ha_results_.size(), " ・・・・・");
		viewPrs(result);
		HYDLA_LOGGER_HA("・・・・・・・・・・・・・・");
		ha_results_.push_back(result);
	}
	
	void HAConverter::output_ha()
	{
		cout << "-・-・-・-・Result Convert・-・-・-・-" << endl;
		std::vector<phase_result_sptrs_t>::iterator it_ha_res = ha_results_.begin();		
		while(it_ha_res != ha_results_.end()){
			convert_phase_results_to_ha(*it_ha_res);
			cout << "-・-・-・-・-・-・-・-・-・-・-・-・-" << endl;
			it_ha_res++;
		}
	}
	
	void HAConverter::convert_phase_results_to_ha(phase_result_sptrs_t result)
	{
		std::vector<std::string> guards;
		std::string guard = "";
		hydla::parse_tree::TreeInfixPrinter tree_printer;
		// TODO: ガード条件が成立している制約名を出力したい
		for(unsigned int i = 0 ; i < result.size() ; i++){
			std::set<boost::shared_ptr<hydla::parse_tree::Ask> >::iterator it = result[i]->positive_asks.begin();
			while(it != result[i]->positive_asks.end()){
				guard += tree_printer.get_infix_string((*it)->get_guard()) + " ";
				it++;
			}
			guards.push_back(guard);
			guard = "";
		}
		cout << "digraph g{" << endl;
		cout << "edge [dir=forward];" << endl;
		cout << "\"start\" [shape=point];" << endl;
		cout << "\"start\"->\"" << result[1]->module_set->get_name() << "\\n" << guards[1] 
			   << "\" [label=\"" << result[0]->module_set->get_name() << "\\n" << guards[0] 
			   << "\", labelfloat=false,arrowtail=dot];" << endl;
		for(unsigned int i = 2 ; i < result.size() ; i++){
			if(result[i]->phase == IntervalPhase){
				cout << "\"" << result[i-2]->module_set->get_name() << "\\n" << guards[i-2] 
						 << "\"->\"" << result[i]->module_set->get_name() << "\\n" << guards[i] 
				     << "\" [label=\"" << result[i-1]->module_set->get_name() << "\\n" << guards[i-1] 
				     << "\", labelfloat=false,arrowtail=dot];" << endl;
			}
		}
		cout << "}" << endl;
	}
  
		void HAConverter::initialize(const parse_tree_sptr& parse_tree)
  {
    Simulator::initialize(parse_tree);
    //初期状態を作ってスタックに入れる
    simulation_phase_sptr_t state(new simulation_phase_t());
    phase_result_sptr_t &pr = state->phase_result;
    pr.reset(new phase_result_t());
    pr->cause_of_termination = NONE;
    
    pr->phase        = simulator::PointPhase;
    pr->step         = 0;
    pr->current_time = value_t(new hydla::symbolic_simulator::SymbolicValue("0"));
    state->module_set_container = msc_original_;
    pr->parent = result_root_;
    push_simulation_phase(state);
  	
  	// HA変換のための初期化
  	
  }

} // simulator
} // hydla


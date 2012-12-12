#include "HAConverter.h"
#include "Timer.h"
#include "SymbolicTrajPrinter.h"
#include "SymbolicValue.h"
#include "PhaseSimulator.h"
#include "../common/TimeOutError.h"

using namespace std;

namespace hydla {
namespace simulator {

  HAConverter::HAConverter(Opts &opts):Simulator(opts){
  }

  HAConverter::~HAConverter(){}

  HAConverter::phase_result_const_sptr_t HAConverter::simulate()
  {
      hydla::output::SymbolicTrajPrinter printer;
      std::string error_str;
  		is_loop_step_ = false;
      while(!state_stack_.empty()) {
        simulation_phase_sptr_t state(pop_simulation_phase());
        bool consistent;
        
        {
          phase_result_sptr_t& pr = state->phase_result;
          if( opts_->max_phase >= 0 && pr->step >= opts_->max_phase){
            pr->parent->cause_of_termination = simulator::STEP_LIMIT;
            continue;
          }
        }
        
        try{
          try{
            state->module_set_container->reset(state->visited_module_sets);
            simulation_phases_t phases = phase_simulator_->simulate_phase(state, consistent);

            cout << "parent_id : " << state->phase_result->parent->id << endl;          
            for(unsigned int i=0;i<state->phase_result->parent->children.size();i++){
                cout << "********" << i << "*******  " << endl;
	              cout << "PhaseType: " << state->phase_result->parent->children[i]->phase << endl;
	              cout << "id: " <<  state->phase_result->parent->children[i]->id << endl;
                printer.output_one_phase(state->phase_result->parent->children[i]);
                cout << "***************" << endl;
            }
            
            cout << "%% Result: " << phases.size() << "Phases" << endl;
            for(unsigned int i=0; i<phases.size();i++){
              phase_result_sptr_t& pr = phases[i]->phase_result;
              cout << "--- Phase"<< i << " ---" << endl;
              cout << "%% PhaseType: " << pr->phase << endl;
              cout << "%% id: " <<  pr->id << endl;
              cout << "%% step: " << pr->step << endl;
              cout << "%% time: " << *pr->current_time << endl;
              cout << "--- parameter map ---\n" << pr->parameter_map << endl;
            }
          	
          	// とりあえずnd_modeは無視
          	phase_result_sptr_t result_ = state->phase_result->parent->children[0];
          	switch (state->phase_result->phase)
          	{
	          	case PointPhase:
	          	{
          			cout << "～・～・～ PP ～・～・～" << endl;
	          		if(is_loop_step_){
		         			phase_results_.push_back(result_);
	          			loop_.push_back(result_);
	          			if (!check_continue()){
	          				// ループ判定ステップを抜ける
		          			cout << "-------- end loop step ----------" << endl;
		          			is_loop_step_ = false;
	          			}
		         			break;
	          		}else{
		         			phase_results_.push_back(result_);
		         			break;
	          		}
	          	}
	          	case IntervalPhase:
	          	{
          			cout << "～・～・～ IP ～・～・～" << endl;
	          		if(is_loop_step_){
		         			phase_results_.push_back(result_);
	          			loop_.push_back(result_);
	          			if (!check_continue()) {
	          				// ループ判定ステップを抜ける
		          			cout << "-------- end loop step ----------" << endl;
		          			is_loop_step_ = false;
	          				break;
	          			}
	          			if (loop_eq_max_ls()){
	          				// エッジ判定ステップへ
	          				check_edge_step();
	          				continue; //while(!state_stack_.empty()) 
	          			}
		         			break;	          		
	          		}else{
		          		if(check_contain(result_)){
		          			cout << "-------- change loop step ----------" << endl;
		          			is_loop_step_ = true;
	          				loop_.clear();
	          				ls_.clear();
			         			phase_results_.push_back(result_);
		          			set_possible_loops(result_);
		          			loop_.push_back(result_);
		          			break;
		          		}
		         			phase_results_.push_back(result_);
		         			break;	
	          		}
	          	}
          	}
          	          	
          	
            if(!phases.empty()){
              // とりあえずnd_modeは無視
              if(opts_->nd_mode){
                for(simulation_phases_t::iterator it = phases.begin();it != phases.end();it++){
                  if((*it)->phase_result->parent != result_root_){
                    (*it)->module_set_container = msc_no_init_;
                  }
                  else{
                    // TODO これだと，最初のPPで分岐が起きた時のモジュール集合がおかしくなるはず
                    (*it)->module_set_container = msc_original_;
                  }
                  push_simulation_phase(*it);
                }
              }else{
                if(phases[0]->phase_result->parent != result_root_){
                  phases[0]->module_set_container = msc_no_init_;
                }else{
                    // TODO これだと，最初のPPで分岐が起きた時のモジュール集合がおかしくなるはず
                  phases[0]->module_set_container = msc_original_;
                }
                push_simulation_phase(phases[0]);
              }
            }
          }
          catch(const hydla::timeout::TimeOutError &te)
          {
            phase_result_sptr_t& pr = state->phase_result;
            HYDLA_LOGGER_PHASE(te.what());
            if(pr->children.empty()){
              pr->cause_of_termination = TIME_OUT_REACHED;
            }else{
              for(unsigned int i=0;i<pr->children.size();i++){
                pr->children[i]->cause_of_termination = TIME_OUT_REACHED;
              }
            }
          }
        }catch(const std::runtime_error &se)
        {
          error_str = se.what();
          HYDLA_LOGGER_PHASE(se.what());
        }
      }//while(!state_stack_.empty())

      if(!error_str.empty()){
        std::cout << error_str;
      }
      return result_root_;
  }
	
	bool HAConverter::compare_phase_result(phase_result_sptr_t r1, phase_result_sptr_t r2){
		// フェーズ
		if(!(r1->phase == r2->phase)) return false;
		// モジュール集合
		cout << "compare :: id:" << r1->id << " " << r1->module_set->get_name() << " <=> id:" << r2->id << " " << r2->module_set->get_name() << endl;
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
	
	bool HAConverter::check_continue()
	{
		cout << "****** check_continue ******" << endl;		
		std::vector<phase_result_sptrs_t>::iterator it_ls_vec = ls_.begin();		
		while(it_ls_vec != ls_.end()){
			phase_result_sptrs_t::iterator it_ls = (*it_ls_vec).begin();
			phase_result_sptrs_t::iterator it_loop = loop_.begin();
			while(it_ls != (*it_ls_vec).end() && it_loop != loop_.end()) {
				// 等しくない状態遷移列だったらlsから削除
				if(!compare_phase_result(*it_ls, *it_loop)){
					ls_.erase(it_ls_vec);
					break;
				}
				it_ls++;
				it_loop++;
				// loopより短い状態遷移列はlsから削除
				if(it_ls == (*it_ls_vec).end() && it_loop != loop_.end()){
					ls_.erase(it_ls_vec);
					break;					
				}
			}	
			it_ls_vec++;
			cout << "*************************" << endl;		
		}
		
		if(ls_.empty()) {
			cout << "****** check_continue : false ******" << endl;		
			return false;
		}
		cout << "****** check_continue : true ******" << endl;		

		return true;
	}
  	
	void HAConverter::set_possible_loops(phase_result_sptr_t result)
	{
		cout << "****** set_possible_loops ******" << endl;
		phase_result_sptrs_t::iterator it_prs = phase_results_.begin();
		while(it_prs != phase_results_.end()){
			if(compare_phase_result(result, *it_prs)){
				phase_result_sptrs_t candidate_loop;
				copy(it_prs, phase_results_.end(), back_inserter(candidate_loop));
				ls_.push_back(candidate_loop);
			}
			it_prs++;
		}
		
		// lsの中身を表示
		viewLs();
		cout << "****** end set_possible_loops ******" << endl;
	}
  	
	bool HAConverter::loop_eq_max_ls()
	{
		// とりあえずlsの最初の要素が最大のものと仮定
		phase_result_sptrs_t::iterator it_ls = ls_[0].begin();
		phase_result_sptrs_t::iterator it_loop = loop_.begin();
		while(it_ls != ls_[0].end() && it_loop != loop_.end()) {
			if(!compare_phase_result(*it_ls, *it_loop)) return false;
			it_ls++;
			it_loop++;
		}
		// どちらかのイテレータが最後まで達していなかったら等しくない
		if(it_ls != ls_[0].end() || it_loop != loop_.end()) return false;
		
		return true;
	}
  	
	void HAConverter::check_edge_step()
	{
		//とりあえず何もしない
	}
  	
	bool HAConverter::check_contain(phase_result_sptr_t result)
	{
		cout << "～・～・～ check_contain ～・～・～" << endl;
		cout << "・・・・・phase_result・・・・・" << endl;
		viewPrs(phase_results_);
		cout << "・・・・・・・・・・・・・・・・" << endl;
		phase_result_sptrs_t::iterator it_prs = phase_results_.begin();
		while(it_prs != phase_results_.end()){
			if(compare_phase_result(result, *it_prs)) return true;
			it_prs++;
		}
		return false;
	}
	
	void HAConverter::viewLs()
	{
		cout << "・・・・・ ls ・・・・・" << endl;
    hydla::output::SymbolicTrajPrinter printer;
		std::vector<phase_result_sptrs_t>::iterator it_ls_vec = ls_.begin();		
		while(it_ls_vec != ls_.end()){
			viewPrs(*it_ls_vec);
			cout << "・・・・・・・・・・・・" << endl;
			it_ls_vec++;
		}
	}

	void HAConverter::viewPrs(phase_result_sptrs_t results)
	{
    hydla::output::SymbolicTrajPrinter printer;
		phase_result_sptrs_t::iterator it_ls = results.begin();
		while(it_ls != results.end()) {
			printer.output_one_phase(*it_ls);
			it_ls++;
		}	
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
  }

} // simulator
} // hydla


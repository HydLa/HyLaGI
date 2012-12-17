#include "HAConverter.h"
#include "Timer.h"
#include "SymbolicTrajPrinter.h"
#include "SymbolicValue.h"
#include "PhaseSimulator.h"
#include "../common/TimeOutError.h"

using namespace std;

namespace hydla {
namespace simulator {

  HAConverter::HAConverter(Opts &opts):Simulator(opts){}

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
          state->module_set_container->reset(state->visited_module_sets);
		      timer::Timer phase_timer;
		      PhaseSimulator::todo_and_results_t phases = phase_simulator_->simulate_phase(state, consistent);

        	cout << "%% Phases size: " << phases.size() << endl;
          cout << "%% Result PHASE: " << endl;
          cout << "parent_id : " << state->phase_result->parent->id << endl;          
          for(unsigned int i=0;i<phases.size();i++){
		        if(phases[i].result.get() != NULL){
	            phase_result_sptr_t& pr = phases[i].result;
	            cout << "--- Phase"<< i << " ---" << endl;
	            cout << "%% PhaseType: " << pr->phase << endl;
	            cout << "%% id: " <<  pr->id << endl;
              printer.output_one_phase(pr);
		        }
          }
        	
          cout << endl;
        	
          cout << "%% Result TODO:" << endl;
          for(unsigned int i=0; i<phases.size();i++){
		        if(phases[i].todo.get() != NULL){
		            phase_result_sptr_t& pr = phases[i].todo->phase_result;
		            cout << "--- Phase"<< i << " ---" << endl;
		            cout << "%% PhaseType: " << pr->phase << endl;
		            cout << "%% id: " <<  pr->id << endl;
		            cout << "%% step: " << pr->step << endl;
		            cout << "%% time: " << *pr->current_time << endl;
		            cout << "--- parameter map ---\n" << pr->parameter_map << endl;
		        }
          }
       	
        	// �Ƃ肠����nd_mode�͖���
        	phase_result_sptr_t result_ = phases[0].result;
        	switch (state->phase_result->phase)
        	{
          	case PointPhase:
          	{
        			cout << "�`�E�`�E�` PP �`�E�`�E�`" << endl;
          		if(is_loop_step_){
	         			phase_results_.push_back(result_);
          			loop_.push_back(result_);
          			cout << "******* loop *******" << endl;
          			viewPrs(loop_);
          			cout << "******* **** *******" << endl;
          			if (!check_continue()){
          				// ���[�v����X�e�b�v�𔲂���
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
        			cout << "�`�E�`�E�` IP �`�E�`�E�`" << endl;
          		if(is_loop_step_){
	         			phase_results_.push_back(result_);
          			loop_.push_back(result_);
          			cout << "******* loop *******" << endl;
          			viewPrs(loop_);
          			cout << "******* **** *******" << endl;
          			if (!check_continue()) {
          				// ���[�v����X�e�b�v�𔲂���
	          			cout << "-------- end loop step ----------" << endl;
	          			is_loop_step_ = false;
          				break;
          			}
          			if (loop_eq_max_ls()){
          				// �G�b�W����X�e�b�v��
          				check_edge_step();
          				// �����ł�ls_�̗v�f�͂P�ɂȂ��Ă���iloop�Ɠ������ő�̂��̂̂ݎc���Ă���j
          				push_result();
          				output_ha();
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
	          			loop_start_id_ = result_->id;
	          			break;
	          		}
	         			phase_results_.push_back(result_);
	         			break;	
          		}
          	}
        	}
        	
		      if((opts_->max_phase_expanded <= 0 || phase_id_ < opts_->max_phase_expanded) && !phases.empty()){
		        for(unsigned int i = 0;i < phases.size();i++){
		          PhaseSimulator::TodoAndResult& tr = phases[i];
		          if(tr.todo.get() != NULL){
		            if(tr.todo->phase_result->parent != result_root_){
		              tr.todo->module_set_container = msc_no_init_;
		            }
		            else{
		              // TODO ���ꂾ�ƁC�ŏ���PP�ŕ��򂪋N�������̃��W���[���W�������������Ȃ�͂�
		              tr.todo->module_set_container = msc_original_;
		            }
		            tr.todo->elapsed_time = phase_timer.get_elapsed_us() + state->elapsed_time;
		            push_simulation_phase(tr.todo);
		          }if(tr.result.get() != NULL){
		            state->phase_result->parent->children.push_back(tr.result);
		          }
		          if(!opts_->nd_mode)break;
		        }
		      }

        	cout << "***************" << endl << endl;
        	
        }//try
        catch(const hydla::timeout::TimeOutError &te)
        {
		      // �^�C���A�E�g����
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
      return result_root_;
  }
	
	bool HAConverter::compare_phase_result(phase_result_sptr_t r1, phase_result_sptr_t r2){
		// �t�F�[�Y
		if(!(r1->phase == r2->phase)) return false;
		// ���W���[���W��
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
		// �ǂ��炩�̃C�e���[�^���Ō�܂ŒB���Ă��Ȃ������瓙�����Ȃ�
		if(it_1 != r1->positive_asks.end() || it_2 != r2->positive_asks.end()) return false;
		
		return true;
	} 
	
	bool HAConverter::check_continue()
	{
		cout << "****** check_continue ******" << endl;		
		viewLs();
		std::vector<phase_result_sptrs_t>::iterator it_ls_vec = ls_.begin();		
		std::vector< std::vector<phase_result_sptrs_t>::iterator > remove_itr;
		while(it_ls_vec != ls_.end()){
			phase_result_sptrs_t::iterator it_ls = (*it_ls_vec).begin();
			phase_result_sptrs_t::iterator it_loop = loop_.begin();
			while(it_ls != (*it_ls_vec).end() && it_loop != loop_.end()) {
				// �������Ȃ���ԑJ�ڗ񂾂�����ls����폜
				if(!compare_phase_result(*it_ls, *it_loop)){
					cout << "delete ls element : not equal loop" << endl;
					// while�ŃC�e���[�g����ls_�̒��g��erase�ł��Ȃ��̂ŁAerase����itr��ۑ����Č��ls_�̗v�f���폜����
					remove_itr.push_back(it_ls_vec);
					break;
				}
				it_ls++;
				it_loop++;
				// loop���Z����ԑJ�ڗ��ls����폜
				if(it_ls == (*it_ls_vec).end() && it_loop != loop_.end()){
					cout << "delete ls element : less than loop" << endl;
					// while�ŃC�e���[�g����ls_�̒��g��erase�ł��Ȃ��̂ŁAerase����itr��ۑ����Č��ls_�̗v�f���폜����
					remove_itr.push_back(it_ls_vec);
				}
			}	
			it_ls_vec++;
			cout << "*************************" << endl;		
		}
		
		for (unsigned int i = 0 ; i < remove_itr.size() ; i++){
			ls_.erase(remove_itr[i]);
		}
		
		viewLs();
		
		if(ls_.empty()) {
			cout << "****** end check_continue : false ******" << endl;		
			return false;
		}
		cout << "****** end check_continue : true ******" << endl;		

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
		
		// ls�̒��g��\��
		viewLs();
		cout << "****** end set_possible_loops ******" << endl;
	}
  	
	bool HAConverter::loop_eq_max_ls()
	{
		cout << "****** loop_eq_max_ls ******" << endl;
		// �Ƃ肠����ls�̍ŏ��̗v�f���ő�̂��̂Ɖ���
		phase_result_sptrs_t::iterator it_ls = ls_[0].begin();
		phase_result_sptrs_t::iterator it_loop = loop_.begin();
		while(it_ls != ls_[0].end() && it_loop != loop_.end()) {
			if(!compare_phase_result(*it_ls, *it_loop)) {
				cout << "****** end loop_eq_max_ls : false ******" << endl;
				return false;
			}
			it_ls++;
			it_loop++;
		}
		// �ǂ��炩�̃C�e���[�^���Ō�܂ŒB���Ă��Ȃ������瓙�����Ȃ�
		if(it_ls != ls_[0].end() || it_loop != loop_.end()) {
			cout << "****** end loop_eq_max_ls : false ******" << endl;
			return false;
		}
		
		cout << "****** end loop_eq_max_ls : true ******" << endl;
		return true;
	}
  	
	void HAConverter::check_edge_step()
	{
		//�Ƃ肠�����������Ȃ�
		cout << "****** check_edge_step ******" << endl;
		cout << "****** end check_edge_step ******" << endl;
	}
  	
	bool HAConverter::check_contain(phase_result_sptr_t result)
	{
		cout << "�`�E�`�E�` check_contain �`�E�`�E�`" << endl;
		cout << "�E�E�E�E�Ephase_result�E�E�E�E�E" << endl;
		viewPrs(phase_results_);
		cout << "�E�E�E�E�E�E�E�E�E�E�E�E�E�E�E�E" << endl;
		phase_result_sptrs_t::iterator it_prs = phase_results_.begin();
		while(it_prs != phase_results_.end()){
			if(compare_phase_result(result, *it_prs)) return true;
			it_prs++;
		}
		return false;
	}
	
	void HAConverter::viewLs()
	{
		cout << "�E�E�E�E�E ls �E�E�E�E�E" << endl;
    hydla::output::SymbolicTrajPrinter printer;
		std::vector<phase_result_sptrs_t>::iterator it_ls_vec = ls_.begin();		
		while(it_ls_vec != ls_.end()){
			viewPrs(*it_ls_vec);
			cout << "�E�E�E�E�E�E�E�E�E�E�E�E" << endl;
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
	
	void HAConverter::push_result()
	{
		phase_result_sptrs_t result;
		for(unsigned int i = 0 ; i < phase_results_.size() ; i++){
			result.push_back(phase_results_[i]);
			if(phase_results_[i]->id == loop_start_id_) break;
		}
		
		cout << "�E�E�E�E�E result " << ha_results_.size() << " �E�E�E�E�E" << endl;
		viewPrs(result);
		cout << "�E�E�E�E�E�E�E�E�E�E�E�E" << endl;
		ha_results_.push_back(result);
	}
	
	void HAConverter::output_ha()
	{
		cout << "-�E-�E-�E-�EResult Convert�E-�E-�E-�E-" << endl;
		std::vector<phase_result_sptrs_t>::iterator it_ha_res = ha_results_.begin();		
		while(it_ha_res != ha_results_.end()){
			convert_phase_results_to_ha(*it_ha_res);
			it_ha_res++;
		}
		cout << "-�E-�E-�E-�E-�E-�E-�E-�E-�E-�E-�E-�E-" << endl;
	}
	
	void HAConverter::convert_phase_results_to_ha(phase_result_sptrs_t result)
	{
		cout << "digraph g{" << endl;
		cout << "edge [dir=forward];" << endl;
		cout << "\"start\" [shape=point];" << endl;
			cout << "\"start\"->\"" << result[1]->module_set->get_name() << "\" [label=\"" << result[0]->module_set->get_name() << "\", labelfloat=false,arrowtail=dot];" << endl;
		for(unsigned int i = 2 ; i < result.size() ; i++){
			if(result[i]->phase == IntervalPhase){
				cout << "\"" << result[i-2]->module_set->get_name() << "\"->\"" << result[i]->module_set->get_name() << "\" [label=\"" << result[i-1]->module_set->get_name() << "\", labelfloat=false,arrowtail=dot];" << endl;
			}
		}
		cout << "}" << endl;
	}
  
		void HAConverter::initialize(const parse_tree_sptr& parse_tree)
  {
    Simulator::initialize(parse_tree);
    //������Ԃ�����ăX�^�b�N�ɓ����
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


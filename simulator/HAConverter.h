#ifndef _INCLUDED_HYDLA_HAConverter_SIMULATOR_H_
#define _INCLUDED_HYDLA_HAConverter_SIMULATOR_H_

#include "Simulator.h"


namespace hydla {
namespace simulator {

class HAConverter:public Simulator{
public:
  typedef PhaseResult                                       phase_result_t;
  typedef boost::shared_ptr<const phase_result_t>           phase_result_const_sptr_t;
  typedef PhaseSimulator                                    phase_simulator_t;
  typedef phase_result_t::phase_result_sptr_t               phase_result_sptr_t;
  typedef std::vector<phase_result_sptr_t >                 phase_result_sptrs_t;
  
  typedef SimulationTodo                                   simulation_phase_t;
  typedef boost::shared_ptr<SimulationTodo>                simulation_phase_sptr_t;
  typedef std::vector<simulation_phase_sptr_t>              simulation_phases_t;

  typedef phase_result_t::variable_map_t variable_map_t;
  typedef phase_result_t::variable_t     variable_t;
  typedef phase_result_t::parameter_t    parameter_t;
  typedef phase_result_t::value_t        value_t;
  typedef phase_result_t::parameter_map_t     parameter_map_t;
  
  typedef std::list<variable_t>                            variable_set_t;
  typedef std::list<parameter_t>                           parameter_set_t;
  typedef value_t                                          time_value_t;
	
  HAConverter(Opts &opts);

  virtual ~HAConverter();
  /**
   * �^����ꂽ����⃂�W���[���W�������ɃV�~�����[�V�������s�������Ȃ�
   */
  virtual phase_result_const_sptr_t simulate();
  /**
   * �V�~�����[�^�̏��������s��
   */
  virtual void initialize(const parse_tree_sptr& parse_tree);
	
private: 
   //�Ƃ肠����SequentialSimulator.h�Ɠ���  ���Ƃ��ƕK�v�ɉ����Ēǉ�
   /**
   * �V�~�����[�V�������Ŏg�p�����ϐ��\�̌��^
   */
  variable_map_t variable_map_;

  
  /*
   * �V�~�����[�V�������Ɏg�p�����ϐ��ƋL���萔�̏W��
   */
  variable_set_t variable_set_;
  parameter_set_t parameter_set_;
  
  
  /**
   * �V�~�����[�V�����ΏۂƂȂ�p�[�X�c���[
   */
  parse_tree_sptr parse_tree_;
	
	
	/**
	 * �ϊ��ɗp����ϐ��̒�`
	 * ���̍\����
 	*/
	struct CurrentCondition
	{
		phase_result_sptrs_t phase_results;
		std::vector<phase_result_sptrs_t> ls;
		phase_result_sptrs_t loop;
		bool is_loop_step;
		int loop_start_id;	
		CurrentCondition(
			phase_result_sptrs_t phase_results, 
			std::vector<phase_result_sptrs_t> ls, 
			phase_result_sptrs_t loop,
			bool is_loop_step,
			int loop_start_id
		): phase_results(phase_results), ls(ls), loop(loop), is_loop_step(is_loop_step), loop_start_id(loop_start_id){}
		CurrentCondition(){}
	};

	typedef CurrentCondition															 current_condition_t;
	typedef std::deque<current_condition_t> 					 		 current_conditions_t;
	
	//current_condition_sptr_t cc_;
	
	current_conditions_t cc_vec_;
		
	std::vector<phase_result_sptrs_t> ha_results_;
	
	// ���[�v����X�e�b�v�𑱂��邩�ils�̗v�f��loop�𕔕��W���Ƃ�����̂����邩�j
	bool check_continue(current_condition_t cc);
	
	// ls�Ƀ��[�v����set
	void set_possible_loops(phase_result_sptr_t result, current_condition_t *cc);
	
	// loop��ls�̍ő�v�f�Ɠ������ǂ���
	bool loop_eq_max_ls(current_condition_t cc);
	
	// �G�b�W����X�e�b�v
	void check_edge_step();
	
	// ��phase_result��phase_results�Ɋ܂܂�邩
	bool check_contain(phase_result_sptr_t result, current_condition_t cc);

	// �Q��phase_result��phase�A���W���[���W���Apositive_ask�����������ǂ�������
	bool compare_phase_result(phase_result_sptr_t r1, phase_result_sptr_t r2);
 
	// ls�̒��g�\��
	void viewLs(current_condition_t cc);
	// phase_result_sptrs_t�̒��g�\��
	void viewPrs(phase_result_sptrs_t results);
	
	// �������ꂽ�S�Ă�HA���o�́idot����j
	void output_ha();
	
	// phase_results��ha��dot����ɕϊ�����
	void convert_phase_results_to_ha(phase_result_sptrs_t result);
	
	// ha_result��HA�ϊ��ɕK�v�ȏ���push����
	void push_result(current_condition_t cc);
	
	// ��ԃL���[�ɐV���ȏ�Ԃ�ǉ�����
	// push_simulation_phase�Ɖ�킹��K�v����
	void push_current_condition(const current_condition_t cc)
	{
		HYDLA_LOGGER_HA("push cc");
		cc_vec_.push_front(cc);
	}

	// ��ԃL���[�����Ԃ��ЂƂ��o��
	// pop_simulation_phase()�ƍ��킹��K�v����
	current_condition_t pop_current_condition()
	{
    current_condition_t cc;
    if(opts_->search_method == simulator::DFS){
      cc = cc_vec_.front();
      cc_vec_.pop_front();
    }else{
      cc = cc_vec_.back();
      cc_vec_.pop_back();
    }
    return cc;

	}
	
};
	
} // simulator
} // hydla

#endif // _INCLUDED_HYDLA_HAConverter_SIMULATOR_H_


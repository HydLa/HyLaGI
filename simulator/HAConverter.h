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
	*/
	bool is_loop_step_;
	phase_result_sptrs_t phase_results_;
	std::vector<phase_result_sptrs_t> ls_;
	std::vector<phase_result_sptrs_t> ha_results_;
	phase_result_sptrs_t loop_;
	std::map<int, phase_result_sptr_t> phase_result_map_;
	
	int loop_start_id_;
	

	// ���[�v����X�e�b�v�𑱂��邩�ils�̗v�f��loop�𕔕��W���Ƃ�����̂����邩�j
	bool check_continue();
	
	// ls�Ƀ��[�v����set
	void set_possible_loops(phase_result_sptr_t result);
	
	// loop��ls�̍ő�v�f�Ɠ������ǂ���
	bool loop_eq_max_ls();
	
	// �G�b�W����X�e�b�v
	void check_edge_step();
	
	// ��phase_result��phase_results�Ɋ܂܂�邩
	bool check_contain(phase_result_sptr_t result);

	// �Q��phase_result��phase�A���W���[���W���Apositive_ask�����������ǂ�������
	bool compare_phase_result(phase_result_sptr_t r1, phase_result_sptr_t r2);
 
	// ls�̒��g�\��
	void viewLs();
	// phase_result_sptrs_t�̒��g�\��
	void viewPrs(phase_result_sptrs_t results);
	
	// �������ꂽ�S�Ă�HA���o�́idot����j
	void output_ha();
	
	// phase_results��ha��dot����ɕϊ�����
	void convert_phase_results_to_ha(phase_result_sptrs_t result);
	
	// ha_result��HA�ϊ��ɕK�v�ȏ���push����
	void push_result();
	
};
	
} // simulator
} // hydla

#endif // _INCLUDED_HYDLA_HAConverter_SIMULATOR_H_


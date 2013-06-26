
#ifndef _INCLUDED_HYDLA_HAConverter_SIMULATOR_H_
#define _INCLUDED_HYDLA_HAConverter_SIMULATOR_H_


#include "BatchSimulator.h"
#include "Node.h"
#include "DefaultTreeVisitor.h"
#include "PhaseResult.h"

#include <map>
#include <string>


namespace hydla {
namespace simulator {

class HAConverter: public BatchSimulator{
public:
	
	HAConverter(Opts &opts);

  virtual ~HAConverter();

  virtual phase_result_const_sptr_t simulate();

  virtual void process_one_todo(simulation_todo_sptr_t& todo);

protected:

	typedef hydla::ch::module_set_sptr 												module_set_sptr_t;
	typedef std::vector<module_set_sptr_t>					 					module_set_sptrs_t;
	
	/**
	 * �ϊ��ɗp����ϐ��̒�`
	 * ���̍\����
 	*/
	enum ResultCheckOccurrence 
	{
		Reachable,
		Unreachable,
		Unknown
	};
	
	// �G�b�W�\���� ���藧�K�[�h�����{�̗p�������W���[���W��
	struct Phase
	{
		module_set_sptr_t module_set;
		negative_asks_t negative_asks;
		positive_asks_t positive_asks;
		
		Phase(module_set_sptr_t module_set, negative_asks_t negative_asks, positive_asks_t positive_asks): 
			module_set(module_set), negative_asks(negative_asks), positive_asks(positive_asks){}
		
		Phase(){}
	};
	
	typedef Phase																phase_t;
  typedef std::vector<phase_t>								phases_t;
	phases_t passed_edge;
	phases_t passed_node;


	typedef Phase																edge_t;
  typedef std::vector<edge_t>									edges_t;

	typedef Phase																node_t;
  typedef std::vector<node_t>              		nodes_t;
	
	void viewEdge(phase_t edge);
	void viewNode(phase_result_sptr_t node);
	typedef std::multimap<phase_result_sptr_t, phase_t>					 unknown_map_t;
	
	// �G�b�W����X�e�b�v�̌���
	struct ResultCheckReachable
	{
		// ����node����h������G�b�W  �m�[�h�Ƃ̑g�ŕ\��
		unknown_map_t reachable;
		unknown_map_t unknown;
		ResultCheckReachable(unknown_map_t reachable, unknown_map_t unknown): reachable(reachable), unknown(unknown){}
		ResultCheckReachable(){}
	};
	typedef ResultCheckReachable																result_check_reachable_t;

	
	struct CurrentCondition
	{
		phase_result_sptrs_t phase_results;
		std::vector<phase_result_sptrs_t> ls;
		phase_result_sptrs_t loop;
		bool is_loop_step;
		unknown_map_t unknown_map;
		int loop_count;
		int loop_start_id;
		// �Ȃ����x�����o��̂ŃR�����g�A�E�g�D�K�v���Ȃ��̂�
		/*
		CurrentCondition(
			phase_result_sptrs_t phase_results, 
			std::vector<phase_result_sptrs_t> ls, 
			phase_result_sptrs_t loop,
			bool is_loop_step,
			int loop_count,
			int loop_start_id,
			unknown_map_t unknown_map
		): phase_results(phase_results), ls(ls), loop(loop), is_loop_step(is_loop_step), loop_count(loop_count), loop_start_id(loop_start_id), unknown_map(unknown_map){}
		*/
		CurrentCondition(){}
	};

	typedef CurrentCondition															 current_condition_t;
	typedef std::deque<current_condition_t> 					 		 current_conditions_t;
	
	typedef std::map<phase_result_sptrs_t, unknown_map_t> 	ha_result_t;

	// �G�b�W�ʉ߉\������̌��ʂ�ێ�
	result_check_reachable_t result_check_reachable_;
	
	// ��ԃL���[
	current_conditions_t cc_vec_;
	
	// �ϊ����ʂ�ێ�
	ha_result_t ha_results_;
	
	// ���[�v����X�e�b�v�𑱂��邩�ils�̗v�f��loop�𕔕��W���Ƃ�����̂����邩�j
	bool check_continue(current_condition_t *cc);
	
	// ls�Ƀ��[�v����set
	void set_possible_loops(phase_result_sptr_t result, current_condition_t *cc);
	
	// loop��ls�̍ő�v�f�Ɠ������ǂ���
	bool loop_eq_max_ls(current_condition_t cc);
	
	// �G�b�W����X�e�b�v �m�[�h�����A���ꂼ��checkEdge���s��
	void checkNode(current_condition_t cc);
	// �m�[�h���Ƃ̃G�b�W����
	void checkEdge(phase_result_sptr_t node, current_condition_t cc);
	// �t�F�[�Y�����łɒʉ߂������̂��ǂ������� �ʉ߂��Ă���true
	bool check_passed_phase(phases_t passed_phase, phase_t phase);
	
	void create_asks_vec(boost::shared_ptr<parse_tree::Ask> ask);	
	std::vector<negative_asks_t> vec_negative_asks;
	std::vector<positive_asks_t> vec_positive_asks;


	
	// �t�F�[�Y���m���������̂��ǂ�������
	bool compare_phase(phase_t p1, phase_t p2);
	// node����edge�ɑJ�ڂ���\���͂��邩���� �ʉ߉\���̔���
	ResultCheckOccurrence check_occurrence(phase_result_sptr_t node, phase_t edge, current_condition_t cc);

	// ��phase_result��phase_results�Ɋ܂܂�邩
	bool check_contain(phase_result_sptr_t result, current_condition_t cc);

	// �Q��phase_result��phase�A���W���[���W���Apositive_ask�����������ǂ�������
	bool compare_phase_result(phase_result_sptr_t r1, phase_result_sptr_t r2);
 
	// ls�̒��g�\��
	void viewLs(current_condition_t cc);
	// phase_result_sptrs_t�̒��g�\��
	void viewPrs(phase_result_sptrs_t results);
	// phase_result_sptr_t�̒��g�\��
	void viewPr(phase_result_sptr_t result);
	// asks�̒��g�\��
	void viewAsks(ask_set_t asks);
	
	// �������ꂽ�S�Ă�HA���o�́idot����j
	void output_ha();
	
	// phase_results��ha��dot����ɕϊ�����
	void convert_phase_results_to_ha(phase_result_sptrs_t result, unknown_map_t unknown_map);
	
	// ha_result��HA�ϊ��ɕK�v�ȏ���push����
	void push_result(current_condition_t cc);
	
	// asks��A�˂���������擾
	std::string get_asks_str(ask_set_t asks);
	
	// ��ԃL���[�ɐV���ȏ�Ԃ�ǉ�����
	// push_simulation_phase�Ɖ�킹��K�v����
	void push_current_condition(const current_condition_t cc)
	{
	  //HYDLA_LOGGER_HA("push cc");
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


};//HAConverter

class GuardGetter : public parse_tree::DefaultTreeVisitor {
public:
	virtual void accept(const boost::shared_ptr<parse_tree::Node>& n);
	
	GuardGetter();
	virtual ~GuardGetter();
	
	// Ask
	virtual void visit(boost::shared_ptr<parse_tree::Ask> node);
	
	ask_set_t asks;
};//GuardGetter


}//namespace hydla
}//namespace simulator 

#endif // _INCLUDED_HYDLA_HAConverter_SIMULATOR_H_


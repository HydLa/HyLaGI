#ifndef _INCLUDED_HYDLA_SIMULATOR_H_
#define _INCLUDED_HYDLA_SIMULATOR_H_

#include <deque>

#include "Node.h"
#include "ParseTree.h"
#include "ModuleSetContainer.h"

#include "PhaseResult.h"
#include "DefaultParameter.h"
#include "ContinuityMapMaker.h"

namespace hydla {
namespace simulator {

typedef enum{
  DFS,
  BFS
}SearchMethod;

typedef struct Opts_ {
  std::string mathlink;
  bool debug_mode;
  std::string max_time;
  bool nd_mode;
  bool interactive_mode;
  bool ha_convert_mode;
  bool dump_relation;
  bool profile_mode;
  bool parallel_mode;
  int parallel_number;
  bool dump_in_progress;
  bool stop_at_failure;
  std::string output_interval;
  int output_precision;
  int approx_precision;
  std::string solver;
  hydla::parse_tree::node_sptr assertion;
  std::set<std::string> output_variables;
  int optimization_level;
  int timeout;
  int timeout_phase;
  int timeout_case;
  int timeout_calc;
  int max_loop_count;
  int max_phase;
  int max_phase_expanded;
  SearchMethod search_method;
} Opts;

typedef boost::shared_ptr<hydla::ch::ModuleSet>           module_set_sptr;
typedef hydla::ch::ModuleSetContainer                     module_set_container_t;
typedef boost::shared_ptr<module_set_container_t>  module_set_container_sptr;
typedef hydla::ch::ModuleSetContainer::module_set_list_t  module_set_list_t;
typedef boost::shared_ptr<hydla::parse_tree::ParseTree>  parse_tree_sptr;
typedef boost::shared_ptr<const hydla::ch::ModuleSet>    module_set_const_sptr;
typedef boost::shared_ptr<hydla::ch::ModuleSetContainer> module_set_container_sptr;

typedef std::map<boost::shared_ptr<hydla::parse_tree::Ask>, bool> entailed_prev_map_t;
typedef std::map<variable_t*, simulator::ValueRange, VariableComparator>      variable_range_map_t;

typedef std::map<std::string, unsigned int> profile_t;


/**
 * �V�~�����[�V�������ׂ��t�F�[�Y��\���\����
 */
struct SimulationTodo{
  typedef std::map<hydla::ch::ModuleSet, variable_range_map_t> ms_cache_t;

  Phase                     phase;
  int                       id;
  time_t                    current_time;
  parameter_map_t           parameter_map;
  positive_asks_t           positive_asks;
  negative_asks_t           negative_asks;
  expanded_always_t         expanded_always;
  entailed_prev_map_t       judged_prev_map;

  /// �O�̃t�F�[�Y
  phase_result_sptr_t parent;

  /// �t�F�[�Y���ňꎞ�I�ɒǉ����鐧��D���򏈗��ȂǂɎg�p
  constraints_t temporary_constraints;
  /// �g�p���鐧�񃂃W���[���W���D�i�t�F�[�Y���ƂɁC��always������܂ނ��ۂ��̍�������j
  module_set_container_sptr module_set_container;
  /// ������̃��W���[���W����ێ����Ă����D���򏈗����C�����W���𕡐��񒲂ׂ邱�Ƃ������悤��
  /// TODO:����C���ꂪ�܂Ƃ��Ɏg���Ă��Ȃ��C������D�܂�C�����Ԉ���Ă���\�������邵�C���ʂ͊m���ɂ���
  module_set_list_t ms_to_visit;
  /// �v���t�@�C�����O����
  profile_t profile;
  /// ��������P�[�X�̌v�Z����
  int elapsed_time;
  /// map to cache result of calculation for each module_set
  ms_cache_t ms_cache;
  
  /**
   * reset members to calculate from the start of the phase
   * TODO: expanded_always�ǂ����悤�D
   * TODO: �L���萔�����ɖ߂��ׂ��H������͌���ł͂����܂Ŗ��Ȃ��͂�
   */
  void reset_from_start_of_phase(){
    ms_cache.clear();
    temporary_constraints.clear();
    ms_to_visit = module_set_container->get_full_ms_list();
    positive_asks.clear();
    negative_asks.clear();
    judged_prev_map.clear();
  }
};


typedef boost::shared_ptr<SimulationTodo>     simulation_todo_sptr_t;
// �v���t�@�C�����O���ʑS��
// �eTodo���Ƃɂ����������ԁi����ł́CTodo���̂��̂�ۑ����Ă���j
typedef std::vector<simulation_todo_sptr_t> entire_profile_t;

class PhaseSimulator;

typedef PhaseResult                                       phase_result_t;
typedef PhaseSimulator                                    phase_simulator_t;

typedef boost::shared_ptr<SimulationTodo>                simulation_todo_sptr_t;



struct TodoContainer
{
  virtual void push_todo(simulation_todo_sptr_t& todo)
  {
    container_.push_back(todo);
  }
  
  virtual simulation_todo_sptr_t pop_todo()
  {
    simulation_todo_sptr_t todo = container_.back();
    container_.pop_back();
    return todo;
  }
  
  virtual bool empty()
  {
    return container_.empty();
  }
  
  virtual int size()
  {
    return container_.size();
  }
  
  protected:
  std::deque<simulation_todo_sptr_t> container_;
};

typedef TodoContainer                                    todo_container_t;

typedef std::list<variable_t>                            variable_set_t;

struct ParameterAndRange
{
  parameter_t parameter;
  range_t     range;
  ParameterAndRange(const parameter_t& p, const range_t& r):parameter(p), range(r){}
};

typedef std::list<ParameterAndRange>                     parameter_set_t;

class Simulator
{
public:  

  Simulator(Opts& opts);
  
  virtual ~Simulator(){}

  /**
   * simulate using given parse_tree
   * @return root of the tree which expresses the result-trajectories
   */
  virtual phase_result_const_sptr_t simulate() = 0;
  
  virtual void initialize(const parse_tree_sptr& parse_tree);
  
  
  /**
   * set the phase simulator which this simulator uses in each phase
   * @param ps a pointer to an instance of phase_simlulator_t (caller must not delete the instance)
   */
  virtual void set_phase_simulator(phase_simulator_t *ps);
  
  /**
   * @return set of introduced parameters and their ranges of values
   */
  parameter_set_t get_parameter_set(){return *parameter_set_;}
  
  phase_result_sptr_t get_result_root(){return result_root_;}
  
  /**
   * push the initial state of simulation into the stack
   */
  virtual simulation_todo_sptr_t make_initial_todo();
  
  /**
   * template of variable maps
   */
  boost::shared_ptr<variable_map_t> variable_map_;
  
  /*
   * set of variables
   */
  boost::shared_ptr<variable_set_t> variable_set_;

  /*
   * set of pairs of introduced parameters and their ranges of values
   */
  boost::shared_ptr<parameter_set_t> parameter_set_;


protected:
  
  /**
   * �V�~�����[�V�������Ɏg�p�����ϐ��\�̃I���W�i���̍쐬
   */
  virtual void init_variable_map(const parse_tree_sptr& parse_tree);
  
  void init_module_set_container(const parse_tree_sptr& parse_tree);
  
  void reset_result_root();


  parse_tree_sptr parse_tree_;
  
  /**
   * PhaseSimulator to use
   */ 
  boost::shared_ptr<phase_simulator_t > phase_simulator_;

  /**
   * ����⃂�W���[���W���̃R���e�i
   */
  module_set_container_sptr msc_original_;
  
  /**
   * mcs_original_�����always���������������
   */
  module_set_container_sptr msc_no_init_;

  /** 
   * root of the tree of result trajectories
   */
  phase_result_sptr_t result_root_;

  Opts*     opts_;

  private:

};

} //namespace simulator
} //namespace hydla 

#endif // _INCLUDED_HYDLA_SIMULATOR_H_
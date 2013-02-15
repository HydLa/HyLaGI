#ifndef _INCLUDED_HYDLA_SIMULATOR_H_
#define _INCLUDED_HYDLA_SIMULATOR_H_

#include <iostream>
#include <string>
#include <stack>
#include <cassert>


#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
//#include <boost/function.hpp>
#include <boost/iterator/indirect_iterator.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/xpressive/xpressive.hpp>

#include "Logger.h"

#include "Node.h"
#include "ParseTree.h"
#include "ModuleSetContainer.h"
#include "ModuleSetContainerCreator.h"
#include "ModuleSetGraph.h"
#include "ModuleSetList.h"


#include "AskDisjunctionSplitter.h"
#include "AskDisjunctionFormatter.h"

#include "PhaseResult.h"
#include "InitNodeRemover.h"
#include "TreeInfixPrinter.h"
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



typedef hydla::parse_tree::node_id_t                      node_id_t;
typedef boost::shared_ptr<hydla::ch::ModuleSet>           module_set_sptr;
typedef hydla::ch::ModuleSetContainer                     module_set_container_t;
typedef boost::shared_ptr<module_set_container_t>  module_set_container_sptr;
typedef hydla::ch::ModuleSetContainer::module_set_list_t  module_set_list_t;
typedef boost::shared_ptr<hydla::parse_tree::ParseTree>  parse_tree_sptr;
typedef boost::shared_ptr<const hydla::ch::ModuleSet>    module_set_const_sptr;
typedef boost::shared_ptr<hydla::ch::ModuleSetContainer> module_set_container_sptr;




typedef std::map<std::string, unsigned int> profile_t;




/**
 * �V�~�����[�V�������ׂ��t�F�[�Y��\���\����
 */
struct SimulationTodo{
  typedef std::map<hydla::ch::ModuleSet, std::map<DefaultVariable*, boost::shared_ptr<Value> , VariableComparator> > ms_cache_t;
  /// ���s���ʂƂȂ�t�F�[�Y
  boost::shared_ptr<PhaseResult> phase_result;
  /// �t�F�[�Y���ňꎞ�I�ɒǉ����鐧��D���򏈗��ȂǂɎg�p
  constraints_t temporary_constraints;
  /// �g�p���鐧�񃂃W���[���W���D�i�t�F�[�Y���ƂɁC��always������܂ނ��ۂ��̍�������j
  module_set_container_sptr module_set_container;
  /// ������̃��W���[���W����ێ����Ă����D���򏈗����C�����W���𕡐��񒲂ׂ邱�Ƃ������悤��
  module_set_list_t ms_to_visit;
  /// �v���t�@�C�����O����
  profile_t profile;
  /// ��������P�[�X�̌v�Z����
  int elapsed_time;
  /// map to cache result of calculation for each module_set
  ms_cache_t ms_cache;
  
  SimulationTodo(){}
  /// �R�s�[�R���X�g���N�^
  SimulationTodo(const SimulationTodo& phase):
    phase_result(phase.phase_result), 
    temporary_constraints(phase.temporary_constraints),
    module_set_container(phase.module_set_container),
    ms_to_visit(phase.ms_to_visit)
  {
  }
};


typedef boost::shared_ptr<SimulationTodo>     simulation_phase_sptr_t;
typedef std::vector<simulation_phase_sptr_t>   simulation_phases_t;
/// �v���t�@�C�����O�̌��� ���O�Ǝ��Ԃ̃}�b�v
typedef std::vector<simulation_phase_sptr_t> entire_profile_t;


std::ostream& operator<<(std::ostream& s, const constraints_t& a);
std::ostream& operator<<(std::ostream& s, const ask_set_t& a);
std::ostream& operator<<(std::ostream& s, const tells_t& a);
std::ostream& operator<<(std::ostream& s, const collected_tells_t& a);
std::ostream& operator<<(std::ostream& s, const expanded_always_t& a);
}
}

namespace {
  using namespace hydla::ch;
  using namespace hydla::simulator;
  using namespace hydla::parse_tree;
  struct ModuleSetContainerInitializer {
    typedef boost::shared_ptr<hydla::parse_tree::ParseTree> parse_tree_sptr;
    typedef hydla::simulator::module_set_container_sptr         module_set_container_sptr;
    template<typename MSCC>
      static void init(
          const parse_tree_sptr& parse_tree,
          module_set_container_sptr& msc_original, 
          module_set_container_sptr& msc_no_init,
          parse_tree_sptr& member_parse_tree)
      {
        ModuleSetContainerCreator<MSCC> mcc;
        {
          parse_tree_sptr pt_original(boost::make_shared<ParseTree>(*parse_tree));
          AskDisjunctionFormatter().format(pt_original.get());
          AskDisjunctionSplitter().split(pt_original.get());
          msc_original = mcc.create(pt_original);
        }

        {
          parse_tree_sptr pt_no_init(boost::make_shared<ParseTree>(*parse_tree));
          InitNodeRemover().apply(pt_no_init.get());
          AskDisjunctionFormatter().format(pt_no_init.get());
          AskDisjunctionSplitter().split(pt_no_init.get());
          msc_no_init = mcc.create(pt_no_init);

          // �œK�����ꂽ�`�̃p�[�X�c���[�𓾂�
          member_parse_tree = pt_no_init;
        }
      }
  };
}


namespace hydla {
namespace simulator {



class PhaseSimulator;


typedef PhaseResult                                       phase_result_t;
typedef boost::shared_ptr<const phase_result_t>           phase_result_const_sptr_t;
typedef PhaseSimulator                                    phase_simulator_t;

typedef SimulationTodo                                   simulation_phase_t;
typedef boost::shared_ptr<SimulationTodo>                simulation_phase_sptr_t;
typedef std::vector<simulation_phase_sptr_t>              simulation_phases_t;

typedef std::list<variable_t>                            variable_set_t;

struct ParameterAndRange
{
  parameter_t parameter;
  range_t     range;
  ParameterAndRange(const parameter_t& p, const range_t& r):parameter(p), range(r){}
};

typedef std::list<ParameterAndRange>                     parameter_set_t;
typedef value_t                                          time_value_t;

/**
 * �V�~�����[�V�����S�̂̐i�s��S������N���X
 * �E���񃂃W���[���W���̔������W������͂Ƃ��C���O���Q���o�͂���
 * �E�e���O���́C����0����PP��IP���������i�ޕ����ɂ̂݌J��Ԃ��Đi�߂邱�Ƃœ�������̂Ƃ���
 */
class Simulator
{
public:  

  Simulator(Opts& opts);
  
  virtual ~Simulator(){}

  /**
   * �^����ꂽ����⃂�W���[���W�������ɃV�~�����[�V�������s�������Ȃ�
   */
  virtual phase_result_const_sptr_t simulate() = 0;
  
  /**
   * �g�p����PhaseSimulator��ݒ肷��D
   * ���̊֐��ɓn��PhaseSimulator�̃C���X�^���X��new�ō쐬���C�Ăяo������delete���Ȃ��悤�ɂ���
   */
  void set_phase_simulator(phase_simulator_t *ps);
  
  virtual void initialize(const parse_tree_sptr& parse_tree);
  
  void init_module_set_container(const parse_tree_sptr& parse_tree);
  
  /**
   * �V�~�����[�V�������Ɏg�p�����ϐ��\�̃I���W�i���̍쐬
   */
  virtual void init_variable_map(const parse_tree_sptr& parse_tree);
  
  /**
   * push the initial state of simulation into the stack
   */
  virtual void push_initial_state();
  
  /**
   * �v���t�@�C�����O�̌��ʂ��擾
   */
  entire_profile_t get_profile(){return profile_vector_;}
  
  /**
   * get set of introduced parameters and their values
   */
  parameter_set_t get_parameter_set(){return parameter_set_;}

protected:

  /**
   * ��ԃL���[�ɐV���ȏ�Ԃ�ǉ�����
   */
  virtual void push_simulation_phase(const simulation_phase_sptr_t& state)
  {
    state->phase_result->id = phase_id_++;
    state_stack_.push_front(state);
  }

  /**
   * ��ԃL���[�����Ԃ��ЂƂ��o��
   */
  simulation_phase_sptr_t pop_simulation_phase()
  {
    simulation_phase_sptr_t state;
    if(opts_->search_method == simulator::DFS){
      state = state_stack_.front();
      state_stack_.pop_front();
    }else{
      state = state_stack_.back();
      state_stack_.pop_back();
    }
    profile_vector_.push_back(state);
    // �Ƃ肠�����C���̊֐��Ŏ�肾�������͕̂K���V�~�����[�V�������s�����Ƃ�O��ɂ���D
    return state;
  }
  /**
   * @return maximum module set without initial constraint
   */
  module_set_sptr get_max_ms_no_init()const
  {
    return msc_no_init_->get_max_module_set();
  }
  
  
  
  /// ���ۂɃV�~�����[�^����v�Z�Ȃǂ��s�����t�F�[�Y�̏W���D��Ƀv���t�@�C�����O�̂��߂Ɏ���Ă����D
  entire_profile_t profile_vector_;

  /**
   * �V�~�����[�V�����ΏۂƂȂ�p�[�X�c���[
   */
  parse_tree_sptr parse_tree_;
  
  
  /**
   * �V�~�����[�V�������Ŏg�p�����ϐ��\�̌��^
   */
  variable_map_t variable_map_;

  
  /*
   * �V�~�����[�V�������Ɏg�p�����ϐ��̏W��
   */
  variable_set_t variable_set_;
  /*
   * �V�~�����[�V�������Ɏg�p�����L���萔�Ƃ��̒l�̏W��
   */
  parameter_set_t parameter_set_;
  int state_id_;

  /**
   * �g�p����PhaseSimulator
   */ 
  boost::shared_ptr<phase_simulator_t > phase_simulator_;


  module_set_container_sptr msc_original_;
  module_set_container_sptr msc_no_init_;

  /**
   * �e��Ԃ�ۑ����Ă������߂̃X�^�b�N
   */
  std::deque<simulation_phase_sptr_t> state_stack_;
  
  parse_tree_sptr parse_tree;
  
  /// ���O���؂̍��D������ԂȂ̂ŁC�q���ȊO�̏��͓���Ȃ�
  phase_result_sptr_t result_root_;
  
  Opts*     opts_;
  /**
   * �ePhaseResult�ɐU���Ă���ID
   */
  int phase_id_;
};

} //namespace simulator
} //namespace hydla 

#endif // _INCLUDED_HYDLA_SIMULATOR_H_

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

//TODO:PhaseResultType����Ȃ��āCValueType�����e���v���[�g�N���X�ɂł��Ȃ��H�Ƃ������݌v�Č����D���̃N���X�͉��ɑΉ����Ă���H
//���s�A���S���Y�����낤���H
/**
 * �V�~�����[�V�����S�̂̐i�s��S������N���X
 * �E���񃂃W���[���W���̔������W������͂Ƃ��C���O���Q���o�͂���
 * �E�e���O���́C����0����PP��IP���������i�ޕ����ɂ̂݌J��Ԃ��Đi�߂邱�Ƃœ�������̂Ƃ���
 */
class Simulator
{
public:  
  typedef PhaseResult                                    phase_result_t;
  typedef boost::shared_ptr<const phase_result_t>        phase_result_const_sptr_t;
  typedef phase_result_t::phase_result_sptr_t            phase_result_sptr_t;
  typedef std::vector<phase_result_sptr_t >              phase_result_sptrs_t;
  typedef std::vector<phase_result_const_sptr_t >        phase_result_const_sptrs_t;
  typedef PhaseSimulator                                 phase_simulator_t;
  
  typedef SimulationTodo                                simulation_phase_t;
  typedef boost::shared_ptr<SimulationTodo>             simulation_phase_sptr_t;

  typedef phase_result_t::variable_map_t      variable_map_t;
  typedef phase_result_t::variable_t          variable_t;
  typedef phase_result_t::parameter_t         parameter_t;
  typedef phase_result_t::value_t             value_t;
  typedef phase_result_t::parameter_map_t     parameter_map_t;
  
  typedef std::list<variable_t>                            variable_set_t;
  typedef std::list<parameter_t>                           parameter_set_t;
  typedef value_t                                          time_value_t;

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
   * �v���t�@�C�����O�̌��ʂ��擾
   */
  entire_profile_t get_profile(){return profile_vector_;}

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
   * �V�~�����[�V�������Ɏg�p�����ϐ��ƋL���萔�̏W��
   */
  variable_set_t variable_set_;
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

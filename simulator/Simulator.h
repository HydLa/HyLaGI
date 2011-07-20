#ifndef _INCLUDED_HYDLA_SIMULATOR_H_
#define _INCLUDED_HYDLA_SIMULATOR_H_

#include <iostream>
#include <string>
#include <stack>
#include <cassert>

#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
//#include <boost/function.hpp>

#include "Logger.h"

#include "Node.h"
#include "ParseTree.h"
#include "ModuleSetContainer.h"

#include "VariableMap.h"
#include "PhaseState.h"
#include "InitNodeRemover.h"
#include "TreeInfixPrinter.h"

namespace hydla {
namespace simulator {

template<typename PhaseStateType>
class Simulator
{
public:  
  typedef PhaseStateType                                   phase_state_t; 
  typedef typename boost::shared_ptr<phase_state_t>        phase_state_sptr; 
  typedef typename boost::shared_ptr<const phase_state_t>  phase_state_const_sptr; 

  typedef typename phase_state_t::variable_map_t variable_map_t;
  typedef typename phase_state_t::variable_t     variable_t;
  typedef typename phase_state_t::value_t        value_t;

  typedef boost::shared_ptr<hydla::parse_tree::ParseTree>  parse_tree_sptr;

  typedef boost::shared_ptr<hydla::ch::ModuleSet>          module_set_sptr;
  typedef boost::shared_ptr<const hydla::ch::ModuleSet>    module_set_const_sptr;
  typedef boost::shared_ptr<hydla::ch::ModuleSetContainer> module_set_container_sptr;

  Simulator()
  {}
  
  virtual ~Simulator()
  {}

  /**
   * �^����ꂽ����⃂�W���[���W�������ɃV�~�����[�V�������s�������Ȃ�
   */
  virtual void simulate()
  {
    while(!state_stack_.empty()) {
      phase_state_sptr state(pop_phase_state());
      state->module_set_container->reset();
      do{
        if(simulate_phase_state(state->module_set_container->get_module_set(), state)){
          state->module_set_container->mark_nodes();
          break;
        }
      }while(state->module_set_container->go_next());
    }
  }

  /**
   * ��ԃL���[�ɐV���ȏ�Ԃ�ǉ�����
   */
  virtual void push_phase_state(const phase_state_sptr& state) 
  {
    state_stack_.push(state);
  }

  /**
   * ��ԃL���[�����Ԃ��ЂƂ��o��
   */
  phase_state_sptr pop_phase_state()
  {
    phase_state_sptr state(state_stack_.top());
    state_stack_.pop();
    return state;
  }

  void initialize(const parse_tree_sptr& parse_tree)
  {
    parse_tree_ = parse_tree;
    init_variable_map(parse_tree);
    do_initialize(parse_tree);
  }

  /**
   * �V����PhaseState�̍쐬
   */
  phase_state_sptr create_new_phase_state() const
  {
    phase_state_sptr ph(new phase_state_t());
    return ph;
  }

  /**
   * �^����ꂽPhaseState�̏����������p�����C
   * �V����PhaseState�̍쐬
   */
  phase_state_sptr create_new_phase_state(const phase_state_const_sptr& old) const
  {
    phase_state_sptr ph(new phase_state_t(*old));
    return ph;
  }

  /**
   * �V�~�����[�V�������Ɏg�p�����ϐ��\�̃I���W�i���̍쐬
   */
  virtual void init_variable_map(const parse_tree_sptr& parse_tree)
  {
    typedef hydla::parse_tree::ParseTree::variable_map_const_iterator vmci;

    vmci it  = parse_tree->variable_map_begin();
    vmci end = parse_tree->variable_map_end();
    for(; it != end; ++it)
    {
      for(int d=0; d<=it->second; ++d) {
        variable_t v;
        v.name             = it->first;
        v.derivative_count = d;
        variable_map_.set_variable(v, value_t());
      }
    }

    HYDLA_LOGGER_REST(
      "#*** variable map ***\n",
      variable_map_);
  }

  virtual bool simulate_phase_state(const module_set_sptr& ms, 
                                    const phase_state_const_sptr& state)
  {      
    bool ret = false;
    hydla::parse_tree::TreeInfixPrinter printer;
    switch(state->phase) 
    {
    case PointPhase:

      { 
      HYDLA_LOGGER_MS_SUMMARY(
          "#***** begin point phase *****",
          "\n#** module set **\n",
          ms->get_name(),
          "\n",
          *ms);

        ret = point_phase(ms, state);
        break;
      }

    case IntervalPhase: 
      {

          HYDLA_LOGGER_MS_SUMMARY(
          "#***** begin interval phase *****",
          "\n#** module set **\n",
          ms->get_name(),
          "\n",
          *ms);

        ret = interval_phase(ms, state);
        break;            
      }

    default:
      assert(0);
    }

    return ret;
  }


  /**
   * id�`����expanded_always��shared_ptr�`���ɕϊ�����
   */
  void expanded_always_id2sptr(const expanded_always_id_t& ea_id, 
                               expanded_always_t& ea_sptr) const
  {
    ea_sptr.clear();

    expanded_always_id_t::const_iterator it  = ea_id.begin();
    expanded_always_id_t::const_iterator end = ea_id.end();
    for(; it!=end; ++it) {
      assert(
        boost::dynamic_pointer_cast<hydla::parse_tree::Always>(
          parse_tree_->get_node(*it)));

      ea_sptr.insert(
        boost::static_pointer_cast<hydla::parse_tree::Always>(
          parse_tree_->get_node(*it)));
    }
  }

  /**
   * shared_ptr�`����expanded_always��id�`���ɕϊ�����
   */
  void expanded_always_sptr2id(const expanded_always_t& ea_sptr, 
                               expanded_always_id_t& ea_id) const
  {
    ea_id.clear();

    expanded_always_t::const_iterator it  = ea_sptr.begin();
    expanded_always_t::const_iterator end = ea_sptr.end();
    for(; it!=end; ++it) {
      //ea_id.push_back(parse_tree_->get_node_id(*it));
      ea_id.push_back((*it)->get_id());
    }
  }

  /**
   * Point Phase�̏���
   */
  virtual bool point_phase(const module_set_sptr& ms, 
                           const phase_state_const_sptr& state) = 0;

  /**
   * Interval Phase�̏���
   */
  virtual bool interval_phase(const module_set_sptr& ms, 
                              const phase_state_const_sptr& state) = 0;

protected:
  virtual void do_initialize(const parse_tree_sptr& parse_tree)
  {}

  /**
   * �V�~�����[�V�����ΏۂƂȂ�p�[�X�c���[
   */
  parse_tree_sptr parse_tree_;

  /**
   * �V�~�����[�V�������Ŏg�p����邷�ׂĂ̕ϐ����i�[�����\
   */
  variable_map_t variable_map_;

  /**
   * �e��Ԃ�ۑ����Ă������߂̃X�^�b�N
   */
  std::stack<phase_state_sptr> state_stack_;
};

} //namespace simulator
} //namespace hydla 

#endif // _INCLUDED_HYDLA_SIMULATOR_H_

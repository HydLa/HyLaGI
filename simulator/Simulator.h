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
  typedef typename phase_state_t::parameter_t     parameter_t;
  typedef typename phase_state_t::value_t        value_t;

  typedef boost::shared_ptr<hydla::parse_tree::ParseTree>  parse_tree_sptr;

  typedef boost::shared_ptr<hydla::ch::ModuleSet>          module_set_sptr;
  typedef boost::shared_ptr<const hydla::ch::ModuleSet>    module_set_const_sptr;
  typedef boost::shared_ptr<hydla::ch::ModuleSetContainer> module_set_container_sptr;
  typedef std::list<variable_t>                             variable_set_t;
  typedef std::list<parameter_t>                            parameter_set_t;

  Simulator()
  {}
  
  virtual ~Simulator()
  {}

  /**
   * 与えられた解候補モジュール集合を元にシミュレーション実行をおこなう
   */
  virtual void simulate()
  {
    assert(0);
  }

  /**
   * 状態キューに新たな状態を追加する
   */
  virtual void push_phase_state(const phase_state_sptr& state) 
  {
    state->id = state_id_++;
    HYDLA_LOGGER_PHASE("%% Simulator::push_phase_state\n");
    HYDLA_LOGGER_PHASE("%% state id: ", state->id);
    HYDLA_LOGGER_PHASE("%% state time: ", state->current_time);
    HYDLA_LOGGER_PHASE("--- parent state variable map ---\n", state->parent->variable_map);
    HYDLA_LOGGER_PHASE("--- state parameter map ---\n", state->parameter_map);
    state_stack_.push(state);
  }

  /**
   * 状態キューから状態をひとつ取り出す
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
   * 新たなPhaseStateの作成
   */
  phase_state_sptr create_new_phase_state() const
  {
    phase_state_sptr ph(new phase_state_t());
    return ph;
  }

  /**
   * 与えられたPhaseStateの情報をを引き継いだ，
   * 新たなPhaseStateの作成
   */
  phase_state_sptr create_new_phase_state(const phase_state_const_sptr& old) const
  {
    phase_state_sptr ph(new phase_state_t(*old));
    return ph;
  }

  /**
   * シミュレーション時に使用される変数表のオリジナルの作成
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
        variable_set_.push_front(v);
        variable_map_.set_variable(&(variable_set_.front()), value_t());
      }
    }

    HYDLA_LOGGER_REST(
      "#*** variable map ***\n",
      variable_map_);
  }

  virtual bool simulate_phase_state(const module_set_sptr& ms, 
                                    phase_state_sptr& state)
  {
    HYDLA_LOGGER_PHASE("#*** Begin Simulator::simulate_phase_state ***");
    HYDLA_LOGGER_PHASE("%% current time:", state->current_time);
    HYDLA_LOGGER_PHASE("--- parent variable map ---\n", state->parent->variable_map);
    HYDLA_LOGGER_PHASE("--- parameter map ---\n", state->parameter_map);
    HYDLA_LOGGER_PHASE("--- module set ---\n",
          ms->get_name(),
          "\n",
          ms->get_infix_string() );
          
    bool ret = false;
    switch(state->phase) 
    {
    case PointPhase:

      { 
      HYDLA_LOGGER_PHASE("%% begin point phase");

        ret = point_phase(ms, state);
        break;
      }

    case IntervalPhase: 
      {
      HYDLA_LOGGER_PHASE("%% begin interval phase");

        ret = interval_phase(ms, state);
        break;            
      }

    default:
      assert(0);
    }

    HYDLA_LOGGER_PHASE("#*** End Simulator::simulate_phase_state ***");
    return ret;
  }


  /**
   * id形式のexpanded_alwaysをshared_ptr形式に変換する
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
   * shared_ptr形式のexpanded_alwaysをid形式に変換する
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
   * Point Phaseの処理
   */
  virtual bool point_phase(const module_set_sptr& ms, 
                           phase_state_sptr& state) = 0;

  /**
   * Interval Phaseの処理
   */
  virtual bool interval_phase(const module_set_sptr& ms, 
                              phase_state_sptr& state) = 0;

protected:
  virtual void do_initialize(const parse_tree_sptr& parse_tree)
  {}

  /**
   * シミュレーション対象となるパースツリー
   */
  parse_tree_sptr parse_tree_;
  
  
  /**
   * シミュレーション中で使用される変数表の原型
   */
  variable_map_t variable_map_;

  
  /*
   * シミュレーション中に使用される変数と記号定数の集合
   */
  variable_set_t variable_set_;
  parameter_set_t parameter_set_;
  int state_id_;

  /**
   * 各状態を保存しておくためのスタック
   */
  std::stack<phase_state_sptr> state_stack_;
};

} //namespace simulator
} //namespace hydla 

#endif // _INCLUDED_HYDLA_SIMULATOR_H_

#ifndef _INCLUDED_HYDLA_SIMULATOR_H_
#define _INCLUDED_HYDLA_SIMULATOR_H_

#include <iostream>

#include <string>
#include <queue>

#include <assert.h>

#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
//#include <boost/function.hpp>

#include "Node.h"
#include "ParseTree.h"
#include "ModuleSetContainer.h"

#include "VariableMap.h"
#include "PhaseState.h"
#include "InitNodeRemover.h"

namespace hydla {
namespace simulator {

template<typename PhaseStateType>
class Simulator
{
public:
  typedef PhaseStateType                         phase_state_t; 
  typedef typename phase_state_t::variable_map_t variable_map_t;
  typedef typename phase_state_t::variable_t     variable_t;
  typedef typename phase_state_t::value_t        value_t;

  typedef boost::shared_ptr<hydla::parse_tree::ParseTree>  parse_tree_sptr;
  typedef typename boost::shared_ptr<PhaseStateType>       phase_state_sptr; 
  typedef hydla::ch::module_set_sptr                       module_set_sptr;
  typedef boost::shared_ptr<hydla::ch::ModuleSetContainer> module_set_container_sptr;

  Simulator(bool debug_mode = false) :
    debug_mode_(debug_mode)    
  {}
  
  virtual ~Simulator()
  {}

  void set_module_set_container(const module_set_container_sptr& msc)
  {
    msc_ = msc;
  }

  void set_module_set_container_no_init(const module_set_container_sptr& msc_no_init)
  {
    msc_no_init_ = msc_no_init;
  }
  
  void set_debug_mode(bool m)
  {
    debug_mode_ = m;
  }

  bool is_debug_mode() const 
  {
    return debug_mode_;
  }

  /**
   * 与えられた解候補モジュール集合を元にシミュレーション実行をおこなう
   */
  void simulate()
  {
    assert(msc_);
    assert(msc_no_init_);

    while(!state_queue_.empty()) {
      phase_state_sptr state(pop_phase_state());

      switch(state->phase) 
      {
        case phase_state_t::PointPhase:
          msc_->dispatch(
            boost::bind(&Simulator<phase_state_t>::point_phase, 
                        this, _1, state));
          break;

        case phase_state_t::IntervalPhase:
          msc_->dispatch(
            boost::bind(&Simulator<phase_state_t>::interval_phase, 
                        this, _1, state));
          break;
        
        default:
          assert(0);
      }
    }
  }

  /**
   * 状態キューに新たな状態を追加する
   */
  void push_phase_state(const phase_state_sptr& state) 
  {
    state_queue_.push(state);
  }

  /**
   * 状態キューから状態をひとつ取り出す
   */
  phase_state_sptr pop_phase_state()
  {
    phase_state_sptr state(state_queue_.front());
    state_queue_.pop();
    return state;
  }

  void initialize(
    const parse_tree_sptr&           parse_tree,
    const module_set_container_sptr& msc, 
    const module_set_container_sptr& msc_no_init)
  {
    msc_         = msc;
    msc_no_init_ = msc_no_init;

    init_variable_map(parse_tree);
    init_state_queue(parse_tree);
    do_initialize();
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
        variable_map_.set_variable(v, value_t());
      }
    }

    if(is_debug_mode()) {
      std::cout << "#*** variable map ***" 
                << variable_map_ 
                << std::endl;
    }
  }

  virtual void init_state_queue(const parse_tree_sptr& parse_tree)
  {
    phase_state_sptr init_state(new phase_state_t);
    init_state->phase = phase_state_t::PointPhase;
    init_state->variable_map = variable_map_;

    push_phase_state(init_state);
  }

  /**
   * Point Phaseの処理
   */
  virtual bool point_phase(module_set_sptr& ms, phase_state_sptr& state) = 0;

  /**
   * Interval Phaseの処理
   */
  virtual bool interval_phase(module_set_sptr& ms, phase_state_sptr& state) = 0;

private:
  virtual void do_initialize()
  {}

  module_set_container_sptr msc_;
  module_set_container_sptr msc_no_init_;
  bool debug_mode_;

  /**
   * シミュレーション中で使用されるすべての変数を格納した表
   */
  variable_map_t variable_map_;

  /**
   * 各状態を保存しておくためのキュー
   */
  std::queue<phase_state_sptr> state_queue_;
};

} //namespace simulator
} //namespace hydla 

#endif // _INCLUDED_HYDLA_SIMULATOR_H_

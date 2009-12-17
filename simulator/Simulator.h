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

  typedef boost::shared_ptr<hydla::parse_tree::ParseTree> parse_tree_sptr;
  typedef typename boost::shared_ptr<PhaseStateType> phase_state_sptr; 

  Simulator()
  {}
  
  virtual ~Simulator()
  {}

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
  void simulate(boost::shared_ptr<hydla::ch::ModuleSetContainer> msc,
                parse_tree_sptr pt)
  {
    parse_tree_sptr pt_no_init_node(new hydla::parse_tree::ParseTree(*pt));
    InitNodeRemover init_node_remover;
    init_node_remover.apply(pt_no_init_node.get());
    if(debug_mode_) {
      std::cout << "#*** No Initial Node Tree ***\n"
        << *pt_no_init_node << std::endl;
    }

    init_state_queue(pt);

    while(!state_queue_.empty()) {
      phase_state_sptr state(pop_phase_state());

      switch(state->phase) 
      {
        case phase_state_t::PointPhase:
          msc->dispatch(
            boost::bind(&Simulator<phase_state_t>::point_phase, 
                        this, _1, state));
          break;

        case phase_state_t::IntervalPhase:
          msc->dispatch(
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

  virtual void init_state_queue(const parse_tree_sptr& pt)
  {
    phase_state_sptr init_state(new phase_state_t);
    init_state->phase = phase_state_t::PointPhase;

    // 変数表作成
    typedef hydla::parse_tree::ParseTree::variable_map_const_iterator vmci;
    vmci it  = pt->variable_map_begin();
    vmci end = pt->variable_map_end();
    for(; it != end; ++it)
    {
      for(int d=0; d<=it->second; ++d) {
        variable_t v;
        v.name             = it->first;
        v.derivative_count = d;
        init_state->variable_map.set_variable(v, value_t());
      }
    }

    std::cout << init_state->variable_map;

    push_phase_state(init_state);
  }

  /**
   * Point Phaseの処理
   */
  virtual bool point_phase(hydla::ch::module_set_sptr& ms, phase_state_sptr& state) = 0;

  /**
   * Interval Phaseの処理
   */
  virtual bool interval_phase(hydla::ch::module_set_sptr& ms, phase_state_sptr& state) = 0;

private:

  bool debug_mode_;

  /**
   * 各状態を保存しておくためのキュー
   */
  std::queue<phase_state_sptr> state_queue_;
};

} //namespace simulator
} //namespace hydla 

#endif // _INCLUDED_HYDLA_SIMULATOR_H_

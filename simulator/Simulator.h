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
#include "ModuleSetContainer.h"

#include "VariableMap.h"
#include "PhaseState.h"

namespace hydla {
namespace simulator {

template<typename PhaseStateType>
class Simulator
{
public:
  typedef PhaseStateType phase_state_t;
  typedef typename boost::shared_ptr<PhaseStateType> phase_state_sptr; 

  Simulator()
  {}
  
  virtual ~Simulator()
  {}

  /**
   * 与えられた解候補モジュール集合を元にシミュレーション実行をおこなう
   */
  void simulate(boost::shared_ptr<hydla::ch::ModuleSetContainer> msc)
                //              boost::function<bool (hydla::ch::module_set_sptr& ms, phase_state_sptr& state)> point_phase_func)
  //                const hydla::parse_tree::variable_map_t& variable_map)
  {
    phase_state_sptr init_state(new phase_state_t);
    init_state->phase = phase_state_t::PointPhase;
    state_queue_.push(init_state);

    while(!state_queue_.empty()) {
      phase_state_sptr state(state_queue_.front());
      state_queue_.pop();
      std::cout << "state queue size:" << state_queue_.size() << std::endl;

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
   * Point Phaseの処理
   */
  virtual bool point_phase(hydla::ch::module_set_sptr& ms, phase_state_sptr& state) = 0;

  /**
   * Interval Phaseの処理
   */
  virtual bool interval_phase(hydla::ch::module_set_sptr& ms, phase_state_sptr& state) = 0;

private:
//  boost::function<bool (hydla::ch::module_set_sptr& ms, phase_state_sptr& state)> point_phase_func_;

  // 各状態を保存しておくためのキュー
  std::queue<phase_state_sptr> state_queue_;
};

} //namespace simulator
} //namespace hydla 

#endif // _INCLUDED_HYDLA_SIMULATOR_H_

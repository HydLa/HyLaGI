#ifndef _INCLUDED_HYDLA_SIMULATOR_PHASE_STATE_H_
#define _INCLUDED_HYDLA_SIMULATOR_PHASE_STATE_H_

#include <set>

namespace hydla {
namespace simulator {

/**
 * 各処理の状態
 */
template<typename VariableMapType, 
         typename TimeType>
struct PhaseState {
  typedef VariableMapType variable_map_t;
  typedef std::set<boost::shared_ptr<hydla::parse_tree::Always> > expanded_always_t;

  /**
   * 処理のフェーズ
   */
  typedef enum Phase_ {
    PointPhase,
    IntervalPhase,
  } Phase;

  Phase               phase;
  TimeType            time;
  variable_map_t      variable_map;
  expanded_always_t   expanded_always;
  bool                initial_time;
};


} // namespace simulator
} // namespace hydla 

#endif // _INCLUDED_HYDLA_SIMULATOR_PHASE_STATE_H_


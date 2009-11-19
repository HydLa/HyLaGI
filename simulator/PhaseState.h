#ifndef _INCLUDED_HYDLA_SIMULATOR_PHASE_STATE_H_
#define _INCLUDED_HYDLA_SIMULATOR_PHASE_STATE_H_

namespace hydla {
namespace simulator {

/**
 * 各処理の状態
 */
template<typename VariableMapType, 
         typename TimeType>
struct PhaseState {
  typedef VariableMapType variable_map_t;
  typedef boost::shared_ptr<hydla::parse_tree::Always>  always_sptr;
  typedef std::vector<always_sptr>                      always_sptr_list_t;

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
  always_sptr_list_t  expanded_always;
  bool                initial_time;
};


} // namespace simulator
} // namespace hydla 

#endif // _INCLUDED_HYDLA_SIMULATOR_PHASE_STATE_H_


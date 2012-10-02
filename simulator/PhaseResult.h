#ifndef _INCLUDED_HYDLA_SIMULATOR_PHASE_RESULT_H_
#define _INCLUDED_HYDLA_SIMULATOR_PHASE_RESULT_H_

#include <vector>

#include <boost/shared_ptr.hpp>

#include "DefaultVariable.h"
#include "ValueRange.h"
#include "./VariableMap.h"
#include "./Types.h"

#include "Timer.h"

namespace hydla {
namespace simulator {

template<typename value> class DefaultParameter;

/**
 * あるフェーズの情報を表すクラス
 */
template<typename ValueType>
struct PhaseResult {
  typedef ValueType                                         value_t;
  typedef ValueRange<value_t>                               range_t;
  typedef DefaultVariable                                   variable_t;
  typedef DefaultParameter<value_t>                         parameter_t;
  typedef value_t                                           time_t;
  typedef VariableMap<variable_t*, value_t>                 variable_map_t;
  typedef VariableMap<parameter_t*, range_t>                parameter_map_t;
  typedef boost::shared_ptr<PhaseResult>                    phase_result_sptr_t;
  typedef std::vector<phase_result_sptr_t >                 phase_result_sptrs_t;


  Phase                     phase;
  int id;
  time_t                    current_time;
  time_t                    end_time;
  variable_map_t            variable_map;
  parameter_map_t           parameter_map;
  expanded_always_t         expanded_always;
  positive_asks_t           positive_asks;
  changed_asks_t            changed_asks;
  /// フェーズ内で一時的に追加する制約．分岐処理などに使用
  constraints_t temporary_constraints;
  module_set_container_sptr module_set_container;
  /// 判定済みのモジュール集合を保持しておく．分岐処理時，同じ集合を複数回調べることが無いように
  std::set<module_set_sptr> visited_module_sets;
  /// シミュレーション実行ステップ数．IP を一度終えるごとに1増加する
  int step;

  /// フェーズの終了原因を表す．
  CauseOfTermination cause_of_termination;
  /// 次のフェーズ
  phase_result_sptrs_t children;
  /// 前のフェーズ
  phase_result_sptr_t parent;

  /// フェーズの処理にかかった時間
  timer::Timer phase_timer;
  /// フェーズの処理内でcalculate_closureにかかった時間
  timer::Timer calculate_closure_timer;

};


} // namespace simulator
} // namespace hydla 

#endif // _INCLUDED_HYDLA_SIMULATOR_PHASE_RESULT_H_


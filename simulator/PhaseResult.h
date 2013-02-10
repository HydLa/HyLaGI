#ifndef _INCLUDED_HYDLA_SIMULATOR_PHASE_RESULT_H_
#define _INCLUDED_HYDLA_SIMULATOR_PHASE_RESULT_H_

#include <vector>

#include <boost/shared_ptr.hpp>

#include "DefaultVariable.h"
#include "ValueRange.h"
#include "ModuleSet.h"

#include "Timer.h"

namespace hydla {
namespace simulator {

struct DefaultParameter;


/**
 * type for cause of termination of simulation
 */
typedef enum{
  TIME_LIMIT,
  STEP_LIMIT,
  SOME_ERROR,
  INCONSISTENCY,
  ASSERTION,
  OTHER_ASSERTION,
  TIME_OUT_REACHED,
  NOT_UNIQUE_IN_INTERVAL,
  NONE
}CauseOfTermination;


/**
 * type of a phase
 */
typedef enum Phase_ {
  PointPhase,
  IntervalPhase,
} Phase;


typedef std::vector<boost::shared_ptr<hydla::parse_tree::Node> > constraints_t;
typedef std::vector<boost::shared_ptr<hydla::parse_tree::Tell> > tells_t;
typedef std::set<boost::shared_ptr<hydla::parse_tree::Tell> >    collected_tells_t;
typedef std::set<boost::shared_ptr<hydla::parse_tree::Always> >  expanded_always_t;
typedef std::set<boost::shared_ptr<hydla::parse_tree::Ask> >     ask_set_t;
typedef ask_set_t                                                positive_asks_t;
typedef ask_set_t                                                negative_asks_t;
typedef std::vector<tells_t>                                     not_adopted_tells_list_t;

/**
 * あるフェーズのシミュレーション結果を表すクラス
 * 解軌道木上のノードに対応する
 */

struct PhaseResult {
  typedef boost::shared_ptr<PhaseResult>                    phase_result_sptr_t;
  typedef std::vector<phase_result_sptr_t >                 phase_result_sptrs_t;

  typedef boost::shared_ptr<Value>                          value_t;
  typedef ValueRange                                        range_t;
  typedef DefaultVariable                                   variable_t;
  typedef DefaultParameter                                  parameter_t;
  typedef value_t                                           time_t;
  
  typedef std::map<variable_t*, value_t, VariableComparator>                    variable_map_t;

  typedef std::map<parameter_t*, range_t>                   parameter_map_t;

  Phase                     phase;
  int id;
  time_t                    current_time, end_time;
  variable_map_t            variable_map;
  parameter_map_t           parameter_map;
  expanded_always_t         expanded_always;
  positive_asks_t           positive_asks;
  negative_asks_t           negative_asks;
  int step;
  hydla::ch::module_set_sptr module_set;

  /// フェーズの終了原因を表す．
  CauseOfTermination cause_of_termination;
  /// 次のフェーズ
  phase_result_sptrs_t children;
  /// 前のフェーズ
  phase_result_sptr_t parent;
};

std::ostream& operator<<(std::ostream& s, const PhaseResult::parameter_map_t& pm);
std::ostream& operator<<(std::ostream& s, const PhaseResult::variable_map_t& vm);

} // namespace simulator
} // namespace hydla 

#endif // _INCLUDED_HYDLA_SIMULATOR_PHASE_RESULT_H_

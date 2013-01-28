#ifndef _INCLUDED_HYDLA_SIMULATOR_TYPES_H_
#define _INCLUDED_HYDLA_SIMULATOR_TYPES_H_

#include <vector>
#include <set>
#include <list>

#include <boost/shared_ptr.hpp>

#include "Node.h"
#include "ModuleSet.h"
#include "ModuleSetContainer.h"
#include "ParseTree.h"
#include "Timer.h"
#include "DefaultVariable.h"
#include "Value.h"

namespace hydla {
namespace simulator {

struct PhaseResult;

/**
 * 処理のフェーズ
 */
typedef enum Phase_ {
  PointPhase,
  IntervalPhase,
} Phase;

typedef enum{
  DFS,
  BFS
}SearchMethod;


typedef struct Opts_ {
  std::string mathlink;
  bool debug_mode;
  std::string max_time;
  bool nd_mode;
  bool interactive_mode;
  bool ha_convert_mode;
  bool dump_relation;
  bool profile_mode;
  bool parallel_mode;
  int parallel_number;
  bool dump_in_progress;
  bool stop_at_failure;
  std::string output_interval;
  int output_precision;
  int approx_precision;
  std::string solver;
  hydla::parse_tree::node_sptr assertion;
  std::set<std::string> output_variables;
  int optimization_level;
  int timeout;
  int timeout_phase;
  int timeout_case;
  int timeout_calc;
  int max_loop_count;
  int max_phase;
  int max_phase_expanded;
  SearchMethod search_method;
} Opts;

/**
 * 
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

typedef hydla::parse_tree::node_id_t                      node_id_t;
typedef boost::shared_ptr<hydla::ch::ModuleSet>           module_set_sptr;
typedef hydla::ch::ModuleSetContainer                     module_set_container_t;
typedef boost::shared_ptr<module_set_container_t>  module_set_container_sptr;
typedef hydla::ch::ModuleSetContainer::module_set_list_t  module_set_list_t;


typedef std::vector<boost::shared_ptr<hydla::parse_tree::Node> > constraints_t;
typedef std::vector<boost::shared_ptr<hydla::parse_tree::Tell> > tells_t;
typedef std::set<boost::shared_ptr<hydla::parse_tree::Tell> >    collected_tells_t;
typedef std::set<boost::shared_ptr<hydla::parse_tree::Always> >  expanded_always_t;
typedef std::set<boost::shared_ptr<hydla::parse_tree::Ask> >     ask_set_t;
typedef ask_set_t                                                positive_asks_t;
typedef ask_set_t                                                negative_asks_t;
typedef std::vector<tells_t>                                     not_adopted_tells_list_t;
typedef std::map<std::string, int>                               continuity_map_t;


typedef boost::shared_ptr<hydla::parse_tree::ParseTree>  parse_tree_sptr;

typedef boost::shared_ptr<const hydla::ch::ModuleSet>    module_set_const_sptr;
typedef boost::shared_ptr<hydla::ch::ModuleSetContainer> module_set_container_sptr;

typedef std::map<std::string, unsigned int> profile_t;

/**
 * シミュレーションすべきフェーズを表す構造体
 */
struct SimulationTodo{
  typedef std::map<hydla::ch::ModuleSet, std::map<DefaultVariable*, boost::shared_ptr<Value> > > ms_cache_t;
  /// 実行結果となるフェーズ
  boost::shared_ptr<PhaseResult> phase_result;
  /// フェーズ内で一時的に追加する制約．分岐処理などに使用
  constraints_t temporary_constraints;
  /// 使用する制約モジュール集合．（フェーズごとに，非always制約を含むか否かの差がある）
  module_set_container_sptr module_set_container;
  /// 未判定のモジュール集合を保持しておく．分岐処理時，同じ集合を複数回調べることが無いように
  module_set_list_t ms_to_visit;
  /// プロファイリング結果
  profile_t profile;
  /// 所属するケースの計算時間
  int elapsed_time;
  /// map to cache result of calculation for each module_set
  ms_cache_t ms_cache;
  
  SimulationTodo(){}
  /// コピーコンストラクタ
  SimulationTodo(const SimulationTodo& phase):
    phase_result(phase.phase_result), 
    temporary_constraints(phase.temporary_constraints),
    module_set_container(phase.module_set_container),
    ms_to_visit(phase.ms_to_visit)
  {
  }
};

typedef boost::shared_ptr<SimulationTodo>     simulation_phase_sptr_t;
typedef std::vector<simulation_phase_sptr_t>   simulation_phases_t;

/// プロファイリングの結果 名前と時間のマップ
typedef std::vector<simulation_phase_sptr_t> entire_profile_t;

std::ostream& operator<<(std::ostream& s, const constraints_t& a);
std::ostream& operator<<(std::ostream& s, const ask_set_t& a);
std::ostream& operator<<(std::ostream& s, const tells_t& a);
std::ostream& operator<<(std::ostream& s, const collected_tells_t& a);
std::ostream& operator<<(std::ostream& s, const expanded_always_t& a);
std::ostream& operator<<(std::ostream& s, const continuity_map_t& continuity_map);

} //namespace simulator
} //namespace hydla 

#endif // _INCLUDED_HYDLA_SIMULATOR_TYPES_H_


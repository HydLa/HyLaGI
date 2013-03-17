#ifndef _INCLUDED_HYDLA_SIMULATOR_H_
#define _INCLUDED_HYDLA_SIMULATOR_H_

#include <deque>

#include "Node.h"
#include "ParseTree.h"
#include "ModuleSetContainer.h"

#include "PhaseResult.h"
#include "DefaultParameter.h"
#include "ContinuityMapMaker.h"

namespace hydla {
namespace simulator {

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

typedef boost::shared_ptr<hydla::ch::ModuleSet>           module_set_sptr;
typedef hydla::ch::ModuleSetContainer                     module_set_container_t;
typedef boost::shared_ptr<module_set_container_t>  module_set_container_sptr;
typedef hydla::ch::ModuleSetContainer::module_set_list_t  module_set_list_t;
typedef boost::shared_ptr<hydla::parse_tree::ParseTree>  parse_tree_sptr;
typedef boost::shared_ptr<const hydla::ch::ModuleSet>    module_set_const_sptr;
typedef boost::shared_ptr<hydla::ch::ModuleSetContainer> module_set_container_sptr;

typedef std::map<boost::shared_ptr<hydla::parse_tree::Ask>, bool> entailed_prev_map_t;
typedef std::map<variable_t*, simulator::ValueRange, VariableComparator>      variable_range_map_t;

typedef std::map<std::string, unsigned int> profile_t;

/**
 * シミュレーションすべきフェーズを表す構造体
 */
struct SimulationTodo{
  typedef std::map<hydla::ch::ModuleSet, variable_range_map_t> ms_cache_t;

  Phase                     phase;
  int                       id;
  time_t                    current_time;
  parameter_map_t           parameter_map;
  positive_asks_t           positive_asks;
  negative_asks_t           negative_asks;
  expanded_always_t         expanded_always;
  entailed_prev_map_t       judged_prev_map;

  /// 前のフェーズ
  phase_result_sptr_t parent;

  /// フェーズ内で一時的に追加する制約．分岐処理などに使用
  constraints_t temporary_constraints;
  /// 使用する制約モジュール集合．（フェーズごとに，非always制約を含むか否かの差がある）
  module_set_container_sptr module_set_container;
  /// 未判定のモジュール集合を保持しておく．分岐処理時，同じ集合を複数回調べることが無いように
  /// TODO:現状，これがまともに使われていない気がする．つまり，何か間違っている可能性があるし，無駄は確実にある
  module_set_list_t ms_to_visit;
  /// プロファイリング結果
  profile_t profile;
  /// 所属するケースの計算時間
  int elapsed_time;
  /// map to cache result of calculation for each module_set
  ms_cache_t ms_cache;
  
  /**
   * reset members to calculate from the start of the phase
   * TODO: expanded_alwaysどうしよう．
   * TODO: 記号定数も元に戻すべき？こちらは現状ではそこまで問題ないはず
   */
  void reset_from_start_of_phase(){
    ms_cache.clear();
    temporary_constraints.clear();
    ms_to_visit = module_set_container->get_full_ms_list();
    positive_asks.clear();
    negative_asks.clear();
    judged_prev_map.clear();
  }
};


typedef boost::shared_ptr<SimulationTodo>     simulation_todo_sptr_t;
// プロファイリング結果全体
// 各Todoごとにかかった時間（現状では，Todoそのものを保存している）
typedef std::vector<simulation_todo_sptr_t> entire_profile_t;

class PhaseSimulator;

typedef PhaseResult                                       phase_result_t;
typedef PhaseSimulator                                    phase_simulator_t;

typedef boost::shared_ptr<SimulationTodo>                simulation_todo_sptr_t;

typedef std::deque<simulation_todo_sptr_t>               todo_container_t;

typedef std::list<variable_t>                            variable_set_t;

struct ParameterAndRange
{
  parameter_t parameter;
  range_t     range;
  ParameterAndRange(const parameter_t& p, const range_t& r):parameter(p), range(r){}
};

typedef std::list<ParameterAndRange>                     parameter_set_t;

/**
 * シミュレーション全体の進行を担当するクラス
 * ・制約モジュール集合の半順序集合を入力とし，解軌道群を出力する
 * ・各解軌道は，時刻0からPPとIPを時刻が進む方向にのみ繰り返して進めることで得られるものとする
 */
class Simulator
{
public:  

  Simulator(Opts& opts);
  
  virtual ~Simulator(){}

  /**
   * simulate using given parse_tree
   * @return root of the tree which expresses the result-trajectories
   */
  virtual phase_result_const_sptr_t simulate() = 0;
  
  /**
   * set the phase simulator which this simulator uses in each phase
   * @param ps a pointer to an instance of phase_simlulator_t (caller must not delete the instance)
   */
  void set_phase_simulator(phase_simulator_t *ps);
  
  virtual void initialize(const parse_tree_sptr& parse_tree);
  
  
  /**
   * @return set of introduced parameters and their ranges of values
   */
  parameter_set_t get_parameter_set(){return parameter_set_;}
  
  phase_result_const_sptr_t get_result_root(){return result_root_;}

protected:
  
  /**
   * シミュレーション時に使用される変数表のオリジナルの作成
   */
  virtual void init_variable_map(const parse_tree_sptr& parse_tree);

  /**
   * @return maximum module set without initial constraint
   */
  module_set_sptr get_max_ms_no_init() const;
  
  /**
   * push the initial state of simulation into the stack
   */
  virtual simulation_todo_sptr_t make_initial_todo();

  parse_tree_sptr parse_tree_;
  
  /**
   * シミュレーション中で使用される変数表の原型
   */
  variable_map_t variable_map_;
  
  /*
   * シミュレーション中に使用される変数の集合
   */
  variable_set_t variable_set_;

  /*
   * シミュレーション中に使用される記号定数とその値の集合
   */
  parameter_set_t parameter_set_;

  /**
   * 使用するPhaseSimulator
   */ 
  boost::shared_ptr<phase_simulator_t > phase_simulator_;

  /**
   * 解候補モジュール集合のコンテナ
   * （非always制約も含んでいるバージョン）
   */
  module_set_container_sptr msc_original_;
  /**
   * 解候補モジュール集合のコンテナ
   * （非always制約を除いたバージョン）
   */
  module_set_container_sptr msc_no_init_;

  /** 
   * 解軌道木の根．
   * 根自体は，解軌道のどの部分にも対応しない．
   * 単純に根としてのみ扱う．
   */
  phase_result_sptr_t result_root_;

  Opts*     opts_;


  private:

  void init_module_set_container(const parse_tree_sptr& parse_tree);
};

} //namespace simulator
} //namespace hydla 

#endif // _INCLUDED_HYDLA_SIMULATOR_H_
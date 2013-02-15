#ifndef _INCLUDED_HYDLA_SIMULATOR_H_
#define _INCLUDED_HYDLA_SIMULATOR_H_

#include <iostream>
#include <string>
#include <stack>
#include <cassert>


#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
//#include <boost/function.hpp>
#include <boost/iterator/indirect_iterator.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/xpressive/xpressive.hpp>

#include "Logger.h"

#include "Node.h"
#include "ParseTree.h"
#include "ModuleSetContainer.h"
#include "ModuleSetContainerCreator.h"
#include "ModuleSetGraph.h"
#include "ModuleSetList.h"


#include "AskDisjunctionSplitter.h"
#include "AskDisjunctionFormatter.h"

#include "PhaseResult.h"
#include "InitNodeRemover.h"
#include "TreeInfixPrinter.h"
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



typedef hydla::parse_tree::node_id_t                      node_id_t;
typedef boost::shared_ptr<hydla::ch::ModuleSet>           module_set_sptr;
typedef hydla::ch::ModuleSetContainer                     module_set_container_t;
typedef boost::shared_ptr<module_set_container_t>  module_set_container_sptr;
typedef hydla::ch::ModuleSetContainer::module_set_list_t  module_set_list_t;
typedef boost::shared_ptr<hydla::parse_tree::ParseTree>  parse_tree_sptr;
typedef boost::shared_ptr<const hydla::ch::ModuleSet>    module_set_const_sptr;
typedef boost::shared_ptr<hydla::ch::ModuleSetContainer> module_set_container_sptr;




typedef std::map<std::string, unsigned int> profile_t;




/**
 * シミュレーションすべきフェーズを表す構造体
 */
struct SimulationTodo{
  typedef std::map<hydla::ch::ModuleSet, std::map<DefaultVariable*, boost::shared_ptr<Value> , VariableComparator> > ms_cache_t;
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
}
}

namespace {
  using namespace hydla::ch;
  using namespace hydla::simulator;
  using namespace hydla::parse_tree;
  struct ModuleSetContainerInitializer {
    typedef boost::shared_ptr<hydla::parse_tree::ParseTree> parse_tree_sptr;
    typedef hydla::simulator::module_set_container_sptr         module_set_container_sptr;
    template<typename MSCC>
      static void init(
          const parse_tree_sptr& parse_tree,
          module_set_container_sptr& msc_original, 
          module_set_container_sptr& msc_no_init,
          parse_tree_sptr& member_parse_tree)
      {
        ModuleSetContainerCreator<MSCC> mcc;
        {
          parse_tree_sptr pt_original(boost::make_shared<ParseTree>(*parse_tree));
          AskDisjunctionFormatter().format(pt_original.get());
          AskDisjunctionSplitter().split(pt_original.get());
          msc_original = mcc.create(pt_original);
        }

        {
          parse_tree_sptr pt_no_init(boost::make_shared<ParseTree>(*parse_tree));
          InitNodeRemover().apply(pt_no_init.get());
          AskDisjunctionFormatter().format(pt_no_init.get());
          AskDisjunctionSplitter().split(pt_no_init.get());
          msc_no_init = mcc.create(pt_no_init);

          // 最適化された形のパースツリーを得る
          member_parse_tree = pt_no_init;
        }
      }
  };
}


namespace hydla {
namespace simulator {



class PhaseSimulator;


typedef PhaseResult                                       phase_result_t;
typedef boost::shared_ptr<const phase_result_t>           phase_result_const_sptr_t;
typedef PhaseSimulator                                    phase_simulator_t;

typedef SimulationTodo                                   simulation_phase_t;
typedef boost::shared_ptr<SimulationTodo>                simulation_phase_sptr_t;
typedef std::vector<simulation_phase_sptr_t>              simulation_phases_t;

typedef std::list<variable_t>                            variable_set_t;

struct ParameterAndRange
{
  parameter_t parameter;
  range_t     range;
  ParameterAndRange(const parameter_t& p, const range_t& r):parameter(p), range(r){}
};

typedef std::list<ParameterAndRange>                     parameter_set_t;
typedef value_t                                          time_value_t;

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
   * 与えられた解候補モジュール集合を元にシミュレーション実行をおこなう
   */
  virtual phase_result_const_sptr_t simulate() = 0;
  
  /**
   * 使用するPhaseSimulatorを設定する．
   * この関数に渡すPhaseSimulatorのインスタンスはnewで作成し，呼び出し側でdeleteしないようにする
   */
  void set_phase_simulator(phase_simulator_t *ps);
  
  virtual void initialize(const parse_tree_sptr& parse_tree);
  
  void init_module_set_container(const parse_tree_sptr& parse_tree);
  
  /**
   * シミュレーション時に使用される変数表のオリジナルの作成
   */
  virtual void init_variable_map(const parse_tree_sptr& parse_tree);
  
  /**
   * push the initial state of simulation into the stack
   */
  virtual void push_initial_state();
  
  /**
   * プロファイリングの結果を取得
   */
  entire_profile_t get_profile(){return profile_vector_;}
  
  /**
   * get set of introduced parameters and their values
   */
  parameter_set_t get_parameter_set(){return parameter_set_;}

protected:

  /**
   * 状態キューに新たな状態を追加する
   */
  virtual void push_simulation_phase(const simulation_phase_sptr_t& state)
  {
    state->phase_result->id = phase_id_++;
    state_stack_.push_front(state);
  }

  /**
   * 状態キューから状態をひとつ取り出す
   */
  simulation_phase_sptr_t pop_simulation_phase()
  {
    simulation_phase_sptr_t state;
    if(opts_->search_method == simulator::DFS){
      state = state_stack_.front();
      state_stack_.pop_front();
    }else{
      state = state_stack_.back();
      state_stack_.pop_back();
    }
    profile_vector_.push_back(state);
    // とりあえず，この関数で取りだしたものは必ずシミュレーションを行うことを前提にする．
    return state;
  }
  /**
   * @return maximum module set without initial constraint
   */
  module_set_sptr get_max_ms_no_init()const
  {
    return msc_no_init_->get_max_module_set();
  }
  
  
  
  /// 実際にシミュレータが閉包計算などを行ったフェーズの集合．主にプロファイリングのために取っておく．
  entire_profile_t profile_vector_;

  /**
   * シミュレーション対象となるパースツリー
   */
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
  int state_id_;

  /**
   * 使用するPhaseSimulator
   */ 
  boost::shared_ptr<phase_simulator_t > phase_simulator_;


  module_set_container_sptr msc_original_;
  module_set_container_sptr msc_no_init_;

  /**
   * 各状態を保存しておくためのスタック
   */
  std::deque<simulation_phase_sptr_t> state_stack_;
  
  parse_tree_sptr parse_tree;
  
  /// 解軌道木の根．初期状態なので，子供以外の情報は入れない
  phase_result_sptr_t result_root_;
  
  Opts*     opts_;
  /**
   * 各PhaseResultに振っていくID
   */
  int phase_id_;
};

} //namespace simulator
} //namespace hydla 

#endif // _INCLUDED_HYDLA_SIMULATOR_H_

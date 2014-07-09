#pragma once


#include <deque>

#include "Node.h"
#include "ParseTree.h"

#include "PhaseResult.h"
#include "Parameter.h"
#include "ConstraintStore.h"
#include "Opts.h"

namespace hydla {

namespace backend
{
  class Backend;
}

namespace interval
{
  class AffineApproximator;
}

namespace simulator {

class RelationGraph;

typedef boost::shared_ptr<backend::Backend> backend_sptr_t;

typedef hierarchy::ModuleSet                              module_set_t;
typedef hierarchy::ModuleSetContainer                     module_set_container_t;
typedef boost::shared_ptr<module_set_container_t>  module_set_container_sptr;
typedef std::set<module_set_t>                         module_set_set_t;
typedef boost::shared_ptr<parse_tree::ParseTree>  parse_tree_sptr;

typedef std::map<boost::shared_ptr<symbolic_expression::Ask>, bool> entailed_prev_map_t;
typedef std::vector<variable_map_t>      variable_maps_t;
typedef std::map<std::string, unsigned int> profile_t;

/**
 * シミュレーションすべきフェーズを表す構造体
 */
struct SimulationTodo{
  PhaseType                 phase_type;
  int                       id;
  value_t                   current_time;
  /// 左極限値のマップ
  variable_map_t            prev_map;
  parameter_map_t           parameter_map;
  ask_set_t           positive_asks;
  ask_set_t           negative_asks;
  std::map<ask_t, bool>     discrete_causes;
  ConstraintStore           expanded_constraints;
  ConstraintStore           current_constraints;   /// 現在のフェーズで有効な制約
  
  entailed_prev_map_t       judged_prev_map;

  /// 前のフェーズ
  phase_result_sptr_t parent;

  /// このフェーズの制約ストアの初期値（離散変化条件など，特に重要な条件を入れる）
  ConstraintStore initial_constraint_store;
  /// 未判定のモジュール集合を保持しておく．分岐処理時，同じ集合を複数回調べることが無いように
  /// TODO:現状，これがまともに使われていない気がする．つまり，何か間違っている可能性があるし，無駄は確実にある
  module_set_set_t ms_to_visit;
  /// 無矛盾極大なモジュール集合の集合
  module_set_set_t maximal_mss;
  /// プロファイリング結果
  profile_t profile;

  SimulationTodo(){}

  /**
   * parentとなるPhaseResultから情報を引き継いだTodoを作る。
   * prev_mapはこのコンストラクタで初期化されない。
   */
  SimulationTodo(const phase_result_sptr_t &parent_phase);

  inline bool in_following_step(){
    return parent.get() && parent->parent && parent->parent->parent;
  }
};

std::ostream& operator<<(std::ostream& s, const SimulationTodo& a);

typedef boost::shared_ptr<SimulationTodo>     simulation_todo_sptr_t;
// プロファイリング結果全体
// 各Todoごとにかかった時間（現状では，Todoそのものを保存している）
typedef std::vector<simulation_todo_sptr_t> entire_profile_t;

class PhaseSimulator;

typedef PhaseResult                                       phase_result_t;
typedef PhaseSimulator                                    phase_simulator_t;

typedef boost::shared_ptr<SimulationTodo>                simulation_todo_sptr_t;


class TodoContainer
{
  public:
  virtual ~TodoContainer(){}
  virtual void push_todo(simulation_todo_sptr_t& todo)
  {
    container_.push_back(todo);
  }
  
  virtual simulation_todo_sptr_t pop_todo()
  {
    simulation_todo_sptr_t todo = container_.back();
    container_.pop_back();
    return todo;
  }
  
  virtual bool empty()
  {
    return container_.empty();
  }
  
  virtual int size()
  {
    return container_.size();
  }
  
  protected:
  std::deque<simulation_todo_sptr_t> container_;
};

typedef TodoContainer                                    todo_container_t;

typedef std::set<variable_t, VariableComparator>                            variable_set_t;

class Simulator
{
public:  

  Simulator(Opts& opts);
  
  virtual ~Simulator();

  /**
   * simulate using given parse_tree
   * @return root of the tree which expresses the result-trajectories
   */
  virtual phase_result_const_sptr_t simulate() = 0;
  
  virtual void initialize(const parse_tree_sptr& parse_tree);
  
  /**
   * set the phase simulator which this simulator uses in each phase
   * @param ps a pointer to an instance of phase_simlulator_t (caller must not delete the instance)
   */
  virtual void set_phase_simulator(phase_simulator_t *ps);
  
  void set_backend(backend_sptr_t back);

  /**
   * @return set of introduced parameters and their ranges of values
   */
  parameter_map_t get_parameter_map()const {return parameter_map_;}

  variable_set_t get_variable_set()const {return variable_set_;}
  
  phase_result_sptr_t get_result_root() const {return result_root_;}
  
  /**
   * push the initial state of simulation into the stack
   */
  virtual simulation_todo_sptr_t make_initial_todo();
  
  /**
   * @return introduced parameter
   */
  parameter_t introduce_parameter(const variable_t &var, const PhaseResult& phase, const ValueRange &range);
  parameter_t introduce_parameter(const std::string &name, int differential_cnt, int id, const ValueRange &range);
  parameter_t introduce_parameter(const parameter_t &par, const ValueRange &range);
  
  /**
   * @return the result of profiling 
   */
  entire_profile_t get_profile(){return *profile_vector_;}


  // TODO: publicメンバが多すぎる気がする
  
  /**
   * template of variable maps
   */
  variable_map_t original_map_;
  
  /*
   * set of variables
   */
  variable_set_t variable_set_;
  
  variable_t system_time_;

  /*
   * map of introduced parameters and their ranges of values
   */
  parameter_map_t parameter_map_;

  backend_sptr_t backend;

  boost::shared_ptr<phase_simulator_t > phase_simulator_;


protected:
  
  /**
   * シミュレーション時に使用される変数表のオリジナルの作成
   */
  virtual void init_variable_map(const parse_tree_sptr& parse_tree);
  
  void init_module_set_container(const parse_tree_sptr& parse_tree);
  
  void reset_result_root();

  parse_tree_sptr parse_tree_;  

  /**
   * a container for candidate module sets
   */
  module_set_container_sptr module_set_container_;
  
  /**
   * vector for results of profiling
   */
  boost::shared_ptr<entire_profile_t> profile_vector_;

  /** 
   * root of the tree of result trajectories
   */
  phase_result_sptr_t result_root_;

  Opts*     opts_;

  interval::AffineApproximator* affine_transformer_;
};

} //namespace simulator
} //namespace hydla 


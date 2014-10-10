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

typedef boost::shared_ptr<backend::Backend>       backend_sptr_t;
typedef hierarchy::ModuleSet                      module_set_t;
typedef hierarchy::ModuleSetContainer             module_set_container_t;
typedef boost::shared_ptr<module_set_container_t> module_set_container_sptr;
typedef std::set<module_set_t>                    module_set_set_t;
typedef boost::shared_ptr<parse_tree::ParseTree>  parse_tree_sptr;
typedef std::map<boost::shared_ptr<symbolic_expression::Ask>, bool>
                                                  entailed_prev_map_t;
typedef std::vector<variable_map_t>               variable_maps_t;
typedef std::map<std::string, unsigned int>       profile_t;


/**
 * シミュレーションにおいて必要な仕事を表す構造体
 */
struct SimulationJob{
  PhaseType                 phase_type;
  int                       id;
  /// 左極限値のマップ
  variable_map_t            prev_map;
  parameter_map_t           parameter_map;
  ask_set_t                 positive_asks;
  ask_set_t                 negative_asks;
  
  constraint_diff_t         expanded_diff;
  module_diff_t             adopted_module_diff;
  
  std::map<ask_t, bool>    discrete_positive_asks, discrete_negative_asks;
  next_pp_candidate_map_t   next_pp_candidate_map; 

  ConstraintStore           expanded_constraints;
  ConstraintStore           initial_constraint_store; /// 暫定的に場合分けとかで使う．TODO:別の方法を考える
  ConstraintStore           current_constraints;   /// 現在のフェーズで有効な制約．TODO:expanded_constraintsとの役割分担について考える．
  
  entailed_prev_map_t       judged_prev_map;

  phase_result_sptr_t owner;

  phase_result_sptrs_t produced_phases;

  /// 未判定のモジュール集合を保持しておく．分岐処理時，同じ集合を複数回調べることが無いように
  module_set_set_t ms_to_visit;
  /// set of module sets which are unadopted in maximal module sets
  module_set_set_t unadopted_mss;
  /// プロファイリング結果
  profile_t profile;

  SimulationJob():id(-1){}

  /**
   * parentとなるPhaseResultから情報を引き継いだJobを作る。
   */
  SimulationJob(const phase_result_sptr_t &parent_phase);

  inline bool in_following_step(){
    return owner && owner->parent && owner->parent->parent;
  }
};

std::ostream& operator<<(std::ostream& s, const SimulationJob& a);

// プロファイリング結果全体
// 各Jobごとにかかった時間（現状では，Jobそのものを保存している）
typedef std::vector<simulation_job_sptr_t> entire_profile_t;

class PhaseSimulator;

typedef PhaseResult                                       phase_result_t;
typedef PhaseSimulator                                    phase_simulator_t;

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
  virtual phase_result_sptr_t simulate() = 0;
  
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
   *  the initial state of simulation into the stack
   */
  virtual simulation_job_sptr_t make_initial_todo();

  virtual simulation_job_sptr_t make_new_todo(phase_result_sptr_t parent);
  
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

  virtual void process_one_todo(simulation_job_sptr_t& todo);  
  
  void reset_result_root();

  parse_tree_sptr parse_tree_;  

  /// container for candidate module sets
  module_set_container_sptr module_set_container_;
  
  /// vector for results of profiling
  boost::shared_ptr<entire_profile_t> profile_vector_;

  /// root of the tree of result trajectories
  phase_result_sptr_t result_root_;

  Opts*     opts_;

  interval::AffineApproximator* affine_transformer_;
};

} //namespace simulator
} //namespace hydla 


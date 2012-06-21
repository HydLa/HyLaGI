#ifndef _INCLUDED_SYMBOLIC_SIMULATOR_H_
#define _INCLUDED_SYMBOLIC_SIMULATOR_H_

#include <string>
#include <iostream>
#include <fstream>

#include <sstream>
#include <stack>

#include "ParseTree.h"

#include "Simulator.h"

#include <boost/scoped_ptr.hpp>

#include "Types.h"
#include "SymbolicTypes.h"
#include "../virtual_constraint_solver/SymbolicVirtualConstraintSolver.h"

namespace hydla {
namespace symbolic_simulator {


class SymbolicSimulator : public simulator_t
{
public:

  typedef hydla::vcs::SymbolicVirtualConstraintSolver solver_t;


  SymbolicSimulator(const Opts& opts);
  virtual ~SymbolicSimulator();

  
  /**
   * 与えられた解候補モジュール集合を元にシミュレーション実行をおこなう
   */
  virtual void simulate();

  /**
   * Point Phaseの処理
   */
  virtual bool point_phase(const module_set_sptr& ms, 
                           phase_state_sptr& state);
  
  /**
   * Interval Phaseの処理
   */
  virtual bool interval_phase(const module_set_sptr& ms, 
                              phase_state_sptr& state);


private:
  /**
   * 初期化処理
   */
  virtual void do_initialize(const parse_tree_sptr& parse_tree);

  variable_map_t range_map_to_value_map(const phase_state_sptr&, const hydla::vcs::SymbolicVirtualConstraintSolver::variable_range_map_t &, parameter_map_t &);

  variable_t* get_variable(const std::string &name, const int &derivative_count){
    return &(*std::find(variable_set_.begin(), variable_set_.end(), (variable_t(name, derivative_count))));
  }
  
  variable_map_t shift_variable_map_time(const variable_map_t& vm,const time_t &time);

  void init_module_set_container(const parse_tree_sptr& parse_tree);
  
  CalculateClosureResult calculate_closure(phase_state_sptr& state,
                        const module_set_sptr& ms, expanded_always_t &expanded_always,
                         positive_asks_t &positive_asks, negative_asks_t &negative_asks,
                         const variable_map_t& variable_map);



  void print_pp(std::vector<std::string> v, variable_map_t v_map,
    int step, std::string ms_set_name, time_t time, std::string _case);
  
  void print_ip(std::vector<std::string> v, variable_map_t v_map,
    int step, std::string ms_set_name, time_t start_time, time_t end_time, std::string _case);

  void output_parameter_map(const parameter_map_t& parameter_map);

  void add_continuity(const continuity_map_t&);
  
  void output_variable_map(std::ostream &, const variable_map_t& variable_map, const time_t& time,  const bool& numeric);
  
  std::string get_state_output(const phase_state_t& result, const bool& numeric, const bool& is_in_progress);
  void output_variable_labels(std::ostream &, const variable_map_t variable_map);

  
  /// 解軌道木を出力する
  void const output_result_tree();
  void const output_result_tree_mathematica();
  void output_result_node(const phase_state_sptr_t &, std::vector<std::string> &, int &,int &);
  
  /**
   * 記号定数の条件によって分岐する際，それぞれの場合をスタックに入れる
   * ただし，true_parameter_mapsの第一要素のみ除く
   */
  void push_branch_states(phase_state_sptr &original, vcs::SymbolicVirtualConstraintSolver::check_consistency_result_t &);

  continuity_map_t variable_derivative_map_;
  module_set_container_sptr msc_original_;

  module_set_container_sptr msc_no_init_;

  Opts     opts_;

  //interactive mode用
  std::vector<phase_state_sptr> all_state;
  bool change_variable_flag;

  /// 使用するソルバへのポインタ
  boost::scoped_ptr<solver_t> solver_;
  bool is_safe_; 
  
  ///解軌道木の根．初期状態なので，子供以外の情報は入れない
  phase_state_sptr_t result_root_;
};

} // namespace symbolic_simulator
} // namespace hydla

#endif //_INCLUDED_SYMBOLIC_SIMULATOR_H_

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

using namespace hydla::simulator;

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
                           const phase_state_const_sptr& state);
  
  /**
   * Interval Phaseの処理
   */
  virtual bool interval_phase(const module_set_sptr& ms, 
                              const phase_state_const_sptr& state);


private:
  /**
   * 初期化処理
   */
  virtual void do_initialize(const parse_tree_sptr& parse_tree);
  
  variable_map_t shift_variable_map_time(const variable_map_t& vm,const time_t &time);

  void init_module_set_container(const parse_tree_sptr& parse_tree);
  
  CalculateClosureResult calculate_closure(const phase_state_const_sptr& state,
                        const module_set_sptr& ms, expanded_always_t &expanded_always,
                         positive_asks_t &positive_asks, negative_asks_t &negative_asks);

  void output(const time_t& time, 
              const variable_map_t& vm);
  
  void output(const time_t& time, 
              const variable_map_t& vm,
              const parameter_map_t& pm);
  

  void output_interval(const time_t& current_time, const time_t& limit_time,
                     const variable_map_t& variable_map, const parameter_map_t& parameter_map);

  void output_point(const time_t& time, const variable_map_t& variable_map, const parameter_map_t& parameter_map);
  
  void output_parameter_map(const parameter_map_t& parameter_map);
  
  //現状では1回出力すると，木の根以外の部分が無くなるようになっているから注意
  void output_result_tree();

                              
  module_set_container_sptr msc_original_;

  module_set_container_sptr msc_no_init_;
  module_set_container_sptr msc_no_init_discreteask_;

  Opts     opts_;
  
  
  virtual void push_phase_state(const phase_state_sptr& state) 
  {
    state_stack_.push(state);
  }
  
  std::string range_to_string(const value_range_t& val);


  boost::scoped_ptr<solver_t> solver_;               //使用するソルバ

  state_result_sptr_t result_root_;                  //結果を示す解軌道木の根．初期状態なので，子供以外の情報は入れない
};

} // namespace symbolic_simulator
} // namespace hydla

#endif //_INCLUDED_SYMBOLIC_SIMULATOR_H_

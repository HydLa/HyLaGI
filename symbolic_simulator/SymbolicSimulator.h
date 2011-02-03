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


  /*
  * 与えられた解候補モジュール集合のリストをreduceのためにファイル出力する関数。initialize前で分岐
  */
    virtual bool reduce_simulate();

  /*
  * reduce出力用関数。現在はpoint_phase関数から分岐
  * svn Rev: 1062
  */
    virtual bool reduce_output(const module_set_sptr& ms, 
                             const phase_state_const_sptr& state);
  
  /*
  * dispatch関数に渡してModuleSetのpair<name,node>を取得
  */

  virtual bool reduce_simulate_phase_state(const module_set_sptr& ms);

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
                     const variable_map_t& variable_map);

  void output_point(const time_t& time, const variable_map_t& variable_map, const parameter_map_t& parameter_map);
              
                              
  module_set_container_sptr msc_original_;

  module_set_container_sptr msc_no_init_;
  module_set_container_sptr msc_no_init_discreteask_;

  Opts     opts_;
  
  
  virtual void push_phase_state(const phase_state_sptr& state) 
  {
    branch_++;
    state_stack_.push(state);
  }
  
  std::string value_to_string(const value_t& val);


  boost::scoped_ptr<solver_t> solver_;               //使用するソルバ
  
  std::vector<std::string> output_vector_;           //全解探索で，全経路出力を行うための配列
  std::stack<int> branch_stack_;                     //直前に分岐した数を記憶するためのスタック
  int branch_;                                       //今フェーズで何状態に分岐したかを示す変数
  std::ostringstream output_buffer_;                 //スタックに入れるための，直前の分岐から現在までの出力保持
};

} // namespace symbolic_simulator
} // namespace hydla

#endif //_INCLUDED_SYMBOLIC_SIMULATOR_H_

#ifndef _INCLUDED_HYDLA_SEQUENTIAL_SIMULATOR_H_
#define _INCLUDED_HYDLA_SEQUENTIAL_SIMULATOR_H_

#include "Simulator.h"

namespace hydla {
namespace simulator {

class SequentialSimulator: public Simulator{
public:
  SequentialSimulator(Opts &opts);
  
  virtual ~SequentialSimulator();
  /**
   * 与えられた解候補モジュール集合を元にシミュレーション実行をおこなう
   */
  virtual phase_result_const_sptr_t simulate();
  
  
  /**
   * @return the result of profiling
   */
  entire_profile_t get_profile(){return profile_vector_;}

protected:

  /**
   * Todoキューに新たなTodoを追加する
   * TODO: この関数が，PhaseSimulator内部から呼ばれるようにする（TodoのIDの整合性のため）
   */
  virtual void push_simulation_todo(const simulation_todo_sptr_t& todo);

  /**
   * 状態キューから状態をひとつ取り出す
   * この関数で取りだしたsimulation_todo_sptr_tについては必ずシミュレーションを行うことを前提としている．
   * (プロファイリング結果の整合性のため）
   */
  simulation_todo_sptr_t pop_simulation_phase();
  
  /**
   * シミュレーション上のTodoを入れておくコンテナ
   */
  todo_container_t todo_stack_;
  
  /**
   * 各Todoに振っていくID
   */
  int todo_id_;

  /**
   * 各Todoに対応するプロファイリングの結果
   */
  entire_profile_t profile_vector_;
};

} // simulator
} // hydla

#endif // _INCLUDED_HYDLA_SEQUENTIAL_SIMULATOR_H_
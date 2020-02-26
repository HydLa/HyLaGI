#pragma once

#include "Simulator.h"
#include "SymbolicTrajPrinter.h"

namespace hydla {
namespace simulator {

class SequentialSimulator : public Simulator {
public:
  SequentialSimulator(Opts &opts);
  virtual ~SequentialSimulator();
  /**
   * 与えられた解候補モジュール集合を元にシミュレーション実行をおこなう
   */
  virtual phase_result_sptr_t simulate();

private:
  void dfs(phase_result_sptr_t current);
  void omit_following_todos(phase_result_sptr_t current);
  io::SymbolicTrajPrinter printer;
};

} // namespace simulator
} // namespace hydla

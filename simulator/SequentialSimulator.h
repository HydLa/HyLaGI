#ifndef _INCLUDED_HYDLA_SEQUENTIAL_SIMULATOR_H_
#define _INCLUDED_HYDLA_SEQUENTIAL_SIMULATOR_H_

#include "BatchSimulator.h"

namespace hydla {
namespace simulator {

class SequentialSimulator: public BatchSimulator{
public:
  SequentialSimulator(Opts &opts);
  
  virtual ~SequentialSimulator();
  /**
   * 与えられた解候補モジュール集合を元にシミュレーション実行をおこなう
   */
  virtual phase_result_const_sptr_t simulate();

protected:
};

} // simulator
} // hydla

#endif // _INCLUDED_HYDLA_SEQUENTIAL_SIMULATOR_H_
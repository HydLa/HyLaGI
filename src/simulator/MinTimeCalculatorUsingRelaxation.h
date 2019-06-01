#pragma once

#include "RelationGraph.h"
#include "PhaseResult.h"
#include "GuardVisitor.h"
#include "Backend.h"

namespace hydla {
namespace simulator {

/**
 * A class to calculate minimum times for asks
 */
class MinTimeCalculatorUsingRelaxation
{
public:
  find_min_time_result_t calculate_min_time();

  MinTimeCalculatorUsingRelaxation();
  ~MinTimeCalculatorUsingRelaxation();
  MinTimeCalculatorUsingRelaxation(backend::Backend *b, std::vector<ask_t> all_asks);
  std::pair<find_min_time_result_t, ask_t> find_min_time_using_relaxation(variable_map_t vm, value_t time_limit);

private:
  backend::Backend *backend_;
  std::vector<ConstraintStore> guards_;
  std::vector<ask_t> all_asks_;
};

} // namespace simulator
} // namespace hydla

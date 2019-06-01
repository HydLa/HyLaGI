#include "MinTimeCalculatorUsingRelaxation.h"
#include "RelationGraph.h"

namespace hydla
{
namespace simulator
{

using namespace backend;

MinTimeCalculatorUsingRelaxation::MinTimeCalculatorUsingRelaxation(){};
MinTimeCalculatorUsingRelaxation::~MinTimeCalculatorUsingRelaxation(){};
MinTimeCalculatorUsingRelaxation::MinTimeCalculatorUsingRelaxation(Backend *b, std::vector<ask_t> all_asks): backend_(b), all_asks_(all_asks)
{
  std::vector<ConstraintStore> guards;
  for (auto ask : all_asks) {
    std::cout << get_infix_string(ask) << std::endl;
    guards.push_back(ask->get_guard());
  }
  b->call("calculateRelaxedGuardsInit", true, 1, "cscl", "", &guards);
}
std::pair<find_min_time_result_t, ask_t> MinTimeCalculatorUsingRelaxation::find_min_time_using_relaxation(variable_map_t vm, value_t time_limit) {

  find_min_time_result_t candidates;
  backend_->call("findMinTimeWithRelaxation", true, 2, "mvtvlt", "f", &vm, &time_limit, &candidates);
  std::pair<find_min_time_result_t, ask_t> ret;
  ret.first = candidates;
  ret.second = all_asks_[candidates.front().guard_indices.front()-1];
  return ret;
}
}
}

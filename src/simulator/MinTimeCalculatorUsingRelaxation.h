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
class MinTimeCalculatorUsingRelaxation: public GuardVisitor
{
public:
  find_min_time_result_t calculate_min_time();

  MinTimeCalculatorUsingRelaxation();
  ~MinTimeCalculatorUsingRelaxation();
  MinTimeCalculatorUsingRelaxation(backend::Backend *b, ConstraintStore guards);

  virtual void visit(AtomicGuardNode *atomic);
  virtual void visit(AndGuardNode *and_node);
  virtual void visit(OrGuardNode *or_node);
  virtual void visit(NotGuardNode *not_node);

private:
  std::set<ConstraintStore> positive_guards;
  backend::Backend *backend;
  guard_time_map_t *guard_time_map;
  constraint_t current_cons;
};

} // namespace simulator
} // namespace hydla

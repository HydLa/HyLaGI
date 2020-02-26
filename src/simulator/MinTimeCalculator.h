#pragma once

#include "Backend.h"
#include "GuardVisitor.h"
#include "PhaseResult.h"
#include "RelationGraph.h"

namespace hydla {
namespace simulator {

/**
 * A class to calculate minimum times for asks
 */
class MinTimeCalculator : public GuardVisitor {
public:
  find_min_time_result_t
  calculate_min_time(guard_time_map_t *g, const constraint_t &guard,
                     bool negated, value_t time_limit,
                     const constraint_t &time_bound = constraint_t(),
                     value_t starting_time = Value());

  MinTimeCalculator(RelationGraph *relation_graph, backend::Backend *b);

  virtual void visit(AtomicGuardNode *atomic);
  virtual void visit(AndGuardNode *and_node);
  virtual void visit(OrGuardNode *or_node);
  virtual void visit(NotGuardNode *not_node);

private:
  RelationGraph *relation_graph;
  guard_time_map_t *guard_time_map;
  constraint_t current_cons;
  backend::Backend *backend;
};

} // namespace simulator
} // namespace hydla

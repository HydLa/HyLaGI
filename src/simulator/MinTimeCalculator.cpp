#include "MinTimeCalculator.h"

namespace hydla {
namespace simulator {

using namespace backend;

MinTimeCalculator::MinTimeCalculator(RelationGraph *rel, Backend *b)
    : relation_graph(rel), backend(b) {}
find_min_time_result_t MinTimeCalculator::calculate_min_time(
    guard_time_map_t *g, const constraint_t &guard, bool negated,
    value_t time_limit, const constraint_t &time_bound, value_t starting_time) {
  guard_time_map = g;
  GuardNode *node = relation_graph->get_guard_node(guard);
  node->accept(this);
  if (negated)
    current_cons.reset(new symbolic_expression::Not(current_cons));

  if (time_bound.get() != nullptr)
    current_cons.reset(
        new symbolic_expression::LogicalAnd(current_cons, time_bound));

  find_min_time_result_t find_min_time_result;

  if (starting_time.undefined()) {
    backend->call("minimizeTime", true, 2, "envlt", "f", &current_cons,
                  &time_limit, &find_min_time_result);
  } else {
    backend->call("minimizeTime", true, 3, "envltvlt", "f", &current_cons,
                  &starting_time, &time_limit, &find_min_time_result);
  }
  return find_min_time_result;
}

void MinTimeCalculator::visit(AtomicGuardNode *atomic) {
  current_cons = (*guard_time_map)[atomic->atomic_guard.constraint];
}

void MinTimeCalculator::visit(OrGuardNode *or_node) {
  or_node->lhs->accept(this);
  constraint_t lhs = current_cons;
  or_node->rhs->accept(this);
  constraint_t rhs = current_cons;
  current_cons.reset(new symbolic_expression::LogicalOr(lhs, rhs));
}

void MinTimeCalculator::visit(AndGuardNode *and_node) {
  and_node->lhs->accept(this);
  constraint_t lhs = current_cons;
  and_node->rhs->accept(this);
  constraint_t rhs = current_cons;
  current_cons.reset(new symbolic_expression::LogicalAnd(lhs, rhs));
}

void MinTimeCalculator::visit(NotGuardNode *not_node) {
  not_node->child->accept(this);
  current_cons.reset(new symbolic_expression::Not(current_cons));
}

} // namespace simulator
} // namespace hydla

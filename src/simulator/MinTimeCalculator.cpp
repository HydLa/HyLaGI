#include "MinTimeCalculator.h"

namespace hydla
{
namespace simulator
{

using namespace backend;

MinTimeCalculator::MinTimeCalculator(RelationGraph *rel, Backend *b): relation_graph(rel), backend(b)
{
}

find_min_time_result_t MinTimeCalculator::calculate_min_time(guard_time_map_t *g, const ask_t &ask_to_be_updated, bool negated)
{
  // TODO: All guards don't have to be updated.
  // TODO: 原子制約単位ではなく，OrGuardNodeやAndGuardNode単位でも結果の再利用は可能なはず
  guard_time_map = g;
  AskNode *ask_node = relation_graph->get_ask_node(ask_to_be_updated);
  GuardNode *node = ask_node->guard_node;

  if(negated) node = (new NotGuardNode(node));

  node->accept(this);


  find_min_time_result_t find_min_time_result;
  backend->call("minimizeTime", 1, "en", "f",
                &current_cons, &find_min_time_result);

  return find_min_time_result;
}

void MinTimeCalculator::visit(AtomicGuardNode *atomic)
{
  current_cons = (*guard_time_map)[atomic->atomic_guard.constraint];
}

void MinTimeCalculator::visit(OrGuardNode *or_node)
{
  or_node->lhs->accept(this);
  constraint_t lhs = current_cons;
  or_node->rhs->accept(this);
  constraint_t rhs = current_cons;
  current_cons.reset(new symbolic_expression::LogicalOr(lhs, rhs));
}

void MinTimeCalculator::visit(AndGuardNode *and_node)
{
  and_node->lhs->accept(this);
  constraint_t lhs = current_cons;
  and_node->rhs->accept(this);
  constraint_t rhs = current_cons;
  current_cons.reset(new symbolic_expression::LogicalAnd(lhs, rhs));
}


void MinTimeCalculator::visit(NotGuardNode *not_node)
{
  not_node->child->accept(this);
  current_cons.reset(new symbolic_expression::Not(current_cons));
}


}
}

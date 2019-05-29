#include "MinTimeCalculatorUsingRelaxation.h"
#include "RelationGraph.h"

namespace hydla
{
namespace simulator
{

using namespace backend;

MinTimeCalculatorUsingRelaxation::MinTimeCalculatorUsingRelaxation(){};
MinTimeCalculatorUsingRelaxation::~MinTimeCalculatorUsingRelaxation(){};
MinTimeCalculatorUsingRelaxation::MinTimeCalculatorUsingRelaxation(Backend *b, ConstraintStore guards): backend(b)
{
  b->call("initEndPoints", true, 1, "cs", "", &guards, "");
}
/*
find_min_time_result_t MinTimeCalculatorUsingRelaxation::calculate_min_time(guard_time_map_t *g, const constraint_t &guard, bool negated, value_t time_limit, const constraint_t &time_bound, value_t starting_time)
{
  guard_time_map = g;
  GuardNode *node = relation_graph->get_guard_node(guard);
  node->accept(this);
  if(negated)current_cons.reset(new symbolic_expression::Not(current_cons));

  if(time_bound.get() != nullptr)current_cons.reset(new symbolic_expression::LogicalAnd(current_cons, time_bound));

  find_min_time_result_t find_min_time_result;

  if(starting_time.undefined()){
    backend->call("minimizeTime", true, 2, "envlt", "f",
                  &current_cons, &time_limit, &find_min_time_result);
  }else{
    backend->call("minimizeTime", true, 3, "envltvlt", "f",
                  &current_cons, &starting_time, &time_limit, &find_min_time_result);
  }
  return find_min_time_result;
}
*/

void MinTimeCalculatorUsingRelaxation::visit(AtomicGuardNode *atomic)
{
  current_cons = (*guard_time_map)[atomic->atomic_guard.constraint];
}

void MinTimeCalculatorUsingRelaxation::visit(OrGuardNode *or_node)
{
  or_node->lhs->accept(this);
  constraint_t lhs = current_cons;
  or_node->rhs->accept(this);
  constraint_t rhs = current_cons;
  current_cons.reset(new symbolic_expression::LogicalOr(lhs, rhs));
}

void MinTimeCalculatorUsingRelaxation::visit(AndGuardNode *and_node)
{
  and_node->lhs->accept(this);
  constraint_t lhs = current_cons;
  and_node->rhs->accept(this);
  constraint_t rhs = current_cons;
  current_cons.reset(new symbolic_expression::LogicalAnd(lhs, rhs));
}


void MinTimeCalculatorUsingRelaxation::visit(NotGuardNode *not_node)
{
  not_node->child->accept(this);
  current_cons.reset(new symbolic_expression::Not(current_cons));
}


}
}

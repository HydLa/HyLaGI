#include "GuardMapApplier.h"
#include "HydLaError.h"
#include "Logger.h"

using namespace std;

namespace hydla
{
namespace simulator
{

using namespace symbolic_expression;

GuardMapApplier::GuardMapApplier()
{
}

GuardMapApplier::~GuardMapApplier()
{
}
  
constraint_t GuardMapApplier::apply(constraint_t guard, const map<constraint_t, bool> *map)
{
  atomic_guards_map = map;
  accept(guard);
  return applied_node;
}

void GuardMapApplier::visit_atomic_constraint(std::shared_ptr<Node> node)
{
  auto it = atomic_guards_map->find(node);
  HYDLA_LOGGER_DEBUG_VAR(get_infix_string(node));
  HYDLA_ASSERT(it != atomic_guards_map->end());
  if(it->second)applied_node = constraint_t(new True());
  else applied_node = constraint_t(new False());
}


void GuardMapApplier::visit(std::shared_ptr<LogicalAnd> node)
{
  accept(node->get_lhs());
  constraint_t lhs_node = applied_node;
  accept(node->get_rhs());
  applied_node = constraint_t(new LogicalAnd(lhs_node, applied_node));
}

void GuardMapApplier::visit(std::shared_ptr<LogicalOr> node)
{
  accept(node->get_lhs());
  constraint_t lhs_node = applied_node;
  accept(node->get_rhs());
  applied_node = constraint_t(new LogicalOr(lhs_node, applied_node));
}

void GuardMapApplier::visit(std::shared_ptr<Not> node)
{
  accept(node->get_child());
  applied_node = constraint_t(new Not(applied_node));
}

void GuardMapApplier::visit(std::shared_ptr<False> node)
{
  applied_node = constraint_t(new False());
}
void GuardMapApplier::visit(std::shared_ptr<True> node)
{
  applied_node = constraint_t(new True());
}



}
}

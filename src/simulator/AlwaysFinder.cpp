#include "AlwaysFinder.h"


using namespace std;

namespace hydla
{
namespace simulator
{

AlwaysFinder::AlwaysFinder()
{
}

AlwaysFinder::~AlwaysFinder()
{
}
  
void AlwaysFinder::find_always(symbolic_expression::node_sptr node, always_set_t* al, ConstraintStore *non_al, asks_t *pos, asks_t *nonal_asks)
{
  always_set = al;
  non_always = non_al;
  positive_asks = pos;
  nonalways_asks = nonal_asks;
  accept(node);
}

void AlwaysFinder::visit(std::shared_ptr<symbolic_expression::Ask> node)
{
  // visit consequents of positive_asks only
  nonalways_asks->insert(node);
  if(positive_asks->count(node) > 0)accept(node->get_child());
}
  
void AlwaysFinder::visit(std::shared_ptr<symbolic_expression::Always> node)
{
  always_set->insert(node);
}

void AlwaysFinder::visit_atomic_constraint(std::shared_ptr<symbolic_expression::Node> node)
{
  if(non_always != nullptr) non_always->add_constraint(node);
}

}
}

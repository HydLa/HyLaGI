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
  
void AlwaysFinder::find_always(symbolic_expression::node_sptr node, always_set_t* al, ConstraintStore *non_al)
{
  always_set = al;
  non_always = non_al;
  accept(node);
}

void AlwaysFinder::visit(boost::shared_ptr<symbolic_expression::Ask> node)
{
  // do nothing
}
  
void AlwaysFinder::visit(boost::shared_ptr<symbolic_expression::Always> node)
{
  always_set->insert(node);
}

void AlwaysFinder::visit_atomic_constraint(boost::shared_ptr<symbolic_expression::Node> node)
{
  if(non_always != nullptr) non_always->add_constraint(node);
}

}
}

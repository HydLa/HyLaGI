#include "ConstraintStoreBuilderPoint.h"
#include <iostream>

using namespace hydla::parse_tree;
using namespace hydla::simulator;

namespace hydla {
namespace symbolic_simulator {


ConstraintStoreBuilderPoint::ConstraintStoreBuilderPoint()
{
  constraint_store_.str = "{}";
}

ConstraintStoreBuilderPoint::~ConstraintStoreBuilderPoint()
{}

void ConstraintStoreBuilderPoint::build_constraint_store( /*variable_map_t variable_map */ )
{
  /* this->constraint_store_ = variable_map */ ;
}

void ConstraintStoreBuilderPoint::build_variable_map(variable_map_t variable_map)
{
  /*variable_map = this->constraint_store_*/;
}

ConstraintStore& ConstraintStoreBuilderPoint::getcs()
{
  return this->constraint_store_;
}

} //namespace symbolic_simulator
} // namespace hydla

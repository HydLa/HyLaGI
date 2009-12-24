#include "ConstraintStoreBuilderPoint.h"

namespace hydla {
namespace bp_simulator {

ConstraintStoreBuilderPoint::ConstraintStoreBuilderPoint()
{}

ConstraintStoreBuilderPoint::~ConstraintStoreBuilderPoint()
{}

void ConstraintStoreBuilderPoint::build_constraint_store( /*variable_map_t variable_map*/ )
{}

ConstraintStore& ConstraintStoreBuilderPoint::getcs()
{
  return this->constraint_store_;
}

} // namespace bp_simulator
} // namespace hydla

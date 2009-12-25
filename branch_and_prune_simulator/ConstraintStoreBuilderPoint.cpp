#include "ConstraintStoreBuilderPoint.h"

using namespace hydla::simulator;

namespace hydla {
namespace bp_simulator {

ConstraintStoreBuilderPoint::ConstraintStoreBuilderPoint(bool debug_mode) :
  constraint_store_(),
  debug_mode_(debug_mode)
{}

ConstraintStoreBuilderPoint::~ConstraintStoreBuilderPoint()
{}

void ConstraintStoreBuilderPoint::build_constraint_store(const variable_map_t& variable_map)
{}

ConstraintStore& ConstraintStoreBuilderPoint::getcs()
{
  return this->constraint_store_;
}

} // namespace bp_simulator
} // namespace hydla

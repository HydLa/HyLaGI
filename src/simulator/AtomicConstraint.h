#pragma once

#include "Node.h"

namespace hydla {
namespace simulator {

typedef symbolic_expression::node_sptr constraint_t;

/// Constants to indicate state of each atomic constraint
enum ConsistencyState
{
  CONSISTENT,
  INCONSISTENT,
  UNKNOWN
};

/// Constants to indicate where f(X) is
enum ConstraintLocation
{
  POSITIVE_AREA,
  NEGATIVE_AREA,
  ZERO_AREA,
  UNKNOWN_AREA
};

class AtomicConstraint
{
public:
  constraint_t constraint;
  ConsistencyState consistency_state;
  ConstraintLocation constraint_location;
};

} // namespace simulator
} // namespace hydla

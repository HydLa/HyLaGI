#pragma once

#include "Node.h"
#include "TreeVisitorForAtomicConstraint.h"
#include "PhaseResult.h"

namespace hydla {
namespace simulator {

class GuardMapApplier : public symbolic_expression::TreeVisitorForAtomicConstraint {
public:

  GuardChecker();

  virtual ~GuardChecker();

  /**
   * substitute true or false for each atomic_guards in guard
   */
  constraint_t check_guard(constraint_t guard, const std::map<constraint_t, bool> *map);

  virtual void visit_atomic_constraint(boost::shared_ptr<symbolic_expression::Node> node);
private:
  const std::map<constraint_t, bool> *atomic_guards_map;
};

} //namespace simulator
} //namespace hydla 

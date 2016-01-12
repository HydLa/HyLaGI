#pragma once

#include "Node.h"
#include "TreeVisitorForAtomicConstraint.h"
#include "PhaseResult.h"

namespace hydla {
namespace simulator {

class GuardMapApplier : public symbolic_expression::TreeVisitorForAtomicConstraint {
public:

  GuardMapApplier();

  virtual ~GuardMapApplier();

  /**
   * substitute true or false for each atomic_guards in guard
   */
  constraint_t apply(constraint_t guard, const std::map<constraint_t, bool> *map);

  
  virtual void visit_atomic_constraint(boost::shared_ptr<symbolic_expression::Node> node);

  using TreeVisitorForAtomicConstraint::visit; // suppress warnings  
  virtual void visit(boost::shared_ptr<symbolic_expression::LogicalAnd> node);
  virtual void visit(boost::shared_ptr<symbolic_expression::LogicalOr> node);
  virtual void visit(boost::shared_ptr<symbolic_expression::Not> node);
  virtual void visit(boost::shared_ptr<symbolic_expression::False> node);
  virtual void visit(boost::shared_ptr<symbolic_expression::True> node);

private:
  const std::map<constraint_t, bool> *atomic_guards_map;
  constraint_t applied_node;
};

} //namespace simulator
} //namespace hydla 

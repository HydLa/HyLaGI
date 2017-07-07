#pragma once

#include "GuardNode.h"

namespace hydla { 
namespace simulator {
  
/**
 * A visitor class for GuardNode
 */
class GuardVisitor
{
public:
  GuardVisitor(){}

  virtual ~GuardVisitor(){}

  virtual void visit(AtomicGuardNode *atomic) = 0;
  virtual void visit(AndGuardNode *and_node) = 0;
  virtual void visit(OrGuardNode *or_node) = 0;
  virtual void visit(NotGuardNode *not_node) = 0;
};

} //namespace simulator
} //namespace hydla

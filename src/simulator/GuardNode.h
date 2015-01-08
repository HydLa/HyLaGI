#pragma once

#include "AtomicConstraint.h"
#include <list>

namespace hydla
{
namespace simulator
{

struct AskNode;
class GuardVisitor;
  
struct GuardNode
{
  std::list<AskNode *> asks;
  GuardNode *parent;

  virtual void accept(GuardVisitor *visitor) = 0;
  GuardNode();
  virtual ~GuardNode();
};

struct OrGuardNode : public GuardNode
{
  virtual void accept(GuardVisitor *visitor);
  OrGuardNode(GuardNode *l, GuardNode *r);
  GuardNode *lhs, *rhs;
};

struct AndGuardNode : public GuardNode
{
  AndGuardNode(GuardNode *l, GuardNode *r);
  virtual void accept(GuardVisitor *visitor);
  GuardNode *lhs, *rhs;
};

struct NotGuardNode : public GuardNode
{
  NotGuardNode(GuardNode *c);
  virtual void accept(GuardVisitor *visitor);
  virtual ~NotGuardNode();
  GuardNode *child;
};
  
struct AtomicGuardNode: public GuardNode{
  AtomicConstraint atomic_guard;
  //AtomicConstraint* get_next_atomic_guard();
  AtomicGuardNode(const AtomicConstraint &guard);
  virtual void accept(GuardVisitor *visitor);
  virtual ~AtomicGuardNode();
};

}

}

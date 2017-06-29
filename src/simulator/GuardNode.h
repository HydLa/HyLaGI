#pragma once

#include "AtomicConstraint.h"
#include <list>

namespace hydla {
namespace simulator {

struct AskNode;
class GuardVisitor;
  
struct GuardNode
{
  std::list<AskNode *> asks;
  GuardNode *parent;
  virtual std::list<AtomicConstraint *> get_atomic_guards() = 0;

  virtual void accept(GuardVisitor *visitor) = 0;
  GuardNode();
  virtual ~GuardNode();
};

struct OrGuardNode : public GuardNode
{
  std::list<AtomicConstraint*> get_atomic_guards();
  virtual void accept(GuardVisitor *visitor);
  OrGuardNode(GuardNode *l, GuardNode *r);
  GuardNode *lhs, *rhs;
};

struct AndGuardNode : public GuardNode
{
  std::list<AtomicConstraint*> get_atomic_guards();
  AndGuardNode(GuardNode *l, GuardNode *r);
  virtual void accept(GuardVisitor *visitor);
  GuardNode *lhs, *rhs;
};

struct NotGuardNode : public GuardNode
{
  std::list<AtomicConstraint*> get_atomic_guards();
  NotGuardNode(GuardNode *c);
  virtual void accept(GuardVisitor *visitor);
  virtual ~NotGuardNode();
  GuardNode *child;
};
  
struct AtomicGuardNode: public GuardNode
{
  AtomicConstraint atomic_guard;
  std::list<AtomicConstraint*> get_atomic_guards();
  //AtomicConstraint* get_next_atomic_guard();
  AtomicGuardNode(const AtomicConstraint &guard);
  virtual void accept(GuardVisitor *visitor);
  virtual ~AtomicGuardNode();
};

} // namespace simulator
} // namespace hydla

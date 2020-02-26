#include "GuardNode.h"
#include "GuardVisitor.h"
#include "RelationGraph.h"

namespace hydla {
namespace simulator {

using namespace std;

GuardNode::GuardNode() : parent(nullptr) {}
GuardNode::~GuardNode() {}

void OrGuardNode::accept(GuardVisitor *visitor) { visitor->visit(this); }

OrGuardNode::OrGuardNode(GuardNode *l, GuardNode *r) : lhs(l), rhs(r) {}

list<AtomicConstraint *> OrGuardNode::get_atomic_guards() {
  auto ret = lhs->get_atomic_guards();
  ret.merge(rhs->get_atomic_guards());
  return ret;
}

AndGuardNode::AndGuardNode(GuardNode *l, GuardNode *r) : lhs(l), rhs(r) {}

void AndGuardNode::accept(GuardVisitor *visitor) { visitor->visit(this); }

list<AtomicConstraint *> AndGuardNode::get_atomic_guards() {
  auto ret = lhs->get_atomic_guards();
  ret.merge(rhs->get_atomic_guards());
  return ret;
}

AtomicGuardNode::AtomicGuardNode(const AtomicConstraint &guard)
    : atomic_guard(guard) {}

void AtomicGuardNode::accept(GuardVisitor *visitor) { visitor->visit(this); }

AtomicGuardNode::~AtomicGuardNode() {}

list<AtomicConstraint *> AtomicGuardNode::get_atomic_guards() {
  list<AtomicConstraint *> ret;
  ret.push_back(&atomic_guard);
  return ret;
}

NotGuardNode::NotGuardNode(GuardNode *c) : child(c) {}
NotGuardNode::~NotGuardNode() {}

void NotGuardNode::accept(GuardVisitor *visitor) { visitor->visit(this); }

list<AtomicConstraint *> NotGuardNode::get_atomic_guards() {
  return child->get_atomic_guards();
}

} // namespace simulator
} // namespace hydla

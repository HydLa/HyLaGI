#include "GuardNode.h"
#include "GuardVisitor.h"
#include "RelationGraph.h"

namespace hydla{
namespace simulator{


GuardNode::GuardNode():parent(nullptr){}
GuardNode::~GuardNode(){}

void OrGuardNode::accept(GuardVisitor *visitor)
{
  visitor->visit(this);
}


OrGuardNode::OrGuardNode(GuardNode *l, GuardNode *r):lhs(l), rhs(r)
{}

AndGuardNode::AndGuardNode(GuardNode *l, GuardNode *r):lhs(l), rhs(r){}

void AndGuardNode::accept(GuardVisitor *visitor)
{
  visitor->visit(this);
}


AtomicGuardNode::AtomicGuardNode(const AtomicConstraint &guard):atomic_guard(guard)
{
}

void AtomicGuardNode::accept(GuardVisitor *visitor)
{
  visitor->visit(this);
}

AtomicGuardNode::~AtomicGuardNode()
{
}


NotGuardNode::NotGuardNode(GuardNode *c):child(c){}
NotGuardNode::~NotGuardNode(){}

void NotGuardNode::accept(GuardVisitor *visitor)
{
  visitor->visit(this);
}



}
}

#include "Node.h"

#include <assert.h>
#include <algorithm>
#include <typeinfo>

#include <boost/bind.hpp>

#include "ParseError.h"
#include "BaseNodeVisitor.h"
#include "TreeVisitor.h"

using namespace std;
using namespace boost;
using namespace hydla::parse_error;

namespace hydla { 
namespace parse_tree {
  
std::ostream& operator<<(std::ostream& s, const Node& node)
{
  return node.dump(s);
}

bool Node::is_same_struct(const Node& n) const
{
  return typeid(*this) == typeid(n);
}

bool UnaryNode::is_same_struct(const Node& n) const
{
  return typeid(*this) == typeid(n) &&
          child_->is_same_struct(*(static_cast<const UnaryNode*>(&n))->child_.get());
}

bool BinaryNode::is_same_struct(const Node& n) const
{
  return typeid(*this) == typeid(n) &&
          lhs_->is_same_struct(*(static_cast<const BinaryNode*>(&n))->lhs_.get()) &&
          rhs_->is_same_struct(*(static_cast<const BinaryNode*>(&n))->rhs_.get());
}

bool Number::is_same_struct(const Node& n) const
{
  return typeid(*this) == typeid(n) &&
          number_ == static_cast<const Number*>(&n)->number_;          
}

bool Variable::is_same_struct(const Node& n) const
{
  return typeid(*this) == typeid(n) &&
          name_ == static_cast<const Variable*>(&n)->name_;          
}

std::ostream& Caller::dump(std::ostream& s) const 
{
  actual_args_t::const_iterator it  = actual_args_.begin();
  actual_args_t::const_iterator end = actual_args_.end();

  s << "call<" 
    << get_id()
    << ","
    << name_
    << "(";

  if(it!=end) s << **(it++);
  while(it!=end) {
    s << "," << **(it++);
  }

  s << ")>";
  if(child_) {
    s <<  "["
      << *child_
      << "]";
  }

  return s;
}

std::ostream& Definition::dump(std::ostream& s) const 
{
  bound_variables_t::const_iterator it  = bound_variables_.begin();
  bound_variables_t::const_iterator end = bound_variables_.end();

  s << name_
    << "<"
    << get_id()
    << ">(";

  if(it!=end) s << *(it++);
  while(it!=end) {
    s << "," << *(it++);
  }
  s << "):=" << *child_;

  return s;
}

node_sptr Caller::clone()
{
  boost::shared_ptr<ProgramCaller> n(new ProgramCaller());
  n->name_ = name_;
  n->id_  = id_;

  n->actual_args_.resize(actual_args_.size());
  copy(actual_args_.begin(), actual_args_.end(),  n->actual_args_.begin());
  
  if(child_) n->child_ = child_->clone();
  
  return n;
}

node_sptr Definition::clone()
{
  boost::shared_ptr<ConstraintDefinition> n(new ConstraintDefinition());
  n->name_ = name_;
  n->id_  = id_;

  n->bound_variables_.resize(bound_variables_.size());
  copy(bound_variables_.begin(), bound_variables_.end(),  n->bound_variables_.begin());
  n->child_ = child_->clone();

  return n;
}

/**
 * em[hฬacceptึ่`
 */

#define DEFINE_ACCEPT_FUNC(CLASS, VISITOR) \
  void CLASS::accept(node_sptr own, \
                     VISITOR* visitor) \
  { \
    assert(this == own.get()); \
    visitor->visit(boost::shared_static_cast<CLASS>(own)); \
  }

/// BaseNodeVisitorฬacceptึ่`
#define DEFINE_BASE_NODE_VISITOR_ACCEPT_FUNC(CLASS) \
  DEFINE_ACCEPT_FUNC(CLASS, BaseNodeVisitor)

DEFINE_BASE_NODE_VISITOR_ACCEPT_FUNC(FactorNode)
DEFINE_BASE_NODE_VISITOR_ACCEPT_FUNC(UnaryNode)
DEFINE_BASE_NODE_VISITOR_ACCEPT_FUNC(BinaryNode)

/// TreeVisitorฬacceptึ่`
#define DEFINE_TREE_VISITOR_ACCEPT_FUNC(CLASS) \
  DEFINE_ACCEPT_FUNC(CLASS, TreeVisitor)

//่`
DEFINE_TREE_VISITOR_ACCEPT_FUNC(ProgramDefinition)
DEFINE_TREE_VISITOR_ACCEPT_FUNC(ConstraintDefinition)

//ฤัoต
DEFINE_TREE_VISITOR_ACCEPT_FUNC(ProgramCaller)
DEFINE_TREE_VISITOR_ACCEPT_FUNC(ConstraintCaller)

 //ง๑ฎ
DEFINE_TREE_VISITOR_ACCEPT_FUNC(Constraint);

//Tellง๑
DEFINE_TREE_VISITOR_ACCEPT_FUNC(Tell)

//Askง๑
DEFINE_TREE_VISITOR_ACCEPT_FUNC(Ask)

//ไrZq
DEFINE_TREE_VISITOR_ACCEPT_FUNC(Equal)
DEFINE_TREE_VISITOR_ACCEPT_FUNC(UnEqual)
DEFINE_TREE_VISITOR_ACCEPT_FUNC(Less)
DEFINE_TREE_VISITOR_ACCEPT_FUNC(LessEqual)
DEFINE_TREE_VISITOR_ACCEPT_FUNC(Greater)
DEFINE_TREE_VISITOR_ACCEPT_FUNC(GreaterEqual)

//_Zq
DEFINE_TREE_VISITOR_ACCEPT_FUNC(LogicalAnd)
DEFINE_TREE_VISITOR_ACCEPT_FUNC(LogicalOr)

//Zp๑Zq
DEFINE_TREE_VISITOR_ACCEPT_FUNC(Plus)
DEFINE_TREE_VISITOR_ACCEPT_FUNC(Subtract)
DEFINE_TREE_VISITOR_ACCEPT_FUNC(Times)
DEFINE_TREE_VISITOR_ACCEPT_FUNC(Divide)

//ZpPZq
DEFINE_TREE_VISITOR_ACCEPT_FUNC(Negative)
DEFINE_TREE_VISITOR_ACCEPT_FUNC(Positive)

//ง๑Kw่`Zq
DEFINE_TREE_VISITOR_ACCEPT_FUNC(Weaker)
DEFINE_TREE_VISITOR_ACCEPT_FUNC(Parallel)

// Zq
DEFINE_TREE_VISITOR_ACCEPT_FUNC(Always)

//๗ช
DEFINE_TREE_VISITOR_ACCEPT_FUNC(Differential)

//ถษภ
DEFINE_TREE_VISITOR_ACCEPT_FUNC(Previous)

//ฯEฉฯ
DEFINE_TREE_VISITOR_ACCEPT_FUNC(Variable)

//
DEFINE_TREE_VISITOR_ACCEPT_FUNC(Number)

} //namespace parse_tree
} //namespace hydla

#include "Node.h"

#include <assert.h>
#include <algorithm>
#include <boost/bind.hpp>

#include "ParseError.h"
#include "TreeVisitor.h"

using namespace std;
using namespace boost;
using namespace hydla::parse_error;

namespace hydla { 
namespace parse_tree {
  
std::ostream& operator<< (std::ostream& s, Node& node)
{
  s << node.to_string();
  return s;
}

std::string Caller::to_string() const
{
  actual_args_t::const_iterator it  = actual_args_.begin();
  actual_args_t::const_iterator end = actual_args_.end();

  std::string s;
  s += "call<";
  s += name_;
  s += "(";

  if(it!=end) s += (*it++)->to_string();
  while(it!=end) {
    s += ",";
    s += (*it++)->to_string();
  }

  s += ")>";
  if(child_) {
    s +=  "[";
    s += child_->to_string();
    s += "]";
  }
  return s;
}

std::string Definition::to_string() const
{
  bound_variables_t::const_iterator it  = bound_variables_.begin();
  bound_variables_t::const_iterator end = bound_variables_.end();

  std::string s;
  s += name_;
  s += "(";

  if(it!=end) s += *(it++);
  while(it!=end) {
    s += ",";
    s += *(it++);
  }

  s += "):=";
  s += child_->to_string();
  return s;
}

node_sptr Caller::clone()
{
  boost::shared_ptr<ProgramCaller> n(new ProgramCaller());
  n->name_ = name_;
  n->actual_args_.resize(actual_args_.size());
  copy(actual_args_.begin(), actual_args_.end(),  n->actual_args_.begin());
  if(child_) n->child_ = child_->clone();
  
  return n;
}

node_sptr Definition::clone()
{
  boost::shared_ptr<ConstraintDefinition> n(new ConstraintDefinition());
  n->name_ = name_;
  n->bound_variables_.resize(bound_variables_.size());
  copy(bound_variables_.begin(), bound_variables_.end(),  n->bound_variables_.begin());
  n->child_ = child_->clone();

  return n;
}

/**
 * �e�m�[�h��accept�֐���`
 */

#define DEFINE_ACCEPT_FUNC(CLASS) \
  void CLASS::accept(node_sptr own, \
                     TreeVisitor* visitor) \
  { \
    assert(this == own.get()); \
    visitor->visit(boost::shared_static_cast<CLASS>(own)); \
  }

//��`
DEFINE_ACCEPT_FUNC(ProgramDefinition)
DEFINE_ACCEPT_FUNC(ConstraintDefinition)

//�Ăяo��
DEFINE_ACCEPT_FUNC(ProgramCaller)
DEFINE_ACCEPT_FUNC(ConstraintCaller)

 //����
DEFINE_ACCEPT_FUNC(Constraint);

//Tell����
DEFINE_ACCEPT_FUNC(Tell)

//Ask����
DEFINE_ACCEPT_FUNC(Ask)

//��r���Z�q
DEFINE_ACCEPT_FUNC(Equal)
DEFINE_ACCEPT_FUNC(UnEqual)
DEFINE_ACCEPT_FUNC(Less)
DEFINE_ACCEPT_FUNC(LessEqual)
DEFINE_ACCEPT_FUNC(Greater)
DEFINE_ACCEPT_FUNC(GreaterEqual)

//�_�����Z�q
DEFINE_ACCEPT_FUNC(LogicalAnd)
DEFINE_ACCEPT_FUNC(LogicalOr)

//�Z�p�񍀉��Z�q
DEFINE_ACCEPT_FUNC(Plus)
DEFINE_ACCEPT_FUNC(Subtract)
DEFINE_ACCEPT_FUNC(Times)
DEFINE_ACCEPT_FUNC(Divide)

//�Z�p�P�����Z�q
DEFINE_ACCEPT_FUNC(Negative)
DEFINE_ACCEPT_FUNC(Positive)

//����K�w��`���Z�q
DEFINE_ACCEPT_FUNC(Weaker)
DEFINE_ACCEPT_FUNC(Parallel)

// �������Z�q
DEFINE_ACCEPT_FUNC(Always)

//����
DEFINE_ACCEPT_FUNC(Differential)

//���Ɍ�
DEFINE_ACCEPT_FUNC(Previous)

//�ϐ��E�����ϐ�
DEFINE_ACCEPT_FUNC(Variable)

//����
DEFINE_ACCEPT_FUNC(Number)

} //namespace parse_tree
} //namespace hydla

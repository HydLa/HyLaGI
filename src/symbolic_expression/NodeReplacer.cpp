#include "NodeReplacer.h"

namespace hydla{
namespace symbolic_expression{

#define DEFINE_VISIT_FACTOR(NAME) \
  void NodeReplacer::visit(boost::shared_ptr<NAME> node) \
  { \
    if(node->is_same_struct(*source_,true)) \
    { \
      new_child_ = dest_; \
    } \
  }

#define DEFINE_VISIT_UNARY(NAME) \
  void NodeReplacer::visit(boost::shared_ptr<NAME> node) \
  { \
    if(node->is_same_struct(*source_,true)) \
    { \
      new_child_ = dest_; \
      return; \
    } \
    accept(node->get_child()); \
    if(new_child_) \
    { \
      node->set_child(new_child_); \
      new_child_.reset(); \
    } \
  }

#define DEFINE_VISIT_CALLER(NAME) \
  void NodeReplacer::visit(boost::shared_ptr<NAME> node) \
  { \
    if(node->is_same_struct(*source_,true)) \
    { \
      new_child_ = dest_; \
      return; \
    } \
    accept(node->get_child()); \
    if(new_child_) \
    { \
      node->set_child(new_child_); \
      new_child_.reset(); \
    } \
    for(auto it = node->actual_arg_begin(); it != node->actual_arg_end(); it++) \
    { \
      accept(*it); \
      if(new_child_) \
      { \
        *it = new_child_; \
        new_child_.reset(); \
      } \
    } \
  }

#define DEFINE_VISIT_BINARY(NAME) \
  void NodeReplacer::visit(boost::shared_ptr<NAME> node) \
  { \
    if(node->is_same_struct(*source_,true)) \
    { \
      new_child_ = dest_; \
      return; \
    } \
    accept(node->get_lhs()); \
    if(new_child_) \
    { \
      node->set_lhs(new_child_); \
      new_child_.reset(); \
    } \
    accept(node->get_rhs()); \
    if(new_child_) \
    { \
      node->set_rhs(new_child_); \
      new_child_.reset(); \
    } \
  }

#define DEFINE_VISIT_ARBITRARY(NAME) \
  void NodeReplacer::visit(boost::shared_ptr<NAME> node) \
  { \
    if(node->is_same_struct(*source_,true)) \
    { \
      new_child_ = dest_; \
      return; \
    } \
    for(int i = 0; i < node->get_arguments_size(); i++) \
    { \
      accept(node->get_argument(i)); \
      if(new_child_) \
      { \
        node->set_argument(new_child_,i); \
        new_child_.reset(); \
      } \
    } \
  }

NodeReplacer::NodeReplacer(){}
NodeReplacer::~NodeReplacer(){}

DEFINE_VISIT_UNARY(ConstraintDefinition)
DEFINE_VISIT_UNARY(ProgramDefinition)
DEFINE_VISIT_UNARY(Constraint)
DEFINE_VISIT_BINARY(Ask)
DEFINE_VISIT_UNARY(Tell)
DEFINE_VISIT_BINARY(Equal)
DEFINE_VISIT_BINARY(UnEqual)
DEFINE_VISIT_BINARY(Less)
DEFINE_VISIT_BINARY(LessEqual)
DEFINE_VISIT_BINARY(Greater)
DEFINE_VISIT_BINARY(GreaterEqual)
DEFINE_VISIT_BINARY(LogicalAnd)
DEFINE_VISIT_BINARY(LogicalOr)
DEFINE_VISIT_BINARY(Plus)
DEFINE_VISIT_BINARY(Subtract)
DEFINE_VISIT_BINARY(Times)
DEFINE_VISIT_BINARY(Divide)
DEFINE_VISIT_BINARY(Power)
DEFINE_VISIT_UNARY(Negative)
DEFINE_VISIT_UNARY(Positive)
DEFINE_VISIT_BINARY(Weaker)
DEFINE_VISIT_BINARY(Parallel)
DEFINE_VISIT_UNARY(Always)
DEFINE_VISIT_UNARY(Differential)
DEFINE_VISIT_UNARY(Previous)
DEFINE_VISIT_FACTOR(Print)
DEFINE_VISIT_FACTOR(PrintPP)
DEFINE_VISIT_FACTOR(PrintIP)
DEFINE_VISIT_FACTOR(Scan)
DEFINE_VISIT_FACTOR(Exit)
DEFINE_VISIT_FACTOR(Abort)
DEFINE_VISIT_FACTOR(SVtimer)
DEFINE_VISIT_UNARY(Not)
DEFINE_VISIT_FACTOR(Pi)
DEFINE_VISIT_FACTOR(E)
DEFINE_VISIT_ARBITRARY(Function)
DEFINE_VISIT_ARBITRARY(UnsupportedFunction)
DEFINE_VISIT_FACTOR(Variable)
DEFINE_VISIT_FACTOR(Number)
DEFINE_VISIT_FACTOR(Float)
DEFINE_VISIT_FACTOR(Parameter)
DEFINE_VISIT_FACTOR(SymbolicT)
DEFINE_VISIT_FACTOR(Infinity)
DEFINE_VISIT_FACTOR(True)
DEFINE_VISIT_FACTOR(False)
DEFINE_VISIT_ARBITRARY(ExpressionList)
DEFINE_VISIT_ARBITRARY(ProgramList)
DEFINE_VISIT_BINARY(EachElement)
DEFINE_VISIT_BINARY(DifferentVariable)
DEFINE_VISIT_BINARY(ExpressionListElement)
DEFINE_VISIT_UNARY(ExpressionListDefinition)
DEFINE_VISIT_UNARY(ProgramListDefinition)
DEFINE_VISIT_BINARY(ProgramListElement)
DEFINE_VISIT_UNARY(SumOfList)
DEFINE_VISIT_UNARY(SizeOfList)
DEFINE_VISIT_BINARY(Range)
DEFINE_VISIT_BINARY(Union)
DEFINE_VISIT_BINARY(Intersection)
DEFINE_VISIT_CALLER(ExpressionListCaller)
DEFINE_VISIT_CALLER(ProgramListCaller)
DEFINE_VISIT_CALLER(ProgramCaller)
DEFINE_VISIT_CALLER(ConstraintCaller)

void NodeReplacer::visit(boost::shared_ptr<ConditionalProgramList> node)
{
  if(node->is_same_struct(*source_,true))
  {
    new_child_ = dest_;
    return;
  }
  accept(node->get_program());
  if(new_child_)
  {
    node->set_program(new_child_);
    new_child_.reset();
  }
  for(int i = 0; i < node->get_arguments_size(); i++)
  {
    accept(node->get_argument(i));
    if(new_child_)
    {
      node->set_argument(new_child_,i);
      new_child_.reset();
    }
  }
}

void NodeReplacer::visit(boost::shared_ptr<ConditionalExpressionList> node)
{
  if(node->is_same_struct(*source_,true))
  {
    new_child_ = dest_;
    return;
  }
  accept(node->get_expression());
  if(new_child_)
  {
    node->set_expression(new_child_);
    new_child_.reset();
  }
  for(int i = 0; i < node->get_arguments_size(); i++)
  {
    accept(node->get_argument(i));
    if(new_child_)
    {
      node->set_argument(new_child_,i);
      new_child_.reset();
    }
  }
}


} // namespace symbolic_expression
} // namespace hydla

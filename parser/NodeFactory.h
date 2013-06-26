#ifndef _INCLUDED_HYDLA_PARSER_NODE_FACTORY_H_
#define _INCLUDED_HYDLA_PARSER_NODE_FACTORY_H_

#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>

#include "Node.h"

namespace hydla { 
namespace parser {

#define NODE_FACTORY_DEFINE_NODE_CREATE_FUNC(NAME) \
  virtual boost::shared_ptr<hydla::parse_tree::NAME> \
  create(hydla::parse_tree::NAME) const = 0;

class NodeFactory 
{
public:
  NodeFactory()
  {}

  virtual ~NodeFactory()
  {}

  template<typename NodeType>
  boost::shared_ptr<NodeType> create() const
  {
    return create(NodeType());
  }

protected:
  //��`
  NODE_FACTORY_DEFINE_NODE_CREATE_FUNC(ProgramDefinition)
  NODE_FACTORY_DEFINE_NODE_CREATE_FUNC(ConstraintDefinition)

  //�Ăяo��
  NODE_FACTORY_DEFINE_NODE_CREATE_FUNC(ProgramCaller)
  NODE_FACTORY_DEFINE_NODE_CREATE_FUNC(ConstraintCaller)
  
  //����
  NODE_FACTORY_DEFINE_NODE_CREATE_FUNC(Constraint)

  //Tell����
  NODE_FACTORY_DEFINE_NODE_CREATE_FUNC(Tell)

  //Ask����
  NODE_FACTORY_DEFINE_NODE_CREATE_FUNC(Ask)

  //��r���Z�q
  NODE_FACTORY_DEFINE_NODE_CREATE_FUNC(Equal)
  NODE_FACTORY_DEFINE_NODE_CREATE_FUNC(UnEqual)
  NODE_FACTORY_DEFINE_NODE_CREATE_FUNC(Less)
  NODE_FACTORY_DEFINE_NODE_CREATE_FUNC(LessEqual)
  NODE_FACTORY_DEFINE_NODE_CREATE_FUNC(Greater)
  NODE_FACTORY_DEFINE_NODE_CREATE_FUNC(GreaterEqual)

  //�_�����Z�q
  NODE_FACTORY_DEFINE_NODE_CREATE_FUNC(LogicalAnd)
  NODE_FACTORY_DEFINE_NODE_CREATE_FUNC(LogicalOr)
  NODE_FACTORY_DEFINE_NODE_CREATE_FUNC(Not)

  //�Z�p�񍀉��Z�q
  NODE_FACTORY_DEFINE_NODE_CREATE_FUNC(Plus)
  NODE_FACTORY_DEFINE_NODE_CREATE_FUNC(Subtract)
  NODE_FACTORY_DEFINE_NODE_CREATE_FUNC(Times)
  NODE_FACTORY_DEFINE_NODE_CREATE_FUNC(Divide)
  NODE_FACTORY_DEFINE_NODE_CREATE_FUNC(Power)
  
  //�Z�p�P�����Z�q
  NODE_FACTORY_DEFINE_NODE_CREATE_FUNC(Negative)
  NODE_FACTORY_DEFINE_NODE_CREATE_FUNC(Positive)

  //����K�w��`���Z�q
  NODE_FACTORY_DEFINE_NODE_CREATE_FUNC(Weaker)
  NODE_FACTORY_DEFINE_NODE_CREATE_FUNC(Parallel)

  // �������Z�q
  NODE_FACTORY_DEFINE_NODE_CREATE_FUNC(Always)

  //����
  NODE_FACTORY_DEFINE_NODE_CREATE_FUNC(Differential)
  
  //���Ɍ�
  NODE_FACTORY_DEFINE_NODE_CREATE_FUNC(Previous)
  
  //�~����
  NODE_FACTORY_DEFINE_NODE_CREATE_FUNC(Pi)
  //���R�ΐ��̒�
  NODE_FACTORY_DEFINE_NODE_CREATE_FUNC(E)
  //�֐�
  NODE_FACTORY_DEFINE_NODE_CREATE_FUNC(Function)
  NODE_FACTORY_DEFINE_NODE_CREATE_FUNC(UnsupportedFunction)
  
  

  //�ϐ��E�����ϐ�
  NODE_FACTORY_DEFINE_NODE_CREATE_FUNC(Variable)

  //����
  NODE_FACTORY_DEFINE_NODE_CREATE_FUNC(Number)
  
  //Print
  NODE_FACTORY_DEFINE_NODE_CREATE_FUNC(Print)
  NODE_FACTORY_DEFINE_NODE_CREATE_FUNC(PrintPP)
  NODE_FACTORY_DEFINE_NODE_CREATE_FUNC(PrintIP)
  NODE_FACTORY_DEFINE_NODE_CREATE_FUNC(Scan)
  NODE_FACTORY_DEFINE_NODE_CREATE_FUNC(Exit)
  NODE_FACTORY_DEFINE_NODE_CREATE_FUNC(Abort)

  //SystemVariable
  NODE_FACTORY_DEFINE_NODE_CREATE_FUNC(SVtimer)

};                                                     

} //namespace parser
} //namespace hydla

#endif //_INCLUDED_HYDLA_PARSER_NODE_FACTORY_H_

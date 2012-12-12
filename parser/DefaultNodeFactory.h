#ifndef _INCLUDED_HYDLA_PARSER_DEFAULT_NODE_FACTORY_H_
#define _INCLUDED_HYDLA_PARSER_DEFAULT_NODE_FACTORY_H_

#include "NodeFactory.h"

namespace hydla { 
namespace parser {

#define DEFAULT_NODE_FACTORY_DEFINE_NODE_CREATE_FUNC(NAME) \
  virtual boost::shared_ptr<hydla::parse_tree::NAME>            \
  create(hydla::parse_tree::NAME) const                   \
  { \
    return boost::make_shared<hydla::parse_tree::NAME>(); \
  }

class DefaultNodeFactory :  public NodeFactory
{
public:

protected:
  //è`
  DEFAULT_NODE_FACTORY_DEFINE_NODE_CREATE_FUNC(ProgramDefinition)
  DEFAULT_NODE_FACTORY_DEFINE_NODE_CREATE_FUNC(ConstraintDefinition)

  //ÄÑoµ
  DEFAULT_NODE_FACTORY_DEFINE_NODE_CREATE_FUNC(ProgramCaller)
  DEFAULT_NODE_FACTORY_DEFINE_NODE_CREATE_FUNC(ConstraintCaller)
  
  //§ñ®
  DEFAULT_NODE_FACTORY_DEFINE_NODE_CREATE_FUNC(Constraint);

  //Tell§ñ
  DEFAULT_NODE_FACTORY_DEFINE_NODE_CREATE_FUNC(Tell)

  //Ask§ñ
  DEFAULT_NODE_FACTORY_DEFINE_NODE_CREATE_FUNC(Ask)

  //ärZq
  DEFAULT_NODE_FACTORY_DEFINE_NODE_CREATE_FUNC(Equal)
  DEFAULT_NODE_FACTORY_DEFINE_NODE_CREATE_FUNC(UnEqual)
  DEFAULT_NODE_FACTORY_DEFINE_NODE_CREATE_FUNC(Less)
  DEFAULT_NODE_FACTORY_DEFINE_NODE_CREATE_FUNC(LessEqual)
  DEFAULT_NODE_FACTORY_DEFINE_NODE_CREATE_FUNC(Greater)
  DEFAULT_NODE_FACTORY_DEFINE_NODE_CREATE_FUNC(GreaterEqual)

  //_Zq
  DEFAULT_NODE_FACTORY_DEFINE_NODE_CREATE_FUNC(LogicalAnd)
  DEFAULT_NODE_FACTORY_DEFINE_NODE_CREATE_FUNC(LogicalOr)
  DEFAULT_NODE_FACTORY_DEFINE_NODE_CREATE_FUNC(Not)

  //ZpñZq
  DEFAULT_NODE_FACTORY_DEFINE_NODE_CREATE_FUNC(Plus)
  DEFAULT_NODE_FACTORY_DEFINE_NODE_CREATE_FUNC(Subtract)
  DEFAULT_NODE_FACTORY_DEFINE_NODE_CREATE_FUNC(Times)
  DEFAULT_NODE_FACTORY_DEFINE_NODE_CREATE_FUNC(Divide)
  DEFAULT_NODE_FACTORY_DEFINE_NODE_CREATE_FUNC(Power)
  
  //ZpPZq
  DEFAULT_NODE_FACTORY_DEFINE_NODE_CREATE_FUNC(Negative)
  DEFAULT_NODE_FACTORY_DEFINE_NODE_CREATE_FUNC(Positive)

  //§ñKwè`Zq
  DEFAULT_NODE_FACTORY_DEFINE_NODE_CREATE_FUNC(Weaker)
  DEFAULT_NODE_FACTORY_DEFINE_NODE_CREATE_FUNC(Parallel)

  // Zq
  DEFAULT_NODE_FACTORY_DEFINE_NODE_CREATE_FUNC(Always)

  //÷ª
  DEFAULT_NODE_FACTORY_DEFINE_NODE_CREATE_FUNC(Differential)
  
  //¶ÉÀ
  DEFAULT_NODE_FACTORY_DEFINE_NODE_CREATE_FUNC(Previous)
  
  // Ö
  DEFAULT_NODE_FACTORY_DEFINE_NODE_CREATE_FUNC(Function)
  DEFAULT_NODE_FACTORY_DEFINE_NODE_CREATE_FUNC(UnsupportedFunction)
  // ~ü¦
  DEFAULT_NODE_FACTORY_DEFINE_NODE_CREATE_FUNC(Pi)
  // ©RÎÌê
  DEFAULT_NODE_FACTORY_DEFINE_NODE_CREATE_FUNC(E)

  //ÏE©Ï
  DEFAULT_NODE_FACTORY_DEFINE_NODE_CREATE_FUNC(Variable)

  //
  DEFAULT_NODE_FACTORY_DEFINE_NODE_CREATE_FUNC(Number)
  
  //wada Print
  DEFAULT_NODE_FACTORY_DEFINE_NODE_CREATE_FUNC(Print)
  DEFAULT_NODE_FACTORY_DEFINE_NODE_CREATE_FUNC(PrintPP)
  DEFAULT_NODE_FACTORY_DEFINE_NODE_CREATE_FUNC(PrintIP)
  DEFAULT_NODE_FACTORY_DEFINE_NODE_CREATE_FUNC(Scan)
  DEFAULT_NODE_FACTORY_DEFINE_NODE_CREATE_FUNC(Exit)
  DEFAULT_NODE_FACTORY_DEFINE_NODE_CREATE_FUNC(Abort)

  //SystemVariable
  DEFAULT_NODE_FACTORY_DEFINE_NODE_CREATE_FUNC(SVtimer)

};


} //namespace parser
} //namespace hydla

#endif //_INCLUDED_HYDLA_PARSER_DEFAULT_NODE_FACTORY_H_

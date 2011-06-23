#ifndef _INCLUDED_HYDLA_PARSER_NODE_FACTORY_H_
#define _INCLUDED_HYDLA_PARSER_NODE_FACTORY_H_

#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>

#include "Node.h"

namespace hydla { 
namespace parser {

#define NODE_FACTORY_DEFILE_NODE_CREATE_FUNC(NAME) \
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
  //è`
  NODE_FACTORY_DEFILE_NODE_CREATE_FUNC(ProgramDefinition)
  NODE_FACTORY_DEFILE_NODE_CREATE_FUNC(ConstraintDefinition)

  //ÄÑoµ
  NODE_FACTORY_DEFILE_NODE_CREATE_FUNC(ProgramCaller)
  NODE_FACTORY_DEFILE_NODE_CREATE_FUNC(ConstraintCaller)
  
  //§ñ®
  NODE_FACTORY_DEFILE_NODE_CREATE_FUNC(Constraint)

  //Tell§ñ
  NODE_FACTORY_DEFILE_NODE_CREATE_FUNC(Tell)

  //Ask§ñ
  NODE_FACTORY_DEFILE_NODE_CREATE_FUNC(Ask)

  //ärZq
  NODE_FACTORY_DEFILE_NODE_CREATE_FUNC(Equal)
  NODE_FACTORY_DEFILE_NODE_CREATE_FUNC(UnEqual)
  NODE_FACTORY_DEFILE_NODE_CREATE_FUNC(Less)
  NODE_FACTORY_DEFILE_NODE_CREATE_FUNC(LessEqual)
  NODE_FACTORY_DEFILE_NODE_CREATE_FUNC(Greater)
  NODE_FACTORY_DEFILE_NODE_CREATE_FUNC(GreaterEqual)

  //_Zq
  NODE_FACTORY_DEFILE_NODE_CREATE_FUNC(LogicalAnd)
  NODE_FACTORY_DEFILE_NODE_CREATE_FUNC(LogicalOr)

  //ZpñZq
  NODE_FACTORY_DEFILE_NODE_CREATE_FUNC(Plus)
  NODE_FACTORY_DEFILE_NODE_CREATE_FUNC(Subtract)
  NODE_FACTORY_DEFILE_NODE_CREATE_FUNC(Times)
  NODE_FACTORY_DEFILE_NODE_CREATE_FUNC(Divide)
  NODE_FACTORY_DEFILE_NODE_CREATE_FUNC(Power)
  
  //ZpPZq
  NODE_FACTORY_DEFILE_NODE_CREATE_FUNC(Negative)
  NODE_FACTORY_DEFILE_NODE_CREATE_FUNC(Positive)

  //§ñKwè`Zq
  NODE_FACTORY_DEFILE_NODE_CREATE_FUNC(Weaker)
  NODE_FACTORY_DEFILE_NODE_CREATE_FUNC(Parallel)

  // Zq
  NODE_FACTORY_DEFILE_NODE_CREATE_FUNC(Always)

  //÷ª
  NODE_FACTORY_DEFILE_NODE_CREATE_FUNC(Differential)
  
  //¶ÉÀ
  NODE_FACTORY_DEFILE_NODE_CREATE_FUNC(Previous)
  
  //OpÖ
  NODE_FACTORY_DEFILE_NODE_CREATE_FUNC(Sin)
  NODE_FACTORY_DEFILE_NODE_CREATE_FUNC(Cos)
  NODE_FACTORY_DEFILE_NODE_CREATE_FUNC(Tan)
  //tOpÖ
  NODE_FACTORY_DEFILE_NODE_CREATE_FUNC(Asin)
  NODE_FACTORY_DEFILE_NODE_CREATE_FUNC(Acos)
  NODE_FACTORY_DEFILE_NODE_CREATE_FUNC(Atan)
  //~ü¦
  NODE_FACTORY_DEFILE_NODE_CREATE_FUNC(Pi)
  //Î
  NODE_FACTORY_DEFILE_NODE_CREATE_FUNC(Log)
  NODE_FACTORY_DEFILE_NODE_CREATE_FUNC(Ln)
  //©RÎÌê
  NODE_FACTORY_DEFILE_NODE_CREATE_FUNC(E)
  //CÓÌ¶ñ
  NODE_FACTORY_DEFILE_NODE_CREATE_FUNC(ArbitraryBinary)
  NODE_FACTORY_DEFILE_NODE_CREATE_FUNC(ArbitraryUnary)
  NODE_FACTORY_DEFILE_NODE_CREATE_FUNC(ArbitraryFactor)
  

  //ÏE©Ï
  NODE_FACTORY_DEFILE_NODE_CREATE_FUNC(Variable)

  //
  NODE_FACTORY_DEFILE_NODE_CREATE_FUNC(Number)
};                                                     

} //namespace parser
} //namespace hydla

#endif //_INCLUDED_HYDLA_PARSER_NODE_FACTORY_H_

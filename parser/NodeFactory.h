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
  //่`
  NODE_FACTORY_DEFILE_NODE_CREATE_FUNC(ProgramDefinition)
  NODE_FACTORY_DEFILE_NODE_CREATE_FUNC(ConstraintDefinition)

  //ฤัoต
  NODE_FACTORY_DEFILE_NODE_CREATE_FUNC(ProgramCaller)
  NODE_FACTORY_DEFILE_NODE_CREATE_FUNC(ConstraintCaller)
  
  //ง๑ฎ
  NODE_FACTORY_DEFILE_NODE_CREATE_FUNC(Constraint)

  //Tellง๑
  NODE_FACTORY_DEFILE_NODE_CREATE_FUNC(Tell)

  //Askง๑
  NODE_FACTORY_DEFILE_NODE_CREATE_FUNC(Ask)

  //ไrZq
  NODE_FACTORY_DEFILE_NODE_CREATE_FUNC(Equal)
  NODE_FACTORY_DEFILE_NODE_CREATE_FUNC(UnEqual)
  NODE_FACTORY_DEFILE_NODE_CREATE_FUNC(Less)
  NODE_FACTORY_DEFILE_NODE_CREATE_FUNC(LessEqual)
  NODE_FACTORY_DEFILE_NODE_CREATE_FUNC(Greater)
  NODE_FACTORY_DEFILE_NODE_CREATE_FUNC(GreaterEqual)

  //_Zq
  NODE_FACTORY_DEFILE_NODE_CREATE_FUNC(LogicalAnd)
  NODE_FACTORY_DEFILE_NODE_CREATE_FUNC(LogicalOr)

  //Zp๑Zq
  NODE_FACTORY_DEFILE_NODE_CREATE_FUNC(Plus)
  NODE_FACTORY_DEFILE_NODE_CREATE_FUNC(Subtract)
  NODE_FACTORY_DEFILE_NODE_CREATE_FUNC(Times)
  NODE_FACTORY_DEFILE_NODE_CREATE_FUNC(Divide)
  NODE_FACTORY_DEFILE_NODE_CREATE_FUNC(Power)
  
  //ZpPZq
  NODE_FACTORY_DEFILE_NODE_CREATE_FUNC(Negative)
  NODE_FACTORY_DEFILE_NODE_CREATE_FUNC(Positive)

  //ง๑Kw่`Zq
  NODE_FACTORY_DEFILE_NODE_CREATE_FUNC(Weaker)
  NODE_FACTORY_DEFILE_NODE_CREATE_FUNC(Parallel)

  // Zq
  NODE_FACTORY_DEFILE_NODE_CREATE_FUNC(Always)

  //๗ช
  NODE_FACTORY_DEFILE_NODE_CREATE_FUNC(Differential)
  
  //ถษภ
  NODE_FACTORY_DEFILE_NODE_CREATE_FUNC(Previous)

  //ฯEฉฯ
  NODE_FACTORY_DEFILE_NODE_CREATE_FUNC(Variable)

  //
  NODE_FACTORY_DEFILE_NODE_CREATE_FUNC(Number)
};                                                     

} //namespace parser
} //namespace hydla

#endif //_INCLUDED_HYDLA_PARSER_NODE_FACTORY_H_

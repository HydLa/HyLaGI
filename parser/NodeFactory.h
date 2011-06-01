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
  //定義
  NODE_FACTORY_DEFILE_NODE_CREATE_FUNC(ProgramDefinition)
  NODE_FACTORY_DEFILE_NODE_CREATE_FUNC(ConstraintDefinition)

  //呼び出し
  NODE_FACTORY_DEFILE_NODE_CREATE_FUNC(ProgramCaller)
  NODE_FACTORY_DEFILE_NODE_CREATE_FUNC(ConstraintCaller)
  
  //制約式
  NODE_FACTORY_DEFILE_NODE_CREATE_FUNC(Constraint)

  //Tell制約
  NODE_FACTORY_DEFILE_NODE_CREATE_FUNC(Tell)

  //Ask制約
  NODE_FACTORY_DEFILE_NODE_CREATE_FUNC(Ask)

  //比較演算子
  NODE_FACTORY_DEFILE_NODE_CREATE_FUNC(Equal)
  NODE_FACTORY_DEFILE_NODE_CREATE_FUNC(UnEqual)
  NODE_FACTORY_DEFILE_NODE_CREATE_FUNC(Less)
  NODE_FACTORY_DEFILE_NODE_CREATE_FUNC(LessEqual)
  NODE_FACTORY_DEFILE_NODE_CREATE_FUNC(Greater)
  NODE_FACTORY_DEFILE_NODE_CREATE_FUNC(GreaterEqual)

  //論理演算子
  NODE_FACTORY_DEFILE_NODE_CREATE_FUNC(LogicalAnd)
  NODE_FACTORY_DEFILE_NODE_CREATE_FUNC(LogicalOr)

  //算術二項演算子
  NODE_FACTORY_DEFILE_NODE_CREATE_FUNC(Plus)
  NODE_FACTORY_DEFILE_NODE_CREATE_FUNC(Subtract)
  NODE_FACTORY_DEFILE_NODE_CREATE_FUNC(Times)
  NODE_FACTORY_DEFILE_NODE_CREATE_FUNC(Divide)
  NODE_FACTORY_DEFILE_NODE_CREATE_FUNC(Power)
  
  //算術単項演算子
  NODE_FACTORY_DEFILE_NODE_CREATE_FUNC(Negative)
  NODE_FACTORY_DEFILE_NODE_CREATE_FUNC(Positive)

  //制約階層定義演算子
  NODE_FACTORY_DEFILE_NODE_CREATE_FUNC(Weaker)
  NODE_FACTORY_DEFILE_NODE_CREATE_FUNC(Parallel)

  // 時相演算子
  NODE_FACTORY_DEFILE_NODE_CREATE_FUNC(Always)

  //微分
  NODE_FACTORY_DEFILE_NODE_CREATE_FUNC(Differential)
  
  //左極限
  NODE_FACTORY_DEFILE_NODE_CREATE_FUNC(Previous)
  
  //直前のPPの値
  NODE_FACTORY_DEFILE_NODE_CREATE_FUNC(PreviousPoint)

  //変数・束縛変数
  NODE_FACTORY_DEFILE_NODE_CREATE_FUNC(Variable)

  //数字
  NODE_FACTORY_DEFILE_NODE_CREATE_FUNC(Number)
};                                                     

} //namespace parser
} //namespace hydla

#endif //_INCLUDED_HYDLA_PARSER_NODE_FACTORY_H_

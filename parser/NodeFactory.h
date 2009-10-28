#ifndef _INCLUDED_HYDLA_NODE_FACTORY_H_
#define _INCLUDED_HYDLA_NODE_FACTORY_H_

#include <string>
#include <vector>

#include <boost/shared_ptr.hpp>

#include "ParseTree.h"

namespace hydla { 
namespace parse_tree {

#define CREATE_NEW_PT_NODE(NAME) \
  virtual boost::shared_ptr<NAME> create##NAME() const { \
    boost::shared_ptr<NAME> p(new NAME()); \
    return p; \
  }

class NodeFactory {
public:
  NodeFactory()
  {}
 
  virtual ~NodeFactory()
  {}
  
  //定義
  CREATE_NEW_PT_NODE(ProgramDefinition)
  CREATE_NEW_PT_NODE(ConstraintDefinition)

  //呼び出し
  CREATE_NEW_PT_NODE(ProgramCaller)
  CREATE_NEW_PT_NODE(ConstraintCaller)
  
   //制約式
  CREATE_NEW_PT_NODE(Constraint);

  //Tell制約
  CREATE_NEW_PT_NODE(Tell)

  //Ask制約
  CREATE_NEW_PT_NODE(Ask)

  //比較演算子
  CREATE_NEW_PT_NODE(Equal)
  CREATE_NEW_PT_NODE(UnEqual)
  CREATE_NEW_PT_NODE(Less)
  CREATE_NEW_PT_NODE(LessEqual)
  CREATE_NEW_PT_NODE(Greater)
  CREATE_NEW_PT_NODE(GreaterEqual)

  //論理演算子
  CREATE_NEW_PT_NODE(LogicalAnd)
  CREATE_NEW_PT_NODE(LogicalOr)

  //算術二項演算子
  CREATE_NEW_PT_NODE(Plus)
  CREATE_NEW_PT_NODE(Subtract)
  CREATE_NEW_PT_NODE(Times)
  CREATE_NEW_PT_NODE(Divide)
  
  //算術単項演算子
  CREATE_NEW_PT_NODE(Negative)
  CREATE_NEW_PT_NODE(Positive)

  //制約階層定義演算子
  CREATE_NEW_PT_NODE(Weaker)
  CREATE_NEW_PT_NODE(Parallel)

  // 時相演算子
  CREATE_NEW_PT_NODE(Always)

  //微分
  CREATE_NEW_PT_NODE(Differential)
  
  //左極限
  CREATE_NEW_PT_NODE(Previous)

  //変数・束縛変数
  CREATE_NEW_PT_NODE(Variable)

  //数字
  CREATE_NEW_PT_NODE(Number)
};

} //namespace parse_tree
} //namespace hydla

#endif //_INCLUDED_HYDLA_NODE_FACTORY_H_

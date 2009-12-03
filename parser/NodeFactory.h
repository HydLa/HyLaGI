#ifndef _INCLUDED_HYDLA_NODE_FACTORY_H_
#define _INCLUDED_HYDLA_NODE_FACTORY_H_

#include <string>
#include <vector>

#include <boost/shared_ptr.hpp>
#include <boost/utility/enable_if.hpp> 
#include <boost/type_traits/is_same.hpp> 

#include "ParseTree.h"

namespace hydla { 
namespace parser {

#define DEFAULT_NODE_FACTORY_PT_NODE(NAME)              \
  boost::shared_ptr<hydla::parse_tree::NAME>            \
  operator()(hydla::parse_tree::NAME)                   \
  {                                                     \
    return boost::shared_ptr<hydla::parse_tree::NAME>(  \
      new hydla::parse_tree::NAME());                   \
  }                                                      

struct DefaultNodeFactory
{

  //定義
  DEFAULT_NODE_FACTORY_PT_NODE(ProgramDefinition)
  DEFAULT_NODE_FACTORY_PT_NODE(ConstraintDefinition)

  //呼び出し
  DEFAULT_NODE_FACTORY_PT_NODE(ProgramCaller)
  DEFAULT_NODE_FACTORY_PT_NODE(ConstraintCaller)
  
  //制約式
  DEFAULT_NODE_FACTORY_PT_NODE(Constraint);

  //Tell制約
  DEFAULT_NODE_FACTORY_PT_NODE(Tell)

  //Ask制約
  DEFAULT_NODE_FACTORY_PT_NODE(Ask)

  //比較演算子
  DEFAULT_NODE_FACTORY_PT_NODE(Equal)
  DEFAULT_NODE_FACTORY_PT_NODE(UnEqual)
  DEFAULT_NODE_FACTORY_PT_NODE(Less)
  DEFAULT_NODE_FACTORY_PT_NODE(LessEqual)
  DEFAULT_NODE_FACTORY_PT_NODE(Greater)
  DEFAULT_NODE_FACTORY_PT_NODE(GreaterEqual)

  //論理演算子
  DEFAULT_NODE_FACTORY_PT_NODE(LogicalAnd)
  DEFAULT_NODE_FACTORY_PT_NODE(LogicalOr)

  //算術二項演算子
  DEFAULT_NODE_FACTORY_PT_NODE(Plus)
  DEFAULT_NODE_FACTORY_PT_NODE(Subtract)
  DEFAULT_NODE_FACTORY_PT_NODE(Times)
  DEFAULT_NODE_FACTORY_PT_NODE(Divide)
  
  //算術単項演算子
  DEFAULT_NODE_FACTORY_PT_NODE(Negative)
  DEFAULT_NODE_FACTORY_PT_NODE(Positive)

  //制約階層定義演算子
  DEFAULT_NODE_FACTORY_PT_NODE(Weaker)
  DEFAULT_NODE_FACTORY_PT_NODE(Parallel)

  // 時相演算子
  DEFAULT_NODE_FACTORY_PT_NODE(Always)

  //微分
  DEFAULT_NODE_FACTORY_PT_NODE(Differential)
  
  //左極限
  DEFAULT_NODE_FACTORY_PT_NODE(Previous)

  //変数・束縛変数
  DEFAULT_NODE_FACTORY_PT_NODE(Variable)

  //数字
  DEFAULT_NODE_FACTORY_PT_NODE(Number)

};

} //namespace parse_tree
} //namespace hydla

#endif //_INCLUDED_HYDLA_NODE_FACTORY_H_

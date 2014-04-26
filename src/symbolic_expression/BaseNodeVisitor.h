#ifndef _INCLUDED_HYDLA_SYMBOLIC_EXPRESSION_BASE_NODE_VISITOR_H_
#define _INCLUDED_HYDLA_SYMBOLIC_EXPRESSION_BASE_NODE_VISITOR_H_

#include <boost/shared_ptr.hpp>

#include "Node.h"

namespace hydla { 
namespace symbolic_expression {
  
/**
 * ParseTreeのノード集合に対するVisitorクラス
 */
class BaseNodeVisitor {
public:
  BaseNodeVisitor();

  virtual ~BaseNodeVisitor();

  /**
   * Nodeクラスのaccept関数呼び出し用ヘルパ関数
   */
  template<class T>
  void accept(const T& n)
  {
    n->accept(n, this);
  }

  /// 因子ノードの呼び出し
  virtual void visit(boost::shared_ptr<FactorNode> node);
  
  /// 1つの子ノードを持つノードの呼び出し
  virtual void visit(boost::shared_ptr<UnaryNode> node);

  /// 2つの子ノードを持つノードの呼び出し
  virtual void visit(boost::shared_ptr<BinaryNode> node);
  
  /// 2つの子ノードを持つノードの呼び出し
  virtual void visit(boost::shared_ptr<ArbitraryNode> node);
};

} //namespace symbolic_expression
} //namespace hydla

#endif //_INCLUDED_HYDLA_SYMBOLIC_EXPRESSION_BASE_NODE_VISITOR_H_

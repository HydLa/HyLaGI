#pragma once
#include <memory>

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
  template <class T> void accept(const T &n) { n->accept(n, this); }

  /// 因子ノードの呼び出し
  virtual void visit(std::shared_ptr<FactorNode> node);

  /// 1つの子ノードを持つノードの呼び出し
  virtual void visit(std::shared_ptr<UnaryNode> node);

  /// 2つの子ノードを持つノードの呼び出し
  virtual void visit(std::shared_ptr<BinaryNode> node);

  /// 2つの子ノードを持つノードの呼び出し
  virtual void visit(std::shared_ptr<VariadicNode> node);
};

} // namespace symbolic_expression
} // namespace hydla

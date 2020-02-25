#pragma once

#include <boost/shared_ptr.hpp>

#include "Node.h"

namespace hydla {
namespace symbolic_expression {

/**
 * ParseTreeのノード集合に対するVisitorクラス
 */
class TreeVisitor {
public:
  TreeVisitor() {}

  virtual ~TreeVisitor() {}

  /**
   * Nodeクラスのaccept関数呼び出し用ヘルパ関数
   */

  virtual void accept(const std::shared_ptr<Node> &n) { n->accept(n, this); }

  // 制約定義
  virtual void visit(std::shared_ptr<ConstraintDefinition> node) = 0;

  // プログラム定義
  virtual void visit(std::shared_ptr<ProgramDefinition> node) = 0;

  // 制約呼び出し
  virtual void visit(std::shared_ptr<ConstraintCaller> node) = 0;

  // プログラム呼び出し
  virtual void visit(std::shared_ptr<ProgramCaller> node) = 0;

  // 制約式
  virtual void visit(std::shared_ptr<Constraint> node) = 0;

  // Ask制約
  virtual void visit(std::shared_ptr<Ask> node) = 0;

  // Exists
  virtual void visit(std::shared_ptr<Exists> node) = 0;

  // Tell制約
  virtual void visit(std::shared_ptr<Tell> node) = 0;

  // 比較演算子
  virtual void visit(std::shared_ptr<Equal> node) = 0;
  virtual void visit(std::shared_ptr<UnEqual> node) = 0;
  virtual void visit(std::shared_ptr<Less> node) = 0;
  virtual void visit(std::shared_ptr<LessEqual> node) = 0;
  virtual void visit(std::shared_ptr<Greater> node) = 0;
  virtual void visit(std::shared_ptr<GreaterEqual> node) = 0;

  // 論理演算子
  virtual void visit(std::shared_ptr<LogicalAnd> node) = 0;
  virtual void visit(std::shared_ptr<LogicalOr> node) = 0;

  // 算術二項演算子
  virtual void visit(std::shared_ptr<Plus> node) = 0;
  virtual void visit(std::shared_ptr<Subtract> node) = 0;
  virtual void visit(std::shared_ptr<Times> node) = 0;
  virtual void visit(std::shared_ptr<Divide> node) = 0;
  virtual void visit(std::shared_ptr<Power> node) = 0;

  // 算術単項演算子
  virtual void visit(std::shared_ptr<Negative> node) = 0;
  virtual void visit(std::shared_ptr<Positive> node) = 0;

  // 制約階層定義演算子
  virtual void visit(std::shared_ptr<Weaker> node) = 0;
  virtual void visit(std::shared_ptr<Parallel> node) = 0;

  // 時相演算子
  virtual void visit(std::shared_ptr<Always> node) = 0;

  // 微分
  virtual void visit(std::shared_ptr<Differential> node) = 0;

  // 左極限
  virtual void visit(std::shared_ptr<Previous> node) = 0;

  // Print
  virtual void visit(std::shared_ptr<Print> node) = 0;
  virtual void visit(std::shared_ptr<PrintPP> node) = 0;
  virtual void visit(std::shared_ptr<PrintIP> node) = 0;

  virtual void visit(std::shared_ptr<Scan> node) = 0;
  virtual void visit(std::shared_ptr<Exit> node) = 0;
  virtual void visit(std::shared_ptr<Abort> node) = 0;

  // SystemVariable
  virtual void visit(std::shared_ptr<SVtimer> node) = 0;

  // 否定
  virtual void visit(std::shared_ptr<Not> node) = 0;

  // 円周率
  virtual void visit(std::shared_ptr<Pi> node) = 0;
  // 自然対数の底
  virtual void visit(std::shared_ptr<E> node) = 0;

  // 関数
  virtual void visit(std::shared_ptr<Function> node) = 0;
  virtual void visit(std::shared_ptr<UnsupportedFunction> node) = 0;

  // 変数
  virtual void visit(std::shared_ptr<Variable> node) = 0;

  // 数字
  virtual void visit(std::shared_ptr<Number> node) = 0;

  virtual void visit(std::shared_ptr<ImaginaryUnit> node) = 0;

  // 浮動小数点数
  virtual void visit(std::shared_ptr<Float> node) = 0;

  // 記号定数
  virtual void visit(std::shared_ptr<Parameter> node) = 0;

  // t（時間）
  virtual void visit(std::shared_ptr<SymbolicT> node) = 0;

  // 無限大
  virtual void visit(std::shared_ptr<Infinity> node) = 0;

  // True
  virtual void visit(std::shared_ptr<True> node) = 0;

  // False
  virtual void visit(std::shared_ptr<False> node) = 0;

  // ExpressionList
  virtual void visit(std::shared_ptr<ExpressionList> node) = 0;

  // ConditionalExpressionList
  virtual void visit(std::shared_ptr<ConditionalExpressionList> node) = 0;

  // ProgramList
  virtual void visit(std::shared_ptr<ProgramList> node) = 0;

  // ConditionalProgramList
  virtual void visit(std::shared_ptr<ConditionalProgramList> node) = 0;

  // EachElement
  virtual void visit(std::shared_ptr<EachElement> node) = 0;

  // DifferentVariable
  virtual void visit(std::shared_ptr<DifferentVariable> node) = 0;

  // ExpressionListElement
  virtual void visit(std::shared_ptr<ExpressionListElement> node) = 0;

  // ExpressionListCaller
  virtual void visit(std::shared_ptr<ExpressionListCaller> node) = 0;

  // ExpressionListDefinition
  virtual void visit(std::shared_ptr<ExpressionListDefinition> node) = 0;

  // ProgramListCaller
  virtual void visit(std::shared_ptr<ProgramListCaller> node) = 0;

  // ProgramListDefinition
  virtual void visit(std::shared_ptr<ProgramListDefinition> node) = 0;

  // ProgramListElement
  virtual void visit(std::shared_ptr<ProgramListElement> node) = 0;

  // SumOfList
  virtual void visit(std::shared_ptr<SumOfList> node) = 0;

  // MulOfList
  virtual void visit(std::shared_ptr<MulOfList> node) = 0;

  // SizeOfList
  virtual void visit(std::shared_ptr<SizeOfList> node) = 0;

  // Range
  virtual void visit(std::shared_ptr<Range> node) = 0;

  // Union
  virtual void visit(std::shared_ptr<Union> node) = 0;

  // Intersection
  virtual void visit(std::shared_ptr<Intersection> node) = 0;
};

} // namespace symbolic_expression
} // namespace hydla

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
  TreeVisitor(){}

  virtual ~TreeVisitor(){}

  /**
   * Nodeクラスのaccept関数呼び出し用ヘルパ関数
   */

  virtual void accept(const boost::shared_ptr<Node>& n)
  {
    n->accept(n, this);
  }

  // 制約定義
  virtual void visit(boost::shared_ptr<ConstraintDefinition> node) = 0;
  
  // プログラム定義
  virtual void visit(boost::shared_ptr<ProgramDefinition> node) = 0;

  // 制約呼び出し
  virtual void visit(boost::shared_ptr<ConstraintCaller> node) = 0;
  
  // プログラム呼び出し
  virtual void visit(boost::shared_ptr<ProgramCaller> node) = 0;

  // 制約式
  virtual void visit(boost::shared_ptr<Constraint> node) = 0;

  // Ask制約
  virtual void visit(boost::shared_ptr<Ask> node) = 0;

  // Tell制約
  virtual void visit(boost::shared_ptr<Tell> node) = 0;

  // 比較演算子
  virtual void visit(boost::shared_ptr<Equal> node) = 0;
  virtual void visit(boost::shared_ptr<UnEqual> node) = 0;
  virtual void visit(boost::shared_ptr<Less> node) = 0;
  virtual void visit(boost::shared_ptr<LessEqual> node) = 0;
  virtual void visit(boost::shared_ptr<Greater> node) = 0;
  virtual void visit(boost::shared_ptr<GreaterEqual> node) = 0;

  // 論理演算子
  virtual void visit(boost::shared_ptr<LogicalAnd> node) = 0;
  virtual void visit(boost::shared_ptr<LogicalOr> node) = 0;
  
  // 算術二項演算子
  virtual void visit(boost::shared_ptr<Plus> node) = 0;
  virtual void visit(boost::shared_ptr<Subtract> node) = 0;
  virtual void visit(boost::shared_ptr<Times> node) = 0;
  virtual void visit(boost::shared_ptr<Divide> node) = 0;
  virtual void visit(boost::shared_ptr<Power> node) = 0;
  
  // 算術単項演算子
  virtual void visit(boost::shared_ptr<Negative> node) = 0;
  virtual void visit(boost::shared_ptr<Positive> node) = 0;
  
  // 制約階層定義演算子
  virtual void visit(boost::shared_ptr<Weaker> node) = 0;
  virtual void visit(boost::shared_ptr<Parallel> node) = 0;

  // 時相演算子
  virtual void visit(boost::shared_ptr<Always> node) = 0;
  
  // 微分
  virtual void visit(boost::shared_ptr<Differential> node) = 0;

  // 左極限
  virtual void visit(boost::shared_ptr<Previous> node) = 0;
  
  //Print
  virtual void visit(boost::shared_ptr<Print> node) = 0;
  virtual void visit(boost::shared_ptr<PrintPP> node) = 0;
  virtual void visit(boost::shared_ptr<PrintIP> node) = 0;
    
  virtual void visit(boost::shared_ptr<Scan> node) = 0;
  virtual void visit(boost::shared_ptr<Exit> node) = 0;
  virtual void visit(boost::shared_ptr<Abort> node) = 0;

  //SystemVariable
  virtual void visit(boost::shared_ptr<SVtimer> node) = 0;
  
  // 否定
  virtual void visit(boost::shared_ptr<Not> node) = 0;
  
  // 円周率
  virtual void visit(boost::shared_ptr<Pi> node) = 0;
  // 自然対数の底
  virtual void visit(boost::shared_ptr<E> node) = 0;
  
  // 関数
  virtual void visit(boost::shared_ptr<Function> node) = 0;
  virtual void visit(boost::shared_ptr<UnsupportedFunction> node) = 0;

  // 変数
  virtual void visit(boost::shared_ptr<Variable> node) = 0;

  // 数字
  virtual void visit(boost::shared_ptr<Number> node) = 0;

  // 浮動小数点数
  virtual void visit(boost::shared_ptr<Float> node) = 0;
  
  // 記号定数
  virtual void visit(boost::shared_ptr<Parameter> node) = 0;
  
  // t（時間）
  virtual void visit(boost::shared_ptr<SymbolicT> node) = 0;
  
  // 無限大
  virtual void visit(boost::shared_ptr<Infinity> node) = 0;

  // True
  virtual void visit(boost::shared_ptr<True> node) = 0;

  // False
  virtual void visit(boost::shared_ptr<False> node) = 0;

  // ExpressionList
  virtual void visit(boost::shared_ptr<ExpressionList> node) = 0;

  // ConditionalExpressionList
  virtual void visit(boost::shared_ptr<ConditionalExpressionList> node) = 0;

  // ProgramList
  virtual void visit(boost::shared_ptr<ProgramList> node) = 0;

  // ConditionalProgramList
  virtual void visit(boost::shared_ptr<ConditionalProgramList> node) = 0;

  // EachElement
  virtual void visit(boost::shared_ptr<EachElement> node) = 0;

  // DifferentVariable
  virtual void visit(boost::shared_ptr<DifferentVariable> node) = 0;

};

} //namespace symbolic_expression
} //namespace hydla

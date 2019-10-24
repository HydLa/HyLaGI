#pragma once

#include "TreeVisitor.h"

namespace hydla { 
namespace symbolic_expression {
  
/**
 * 各ノードに対して全子ノードを走査するクラス．
 * 継承先で各visitをオーバーライドして新たなVisitorを作る．
 */
class DefaultTreeVisitor: public TreeVisitor {
public:
  DefaultTreeVisitor();

  virtual ~DefaultTreeVisitor();

  // 制約定義
  virtual void visit(std::shared_ptr<ConstraintDefinition> node);
  
  // プログラム定義
  virtual void visit(std::shared_ptr<ProgramDefinition> node);

  // 制約呼び出し
  virtual void visit(std::shared_ptr<ConstraintCaller> node);
  
  // プログラム呼び出し
  virtual void visit(std::shared_ptr<ProgramCaller> node);

  // 制約式
  virtual void visit(std::shared_ptr<Constraint> node);

  // Ask制約
  virtual void visit(std::shared_ptr<Ask> node);

  // Tell制約
  virtual void visit(std::shared_ptr<Tell> node);

  // 比較演算子
  virtual void visit(std::shared_ptr<Equal> node);
  virtual void visit(std::shared_ptr<UnEqual> node);
  virtual void visit(std::shared_ptr<Less> node);
  virtual void visit(std::shared_ptr<LessEqual> node);
  virtual void visit(std::shared_ptr<Greater> node);
  virtual void visit(std::shared_ptr<GreaterEqual> node);

  // 論理演算子
  virtual void visit(std::shared_ptr<LogicalAnd> node);
  virtual void visit(std::shared_ptr<LogicalOr> node);
  
  // 算術二項演算子
  virtual void visit(std::shared_ptr<Plus> node);
  virtual void visit(std::shared_ptr<Subtract> node);
  virtual void visit(std::shared_ptr<Times> node);
  virtual void visit(std::shared_ptr<Divide> node);
  virtual void visit(std::shared_ptr<Power> node);
  
  // 算術単項演算子
  virtual void visit(std::shared_ptr<Negative> node);
  virtual void visit(std::shared_ptr<Positive> node);
  
  // 制約階層定義演算子
  virtual void visit(std::shared_ptr<Weaker> node);
  virtual void visit(std::shared_ptr<Parallel> node);

  // 時相演算子
  virtual void visit(std::shared_ptr<Always> node);
  
  // 微分
  virtual void visit(std::shared_ptr<Differential> node);

  // 左極限
  virtual void visit(std::shared_ptr<Previous> node);
  
  //Print
  virtual void visit(std::shared_ptr<Print> node);
  virtual void visit(std::shared_ptr<PrintPP> node);
  virtual void visit(std::shared_ptr<PrintIP> node);
    
  virtual void visit(std::shared_ptr<Scan> node);
  virtual void visit(std::shared_ptr<Exit> node);
  virtual void visit(std::shared_ptr<Abort> node);

  //SystemVariable
  virtual void visit(std::shared_ptr<SVtimer> node);

  // 否定
  virtual void visit(std::shared_ptr<Not> node);
  
  // 円周率
  virtual void visit(std::shared_ptr<Pi> node);
  // 自然対数の底
  virtual void visit(std::shared_ptr<E> node);
  // True
  virtual void visit(std::shared_ptr<True> node);
  // False
  virtual void visit(std::shared_ptr<False> node);
  
  //関数
  virtual void visit(std::shared_ptr<Function> node);
  virtual void visit(std::shared_ptr<UnsupportedFunction> node);


  // 変数
  virtual void visit(std::shared_ptr<Variable> node);

  // 数字
  virtual void visit(std::shared_ptr<Number> node);
  virtual void visit(std::shared_ptr<Float> node);
  
  // 記号定数
  virtual void visit(std::shared_ptr<Parameter> node);
  
  // t（時間）
  virtual void visit(std::shared_ptr<SymbolicT> node);

  // Infinity
  virtual void visit(std::shared_ptr<Infinity> node);

  virtual void visit(std::shared_ptr<ImaginaryUnit> node);

  // ExpressionList
  virtual void visit(std::shared_ptr<ExpressionList> node);

  // ConditionalExpressionList
  virtual void visit(std::shared_ptr<ConditionalExpressionList> node);

  // ProgramList
  virtual void visit(std::shared_ptr<ProgramList> node);

  // ConditionalProgramList
  virtual void visit(std::shared_ptr<ConditionalProgramList> node);

  // EachElement
  virtual void visit(std::shared_ptr<EachElement> node);

  // DifferentVariable
  virtual void visit(std::shared_ptr<DifferentVariable> node);

  // ProgramListDefinition 
  virtual void visit(std::shared_ptr<ProgramListDefinition> node);

  // ExpressionListDefinition 
  virtual void visit(std::shared_ptr<ExpressionListDefinition> node);

  // ExpressionListCaller 
  virtual void visit(std::shared_ptr<ExpressionListCaller> node);

  // ExpressionListElement 
  virtual void visit(std::shared_ptr<ExpressionListElement> node);

  // ProgramListCaller 
  virtual void visit(std::shared_ptr<ProgramListCaller> node);

  // ProgramListElement 
  virtual void visit(std::shared_ptr<ProgramListElement> node);

  // Union 
  virtual void visit(std::shared_ptr<Union> node);

  // Intersection 
  virtual void visit(std::shared_ptr<Intersection> node);

  // Range 
  virtual void visit(std::shared_ptr<Range> node);

  // SizeOfLIst 
  virtual void visit(std::shared_ptr<SizeOfList> node);

  // SumOfList
  virtual void visit(std::shared_ptr<SumOfList> node);

  // MulOfList
  virtual void visit(std::shared_ptr<MulOfList> node);

};

} //namespace symbolic_expression
} //namespace hydla


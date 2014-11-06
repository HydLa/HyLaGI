#pragma once

#include "TreeVisitor.h"

namespace hydla{
namespace symbolic_expression{

class NodeReplacer : public TreeVisitor
{
public:
  NodeReplacer();
  virtual ~NodeReplacer();

  void set_source(node_sptr& node)
  {
    source_ = node;
  }

  void set_dest(node_sptr& node)
  {
    dest_ = node;
  }

  void replace(node_sptr& node)
  {
    accept(node);
    if(new_child_) node = new_child_;
    new_child_.reset();
  }

  virtual void visit(boost::shared_ptr<ConstraintDefinition> node);
  
  // プログラム定義
  virtual void visit(boost::shared_ptr<ProgramDefinition> node);

  // 制約呼び出し
  virtual void visit(boost::shared_ptr<ConstraintCaller> node);
  
  // プログラム呼び出し
  virtual void visit(boost::shared_ptr<ProgramCaller> node);

  // 制約式
  virtual void visit(boost::shared_ptr<Constraint> node);

  // Ask制約
  virtual void visit(boost::shared_ptr<Ask> node);

  // Tell制約
  virtual void visit(boost::shared_ptr<Tell> node);

  // 比較演算子
  virtual void visit(boost::shared_ptr<Equal> node);
  virtual void visit(boost::shared_ptr<UnEqual> node);
  virtual void visit(boost::shared_ptr<Less> node);
  virtual void visit(boost::shared_ptr<LessEqual> node);
  virtual void visit(boost::shared_ptr<Greater> node);
  virtual void visit(boost::shared_ptr<GreaterEqual> node);

  // 論理演算子
  virtual void visit(boost::shared_ptr<LogicalAnd> node);
  virtual void visit(boost::shared_ptr<LogicalOr> node);
  
  // 算術二項演算子
  virtual void visit(boost::shared_ptr<Plus> node);
  virtual void visit(boost::shared_ptr<Subtract> node);
  virtual void visit(boost::shared_ptr<Times> node);
  virtual void visit(boost::shared_ptr<Divide> node);
  virtual void visit(boost::shared_ptr<Power> node);
  
  // 算術単項演算子
  virtual void visit(boost::shared_ptr<Negative> node);
  virtual void visit(boost::shared_ptr<Positive> node);
  
  // 制約階層定義演算子
  virtual void visit(boost::shared_ptr<Weaker> node);
  virtual void visit(boost::shared_ptr<Parallel> node);

  // 時相演算子
  virtual void visit(boost::shared_ptr<Always> node);
  
  // 微分
  virtual void visit(boost::shared_ptr<Differential> node);

  // 左極限
  virtual void visit(boost::shared_ptr<Previous> node);
  
  //Print
  virtual void visit(boost::shared_ptr<Print> node);
  virtual void visit(boost::shared_ptr<PrintPP> node);
  virtual void visit(boost::shared_ptr<PrintIP> node);
    
  virtual void visit(boost::shared_ptr<Scan> node);
  virtual void visit(boost::shared_ptr<Exit> node);
  virtual void visit(boost::shared_ptr<Abort> node);

  //SystemVariable
  virtual void visit(boost::shared_ptr<SVtimer> node);
  
  // 否定
  virtual void visit(boost::shared_ptr<Not> node);
  
  // 円周率
  virtual void visit(boost::shared_ptr<Pi> node);
  // 自然対数の底
  virtual void visit(boost::shared_ptr<E> node);
  
  // 関数
  virtual void visit(boost::shared_ptr<Function> node);
  virtual void visit(boost::shared_ptr<UnsupportedFunction> node);

  // 変数
  virtual void visit(boost::shared_ptr<Variable> node);

  // 数字
  virtual void visit(boost::shared_ptr<Number> node);

  // 浮動小数点数
  virtual void visit(boost::shared_ptr<Float> node);
  
  // 記号定数
  virtual void visit(boost::shared_ptr<Parameter> node);
  
  // t（時間）
  virtual void visit(boost::shared_ptr<SymbolicT> node);
  
  // 無限大
  virtual void visit(boost::shared_ptr<Infinity> node);

  // True
  virtual void visit(boost::shared_ptr<True> node);

  // False
  virtual void visit(boost::shared_ptr<False> node);

  // ExpressionList
  virtual void visit(boost::shared_ptr<ExpressionList> node);

  // ConditionalExpressionList
  virtual void visit(boost::shared_ptr<ConditionalExpressionList> node);

  // ProgramList
  virtual void visit(boost::shared_ptr<ProgramList> node);

  // ConditionalProgramList
  virtual void visit(boost::shared_ptr<ConditionalProgramList> node);

  // EachElement
  virtual void visit(boost::shared_ptr<EachElement> node);

  // DifferentVariable
  virtual void visit(boost::shared_ptr<DifferentVariable> node);

  // ExpressionListElement
  virtual void visit(boost::shared_ptr<ExpressionListElement> node);

  // ExpressionListCaller
  virtual void visit(boost::shared_ptr<ExpressionListCaller> node);

  // ExpressionListDefinition
  virtual void visit(boost::shared_ptr<ExpressionListDefinition> node);
  
  // ProgramListCaller
  virtual void visit(boost::shared_ptr<ProgramListCaller> node);

  // ProgramListDefinition
  virtual void visit(boost::shared_ptr<ProgramListDefinition> node);

  // ProgramListElement
  virtual void visit(boost::shared_ptr<ProgramListElement> node);

  // SumOfList 
  virtual void visit(boost::shared_ptr<SumOfList> node);

  // SizeOfList 
  virtual void visit(boost::shared_ptr<SizeOfList> node);

  // Range
  virtual void visit(boost::shared_ptr<Range> node);

  // Union
  virtual void visit(boost::shared_ptr<Union> node);

  // Intersection 
  virtual void visit(boost::shared_ptr<Intersection> node);

private:
  node_sptr source_;
  node_sptr dest_;
  node_sptr new_child_;
};



} //namespace symbolic_expression 
} //namespace hydla

#pragma once

#include "Node.h"
#include "TreeVisitor.h"
#include "ParseTree.h"
#include "../io/picojson.h"

namespace hydla { 
namespace parser {
  
/**
 * ParseTree を JSON 形式でダンプする
 */
class ParseTreeJsonDumper : 
  public symbolic_expression::TreeVisitor
{
public:
  typedef symbolic_expression::node_sptr node_sptr;

  ParseTreeJsonDumper();

  virtual ~ParseTreeJsonDumper();

  /**
   * JSON 言語形式での出力をおこなう
   */
  std::ostream& dump(std::ostream& s, const symbolic_expression::node_sptr& node);
  
  // 定義
  virtual void visit(std::shared_ptr<symbolic_expression::ConstraintDefinition> node);
  virtual void visit(std::shared_ptr<symbolic_expression::ProgramDefinition> node);
  // 呼び出し
  virtual void visit(std::shared_ptr<symbolic_expression::ConstraintCaller> node);
  virtual void visit(std::shared_ptr<symbolic_expression::ProgramCaller> node);

  // 制約式
  virtual void visit(std::shared_ptr<symbolic_expression::Constraint> node);

  // Ask 制約
  virtual void visit(std::shared_ptr<symbolic_expression::Ask> node);

  // Tell 制約
  virtual void visit(std::shared_ptr<symbolic_expression::Tell> node);

  // 比較演算子
  virtual void visit(std::shared_ptr<symbolic_expression::Equal> node);
  virtual void visit(std::shared_ptr<symbolic_expression::UnEqual> node);
  virtual void visit(std::shared_ptr<symbolic_expression::Less> node);
  virtual void visit(std::shared_ptr<symbolic_expression::LessEqual> node);
  virtual void visit(std::shared_ptr<symbolic_expression::Greater> node);
  virtual void visit(std::shared_ptr<symbolic_expression::GreaterEqual> node);

  // 論理演算子
  virtual void visit(std::shared_ptr<symbolic_expression::LogicalAnd> node);
  virtual void visit(std::shared_ptr<symbolic_expression::LogicalOr> node);
  virtual void visit(std::shared_ptr<symbolic_expression::Not> node);
  
  // 算術二項演算子
  virtual void visit(std::shared_ptr<symbolic_expression::Plus> node);
  virtual void visit(std::shared_ptr<symbolic_expression::Subtract> node);
  virtual void visit(std::shared_ptr<symbolic_expression::Times> node);
  virtual void visit(std::shared_ptr<symbolic_expression::Divide> node);
  virtual void visit(std::shared_ptr<symbolic_expression::Power> node);
  
  // 算術単項演算子
  virtual void visit(std::shared_ptr<symbolic_expression::Negative> node);
  virtual void visit(std::shared_ptr<symbolic_expression::Positive> node);
  
  // 制約階層定義演算子
  virtual void visit(std::shared_ptr<symbolic_expression::Weaker> node);
  virtual void visit(std::shared_ptr<symbolic_expression::Parallel> node);
  
  // 時相演算子
  virtual void visit(std::shared_ptr<symbolic_expression::Always> node);
  
  // 微分
  virtual void visit(std::shared_ptr<symbolic_expression::Differential> node);

  // 左極限
  virtual void visit(std::shared_ptr<symbolic_expression::Previous> node);
 
  // Print 
  virtual void visit(std::shared_ptr<symbolic_expression::Print> node);
  virtual void visit(std::shared_ptr<symbolic_expression::PrintPP> node);
  virtual void visit(std::shared_ptr<symbolic_expression::PrintIP> node);
  virtual void visit(std::shared_ptr<symbolic_expression::Scan> node);
  virtual void visit(std::shared_ptr<symbolic_expression::Exit> node);
  virtual void visit(std::shared_ptr<symbolic_expression::Abort> node);

  // SystemVariable
  virtual void visit(std::shared_ptr<symbolic_expression::SVtimer> node);

  virtual void visit(std::shared_ptr<symbolic_expression::True> node);
  
  virtual void visit(std::shared_ptr<symbolic_expression::False> node);
  // 関数
  virtual void visit(std::shared_ptr<symbolic_expression::Function> node);
  virtual void visit(std::shared_ptr<symbolic_expression::UnsupportedFunction> node);
  
  // 円周率
  virtual void visit(std::shared_ptr<symbolic_expression::Pi> node);
  // 自然対数の底
  virtual void visit(std::shared_ptr<symbolic_expression::E> node);
  // 変数
  virtual void visit(std::shared_ptr<symbolic_expression::Variable> node);

  // 数字
  virtual void visit(std::shared_ptr<symbolic_expression::Number> node);
  virtual void visit(std::shared_ptr<symbolic_expression::Float> node);

  virtual void visit(std::shared_ptr<symbolic_expression::ImaginaryUnit> node);

  // Parameter
  virtual void visit(std::shared_ptr<symbolic_expression::Parameter> node);
  // SymbolicT
  virtual void visit(std::shared_ptr<symbolic_expression::SymbolicT> node);
  // Infinity
  virtual void visit(std::shared_ptr<symbolic_expression::Infinity> node);
  // ExpressionList
  virtual void visit(std::shared_ptr<symbolic_expression::ExpressionList> node);
  // ConditionalExpressionList
  virtual void visit(std::shared_ptr<symbolic_expression::ConditionalExpressionList> node);
  // ProgramList
  virtual void visit(std::shared_ptr<symbolic_expression::ProgramList> node);
  // ConditionalProgramList
  virtual void visit(std::shared_ptr<symbolic_expression::ConditionalProgramList> node);
  // EachElement
  virtual void visit(std::shared_ptr<symbolic_expression::EachElement> node);
  // DifferentVariable
  virtual void visit(std::shared_ptr<symbolic_expression::DifferentVariable> node);
  // ExpressionListElement
  virtual void visit(std::shared_ptr<symbolic_expression::ExpressionListElement> node);
  // ExpressionListCaller
  virtual void visit(std::shared_ptr<symbolic_expression::ExpressionListCaller> node);
  // ExpressionListDefinition
  virtual void visit(std::shared_ptr<symbolic_expression::ExpressionListDefinition> node);
  // ProgramListCaller
  virtual void visit(std::shared_ptr<symbolic_expression::ProgramListCaller> node);
  // ProgramListDefinition
  virtual void visit(std::shared_ptr<symbolic_expression::ProgramListDefinition> node);
  // ProgramListElement
  virtual void visit(std::shared_ptr<symbolic_expression::ProgramListElement> node);
  // Union 
  virtual void visit(std::shared_ptr<symbolic_expression::Union> node);
  // Intersection 
  virtual void visit(std::shared_ptr<symbolic_expression::Intersection> node);
  // Range 
  virtual void visit(std::shared_ptr<symbolic_expression::Range> node);
  // SizeOfList 
  virtual void visit(std::shared_ptr<symbolic_expression::SizeOfList> node);
  // SumOfList
  virtual void visit(std::shared_ptr<symbolic_expression::SumOfList> node);
  // MulOfList
  virtual void visit(std::shared_ptr<symbolic_expression::MulOfList> node);

private:
  void dump_node(std::shared_ptr<symbolic_expression::FactorNode> node);
  void dump_node(std::shared_ptr<symbolic_expression::UnaryNode> node);
  void dump_node(std::shared_ptr<symbolic_expression::BinaryNode> node);
  void dump_node(std::shared_ptr<symbolic_expression::VariadicNode> node);

 // JSON object
  picojson::value json_;
  picojson::value * current_ = nullptr;
};

} // namespace parser
} // namespace hydla


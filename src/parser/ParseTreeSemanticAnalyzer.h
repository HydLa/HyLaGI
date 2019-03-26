#pragma once

#include <stack>

#include "Node.h"
#include "TreeVisitor.h"
#include "DefinitionContainer.h"
#include "ParseTree.h"
#include "ListExpander.h"

namespace hydla { 
namespace parser {

class ParseTreeSemanticAnalyzer : 
  public symbolic_expression::TreeVisitor
{
public:
  typedef symbolic_expression::node_sptr                 node_sptr;

  ParseTreeSemanticAnalyzer(
    DefinitionContainer<symbolic_expression::ConstraintDefinition>&,
    DefinitionContainer<symbolic_expression::ProgramDefinition>&,
    DefinitionContainer<symbolic_expression::ExpressionListDefinition>&,
    DefinitionContainer<symbolic_expression::ProgramListDefinition>&,
    parse_tree::ParseTree* parse_tree);
  
  virtual ~ParseTreeSemanticAnalyzer();

  /**
   * 解析および制約呼び出しの展開をおこなう
   */
  void analyze(symbolic_expression::node_sptr& n);

private:

  // 定義
  virtual void visit(boost::shared_ptr<symbolic_expression::ConstraintDefinition> node);
  virtual void visit(boost::shared_ptr<symbolic_expression::ProgramDefinition> node);

  // 呼び出し
  virtual void visit(boost::shared_ptr<symbolic_expression::ConstraintCaller> node);
  virtual void visit(boost::shared_ptr<symbolic_expression::ProgramCaller> node);

  // 制約式
  virtual void visit(boost::shared_ptr<symbolic_expression::Constraint> node);

  // Ask制約
  virtual void visit(boost::shared_ptr<symbolic_expression::Ask> node);

  // Tell制約
  virtual void visit(boost::shared_ptr<symbolic_expression::Tell> node);

  // 比較演算子
  virtual void visit(boost::shared_ptr<symbolic_expression::Equal> node);
  virtual void visit(boost::shared_ptr<symbolic_expression::UnEqual> node);
  virtual void visit(boost::shared_ptr<symbolic_expression::Less> node);
  virtual void visit(boost::shared_ptr<symbolic_expression::LessEqual> node);
  virtual void visit(boost::shared_ptr<symbolic_expression::Greater> node);
  virtual void visit(boost::shared_ptr<symbolic_expression::GreaterEqual> node);

  // 論理演算子
  virtual void visit(boost::shared_ptr<symbolic_expression::LogicalAnd> node);
  virtual void visit(boost::shared_ptr<symbolic_expression::LogicalOr> node);
  virtual void visit(boost::shared_ptr<symbolic_expression::Not> node);
  
  // 算術二項演算子
  virtual void visit(boost::shared_ptr<symbolic_expression::Plus> node);
  virtual void visit(boost::shared_ptr<symbolic_expression::Subtract> node);
  virtual void visit(boost::shared_ptr<symbolic_expression::Times> node);
  virtual void visit(boost::shared_ptr<symbolic_expression::Divide> node);
  virtual void visit(boost::shared_ptr<symbolic_expression::Power> node);
  
  // 算術単項演算子
  virtual void visit(boost::shared_ptr<symbolic_expression::Negative> node);
  virtual void visit(boost::shared_ptr<symbolic_expression::Positive> node);
  
  // 制約階層定義演算子
  virtual void visit(boost::shared_ptr<symbolic_expression::Weaker> node);
  virtual void visit(boost::shared_ptr<symbolic_expression::Parallel> node);

  // 時相演算子
  virtual void visit(boost::shared_ptr<symbolic_expression::Always> node);
  
  // 微分
  virtual void visit(boost::shared_ptr<symbolic_expression::Differential> node);

  // 左極限
  virtual void visit(boost::shared_ptr<symbolic_expression::Previous> node);
  
  // 関数
  virtual void visit(boost::shared_ptr<symbolic_expression::Function> node);
  virtual void visit(boost::shared_ptr<symbolic_expression::UnsupportedFunction> node);

  // 円周率
  virtual void visit(boost::shared_ptr<symbolic_expression::Pi> node);
  // 自然対数の底
  virtual void visit(boost::shared_ptr<symbolic_expression::E> node);
  
  
  // 変数
  virtual void visit(boost::shared_ptr<symbolic_expression::Variable> node);

  // 数字
  virtual void visit(boost::shared_ptr<symbolic_expression::Number> node);
  virtual void visit(boost::shared_ptr<symbolic_expression::Float> node);


  // Print
  virtual void visit(boost::shared_ptr<symbolic_expression::Print> node);
  virtual void visit(boost::shared_ptr<symbolic_expression::PrintPP> node);
  virtual void visit(boost::shared_ptr<symbolic_expression::PrintIP> node);
  virtual void visit(boost::shared_ptr<symbolic_expression::Scan> node);
  virtual void visit(boost::shared_ptr<symbolic_expression::Exit> node);
  virtual void visit(boost::shared_ptr<symbolic_expression::Abort> node);

  //Parameter
  virtual void visit(boost::shared_ptr<symbolic_expression::Parameter> node){/*assert(0);*/}
  //Infinity
  virtual void visit(boost::shared_ptr<symbolic_expression::Infinity> node);
  //SymbolicT
  virtual void visit(boost::shared_ptr<symbolic_expression::SymbolicT> node);

  virtual void visit(boost::shared_ptr<symbolic_expression::ImaginaryUnit> node);

  //SystemVariable
  virtual void visit(boost::shared_ptr<symbolic_expression::SVtimer> node);
  
  //True
  virtual void visit(boost::shared_ptr<symbolic_expression::True> node);
 
  //False
  virtual void visit(boost::shared_ptr<symbolic_expression::False> node);

  // ExpressionList
  virtual void visit(boost::shared_ptr<symbolic_expression::ExpressionList> node);

  // ConditionalExpressionList
  virtual void visit(boost::shared_ptr<symbolic_expression::ConditionalExpressionList> node);

  // ProgramList
  virtual void visit(boost::shared_ptr<symbolic_expression::ProgramList> node);

  // ConditionalProgramList
  virtual void visit(boost::shared_ptr<symbolic_expression::ConditionalProgramList> node);

  // EachElement
  virtual void visit(boost::shared_ptr<symbolic_expression::EachElement> node);

  // DifferentVariable
  virtual void visit(boost::shared_ptr<symbolic_expression::DifferentVariable> node);

  // ExpressionListElement
  virtual void visit(boost::shared_ptr<symbolic_expression::ExpressionListElement> node);

  // ExpressionListCaller
  virtual void visit(boost::shared_ptr<symbolic_expression::ExpressionListCaller> node);

  // ExpressionListDefinition
  virtual void visit(boost::shared_ptr<symbolic_expression::ExpressionListDefinition> node);

  // ProgramListDefinition
  virtual void visit(boost::shared_ptr<symbolic_expression::ProgramListDefinition> node);

  // ProgramListCaller
  virtual void visit(boost::shared_ptr<symbolic_expression::ProgramListCaller> node);

  // ProgramListElement
  virtual void visit(boost::shared_ptr<symbolic_expression::ProgramListElement> node);

  // SizeOfList
  virtual void visit(boost::shared_ptr<symbolic_expression::SizeOfList> node);

  // SumOfList
  virtual void visit(boost::shared_ptr<symbolic_expression::SumOfList> node);

  // MulOfList
  virtual void visit(boost::shared_ptr<symbolic_expression::MulOfList> node);

  // Range
  virtual void visit(boost::shared_ptr<symbolic_expression::Range> node);

  // Union
  virtual void visit(boost::shared_ptr<symbolic_expression::Union> node);

  // Intersection 
  virtual void visit(boost::shared_ptr<symbolic_expression::Intersection> node);

  typedef parser::DefinitionContainer<
    symbolic_expression::Definition>::definition_map_key_t referenced_definition_t;

  typedef std::set<referenced_definition_t>         referenced_definition_list_t;
  
  typedef std::map<std::string, symbolic_expression::node_sptr>        formal_arg_map_t;

  /**
   * 探索中のノードツリーの状態を保存するための構造体
   */
  struct State {
    /// ガードの中かどうか
    bool in_guard;

    /// 制約式の中かどうか
    bool in_constraint;

    /// always制約の有効範囲内かどうか
    bool in_always;

    /// 微分記号を通過した回数
    /// 変数に到達した際、この値がその変数に対する微分の最大回数
    int differential_count;

    /// 展開された定義のリスト 
    referenced_definition_list_t referenced_definition_list;

    /// 仮引数とそれに対応する実引数ノードの対応表  
    formal_arg_map_t formal_arg_map;
  };

  /// Stateをつむためのスタック
  std::stack<State> todo_stack_;

  std::set<boost::shared_ptr<symbolic_expression::ConstraintDefinition> > unused_constraint_definition;
  std::set<boost::shared_ptr<symbolic_expression::ProgramDefinition> >  unused_program_definition;
  std::set<boost::shared_ptr<symbolic_expression::ExpressionListDefinition> >  unused_expression_list_definition;
  std::set<boost::shared_ptr<symbolic_expression::ProgramListDefinition> >  unused_program_list_definition;

  /**
   * 新しい子ノード
   * accept後、これに値が入っている場合はノードの値を交換する
   */
  symbolic_expression::node_sptr new_child_;

  /**
   * 制約定義の情報
   */
  DefinitionContainer<symbolic_expression::ConstraintDefinition>& 
    constraint_definition_;
    
  /**
   * プログラム定義の情報
   */
  DefinitionContainer<symbolic_expression::ProgramDefinition>&    
    program_definition_;

  /**
   * 式リスト定義の情報
   */
  DefinitionContainer<symbolic_expression::ExpressionListDefinition>& 
    expression_list_definition_;

  /**
   * プログラムリスト定義の情報
   */
  DefinitionContainer<symbolic_expression::ProgramListDefinition>& 
    program_list_definition_;

  std::stack<std::map<node_sptr, node_sptr> > local_variables_in_list_;

  

  ListExpander list_expander_;

  parse_tree::ParseTree* parse_tree_;
  
  /**
   * 指定したノードを呼び出し、
   * new_child_に値が設定されていた場合、子ノードを入れ替える
   */
  template<class C, 
           const symbolic_expression::node_sptr& (C::*getter)() const,
           void (C::*setter)(const symbolic_expression::node_sptr& child)>
  void dispatch(C* n) 
  {
    accept((n->*getter)());
    if(new_child_) {
      (n->*setter)(new_child_);
      new_child_.reset();
    }
  }
  
  template<class NodeType>
  void dispatch_child(NodeType& node)
  {
    dispatch<symbolic_expression::UnaryNode, 
      &symbolic_expression::UnaryNode::get_child, 
      &symbolic_expression::UnaryNode::set_child>(node.get());
  }

  template<class NodeType>
  void dispatch_rhs(NodeType& node)
  {
    dispatch<symbolic_expression::BinaryNode, 
      &symbolic_expression::BinaryNode::get_rhs, 
      &symbolic_expression::BinaryNode::set_rhs>(node.get());
  }

  template<class NodeType>
  void dispatch_lhs(NodeType& node)
  {
    dispatch<symbolic_expression::BinaryNode, 
      &symbolic_expression::BinaryNode::get_lhs, 
      &symbolic_expression::BinaryNode::set_lhs>(node.get());
  }

  /**
   * 定義の簡約化(展開)をおこなう
   */
  symbolic_expression::node_sptr apply_definition(
    const referenced_definition_t& def_type,
    boost::shared_ptr<symbolic_expression::Caller> caller, 
    boost::shared_ptr<symbolic_expression::Definition> definition);
};

} //namespace parser
} //namespace ydla


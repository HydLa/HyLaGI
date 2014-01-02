#ifndef _INCLUDED_HYDLA_PARSE_TREE_SEMANTIC_ANALYZER_H_
#define _INCLUDED_HYDLA_PARSE_TREE_SEMANTIC_ANALYZER_H_

#include <stack>
#include <iostream>

#include "Node.h"
#include "TreeVisitor.h"
#include "DefinitionContainer.h"
#include "ParseTree.h"

namespace hydla { 
namespace parser {

class ParseTreeSemanticAnalyzer : 
  public hydla::parse_tree::TreeVisitor
{
public:
  typedef hydla::parse_tree::node_sptr                 node_sptr;
  typedef hydla::parse_tree::ParseTree::variable_map_t variable_map_t;

  ParseTreeSemanticAnalyzer(
    DefinitionContainer<hydla::parse_tree::ConstraintDefinition>& constraint_definition,
    DefinitionContainer<hydla::parse_tree::ProgramDefinition>&    program_definition,
    hydla::parse_tree::ParseTree* parse_tree);
  
  virtual ~ParseTreeSemanticAnalyzer();

  /**
   * 解析および制約呼び出しの展開をおこなう
   */
  void analyze(node_sptr& n/*, variable_map_t& variable_map*/);

  // 定義
  virtual void visit(boost::shared_ptr<hydla::parse_tree::ConstraintDefinition> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::ProgramDefinition> node);

  // 呼び出し
  virtual void visit(boost::shared_ptr<hydla::parse_tree::ConstraintCaller> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::ProgramCaller> node);

  // 制約式
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Constraint> node);

  // Ask制約
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Ask> node);

  // Tell制約
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Tell> node);

  // 比較演算子
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Equal> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::UnEqual> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Less> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::LessEqual> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Greater> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::GreaterEqual> node);

  // 論理演算子
  virtual void visit(boost::shared_ptr<hydla::parse_tree::LogicalAnd> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::LogicalOr> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Not> node);
  
  // 算術二項演算子
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Plus> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Subtract> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Times> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Divide> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Power> node);
  
  // 算術単項演算子
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Negative> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Positive> node);
  
  // 制約階層定義演算子
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Weaker> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Parallel> node);

  // 時相演算子
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Always> node);
  
  // 微分
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Differential> node);

  // 左極限
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Previous> node);
  
  // 関数
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Function> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::UnsupportedFunction> node);

  // 円周率
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Pi> node);
  // 自然対数の底
  virtual void visit(boost::shared_ptr<hydla::parse_tree::E> node);
  
  
  // 変数
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Variable> node);

  // 数字
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Number> node);

  // Print
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Print> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::PrintPP> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::PrintIP> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Scan> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Exit> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Abort> node);

  //Parameter
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Parameter> node){assert(0);}
  //Infinity
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Infinity> node){assert(0);}
  //SymbolicT
  virtual void visit(boost::shared_ptr<hydla::parse_tree::SymbolicT> node);

  //SystemVariable
  virtual void visit(boost::shared_ptr<hydla::parse_tree::SVtimer> node);
  
  //True
  virtual void visit(boost::shared_ptr<hydla::parse_tree::True> node);
 
  //False
  virtual void visit(boost::shared_ptr<hydla::parse_tree::False> node);
  

private:
  typedef hydla::parser::DefinitionContainer<
    hydla::parse_tree::Definition>::definition_map_key_t referenced_definition_t;

  typedef std::set<referenced_definition_t>         referenced_definition_list_t;
  
  typedef std::map<std::string, node_sptr>        formal_arg_map_t;

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
  
  /// プログラム中で使用される変数の一覧
  variable_map_t* variable_map_;
  
  /**
   * 新しい子ノード
   * accept後、これに値が入っている場合はノードの値を交換する
   */
  node_sptr new_child_;

  /**
   * 制約定義の情報
   */
  DefinitionContainer<hydla::parse_tree::ConstraintDefinition>& 
    constraint_definition_;
    
  /**
   * プログラム定義の情報
   */
  DefinitionContainer<hydla::parse_tree::ProgramDefinition>&    
    program_definition_;

  hydla::parse_tree::ParseTree* parse_tree_;
  
  /**
   * 指定したノードを呼び出し、
   * new_child_に値が設定されていた場合、子ノードを入れ替える
   */
  template<class C, 
           const node_sptr& (C::*getter)() const,
           void (C::*setter)(const node_sptr& child)>
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
    dispatch<hydla::parse_tree::UnaryNode, 
      &hydla::parse_tree::UnaryNode::get_child, 
      &hydla::parse_tree::UnaryNode::set_child>(node.get());
  }

  template<class NodeType>
  void dispatch_rhs(NodeType& node)
  {
    dispatch<hydla::parse_tree::BinaryNode, 
      &hydla::parse_tree::BinaryNode::get_rhs, 
      &hydla::parse_tree::BinaryNode::set_rhs>(node.get());
  }

  template<class NodeType>
  void dispatch_lhs(NodeType& node)
  {
    dispatch<hydla::parse_tree::BinaryNode, 
      &hydla::parse_tree::BinaryNode::get_lhs, 
      &hydla::parse_tree::BinaryNode::set_lhs>(node.get());
  }

  /**
   * 定義の簡約化(展開)をおこなう
   */
  node_sptr apply_definition(
    const referenced_definition_t& def_type,
    boost::shared_ptr<hydla::parse_tree::Caller> caller, 
    boost::shared_ptr<hydla::parse_tree::Definition> definition);
};

} //namespace parser
} //namespace hydla

#endif //_INCLUDED_HYDLA_PARSE_TREE_SEMANTIC_ANALYZER_H_

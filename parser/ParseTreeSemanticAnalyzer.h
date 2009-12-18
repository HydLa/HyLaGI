#ifndef _INCLUDED_HYDLA_PARSE_TREE_SEMANTIC_ANALYZER_H_
#define _INCLUDED_HYDLA_PARSE_TREE_SEMANTIC_ANALYZER_H_

#include <stack>
#include <iostream>
#include "Node.h"
#include "TreeVisitor.h"

#include "ParseTree.h"

namespace hydla { 
namespace parser {

class ParseTreeSemanticAnalyzer : public hydla::parse_tree::TreeVisitor
{
public:
  typedef hydla::parse_tree::node_sptr node_sptr;

  ParseTreeSemanticAnalyzer();
  virtual ~ParseTreeSemanticAnalyzer();

  /**
   * 解析および制約呼び出しの展開をおこなう
   */
  void analyze(hydla::parse_tree::ParseTree* pt);


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
  
  // 算術二項演算子
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Plus> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Subtract> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Times> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Divide> node);
  
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
  
  // 変数
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Variable> node);

  // 数字
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Number> node);

private:
  typedef hydla::parse_tree::ParseTree::difinition_type_t difinition_type_t;

  typedef std::set<difinition_type_t>             referenced_definition_list_t;

  typedef std::map<std::string, node_sptr>        formal_arg_map_t;

  typedef std::map<std::string, int>              variable_map_t;


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
  std::stack<State> state_stack_;
  
  /// プログラム中で使用される変数の表
  variable_map_t variable_map_;
  
  /**
   * 新しい子ノード
   * accept後、これに値が入っている場合はノードの値を交換する
   */
  node_sptr new_child_;

  /// 解析対象のParseTree
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
   * ノードのIDを更新する
   */
  template<typename T>
  void update_node_id(const T& n)
  {
    hydla::parse_tree::node_id_t id = n->get_id();
    if(id == 0) {
      parse_tree_->register_node(n);
    }
    else {
      parse_tree_->update_node(id, n);
    }
  }

  /**
   * 定義の簡約化(展開)をおこなう
   */
  node_sptr apply_definition(
    difinition_type_t* def_type,
    const boost::shared_ptr<hydla::parse_tree::Caller>& caller, 
    hydla::parse_tree::Definition* definition);
};

} //namespace parser
} //namespace hydla

#endif //_INCLUDED_HYDLA_PARSE_TREE_SEMANTIC_ANALYZER_H_

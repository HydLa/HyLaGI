#ifndef _INCLUDED_HYDLA_PREPROCESS_PARSE_TREE_H_
#define _INCLUDED_HYDLA_PREPROCESS_PARSE_TREE_H_

#include <stack>

#include "Node.h"
#include "TreeVisitor.h"

namespace hydla { 
namespace parse_tree {

class PreprocessParseTree : public TreeVisitor
{
public:
  PreprocessParseTree();
  virtual ~PreprocessParseTree();

  // 定義
  virtual void visit(boost::shared_ptr<ConstraintDefinition> node);
  virtual void visit(boost::shared_ptr<ProgramDefinition> node);

  // 呼び出し
  virtual void visit(boost::shared_ptr<ConstraintCaller> node);
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
  
  // 変数
  virtual void visit(boost::shared_ptr<Variable> node);

  // 数字
  virtual void visit(boost::shared_ptr<Number> node);

private:
  // 定義の型
  typedef std::string                             difinition_name_t;
  typedef int                                     bound_variable_count_t;
  typedef std::pair<difinition_name_t, 
                    bound_variable_count_t>       difinition_type_t;

  // 制約定義
  typedef boost::shared_ptr<ConstraintDefinition> constraint_def_map_value_t;
  typedef std::map<difinition_type_t,
                   constraint_def_map_value_t>    constraint_def_map_t;

  // プログラム定義
  typedef boost::shared_ptr<ProgramDefinition>    program_def_map_value_t;
  typedef std::map<difinition_type_t,
                   program_def_map_value_t>       program_def_map_t;

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

  /// 制約定義の表
  constraint_def_map_t constraint_def_map_;

  /// プログラム定義の表
  program_def_map_t program_def_map_;
  
  /// プログラム中で使用される変数の表
  variable_map_t variable_map_;
  
  /**
   * 新しい子ノード
   * accept後、これに値が入っている場合はノードの値を交換する
   */
  node_sptr new_child_;
  
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
    dispatch<UnaryNode, &UnaryNode::get_child, &UnaryNode::set_child>(node.get());
  }

  template<class NodeType>
  void dispatch_rhs(NodeType& node)
  {
    dispatch<BinaryNode, &BinaryNode::get_rhs, &BinaryNode::set_rhs>(node.get());
  }

  template<class NodeType>
  void dispatch_lhs(NodeType& node)
  {
    dispatch<BinaryNode, &BinaryNode::get_lhs, &BinaryNode::set_lhs>(node.get());
  }

};

} //namespace parse_tree
} //namespace hydla

#endif //_INCLUDED_HYDLA_PREPROCESS_PARSE_TREE_H_

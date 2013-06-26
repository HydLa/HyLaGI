#ifndef _INCLUDED_HYDLA_SIMULATOR_ASK_DISJUNCTION_FORMATTER_H_
#define _INCLUDED_HYDLA_SIMULATOR_ASK_DISJUNCTION_FORMATTER_H_

#include "Node.h"
#include "ParseTree.h"
#include "TreeVisitor.h"

namespace hydla {
namespace simulator {

class AskDisjunctionFormatter : 
  public hydla::parse_tree::TreeVisitor
{
public:
  typedef hydla::parse_tree::node_sptr node_sptr;
  typedef boost::shared_ptr<hydla::parse_tree::LogicalOr>  logical_or_sptr;
  typedef boost::shared_ptr<hydla::parse_tree::LogicalAnd> logical_and_sptr;

  AskDisjunctionFormatter();

  virtual ~AskDisjunctionFormatter();

  void format(hydla::parse_tree::ParseTree* pt);

  // 制約呼び出し
  virtual void visit(boost::shared_ptr<hydla::parse_tree::ConstraintCaller> node);

  // プログラム呼び出し
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
  
  // 制約階層定義演算子
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Weaker> node);

  virtual void visit(boost::shared_ptr<hydla::parse_tree::Parallel> node);

  // 時相演算子
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Always> node);
  // Print
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Print> node);

private:   
  template<class NodeType>
  void dispatch_unary_node(const NodeType& node)
  {       
    accept(node->get_child());
    if(new_child_) {
      node->set_child(new_child_);
      new_child_.reset();
    }
  }

  template<class NodeType>
  void dispatch_lhs(const NodeType& node)
  {
    accept(node->get_lhs());
    if(new_child_) {
      node->set_lhs(new_child_);
      new_child_.reset();
    }
  }

  template<class NodeType>
  void dispatch_rhs(const NodeType& node)
  {
    accept(node->get_rhs());
    if(new_child_) {
      node->set_rhs(new_child_);
      new_child_.reset();
    }
  }

  template<class NodeType>
  void dispatch_binary_node(const NodeType& node)
  {
    dispatch_lhs(node);
    dispatch_rhs(node);
  }


  /**
   * フォーマット対象となるParseTree
   */
  hydla::parse_tree::ParseTree* pt_;

  /**
   * 新しい子ノード
   * accept後、これに値が入っている場合はノードの値を交換する
   */
  node_sptr new_child_;

  /**
   * ORノードとANDノード間の交換がおこなわれたかどうか
   */
  bool swapped_;
};

} //namespace simulator
} //namespace hydla 

#endif // _INCLUDED_HYDLA_SIMULATOR_ASK_DISJUNCTION_FORMATTER_H_

#ifndef _INCLUDED_HYDLA_SIMULATOR_ASK_DISJUNCTION_FORMATTER_H_
#define _INCLUDED_HYDLA_SIMULATOR_ASK_DISJUNCTION_FORMATTER_H_

#include "Node.h"
#include "ParseTree.h"
#include "DefaultTreeVisitor.h"

namespace hydla {
namespace simulator {

class AskDisjunctionFormatter : 
  public hydla::symbolic_expression::DefaultTreeVisitor
{
public:
  typedef hydla::symbolic_expression::node_sptr node_sptr;
  typedef boost::shared_ptr<hydla::symbolic_expression::LogicalOr>  logical_or_sptr;
  typedef boost::shared_ptr<hydla::symbolic_expression::LogicalAnd> logical_and_sptr;

  AskDisjunctionFormatter();

  virtual ~AskDisjunctionFormatter();

  void format(hydla::parse_tree::ParseTree* pt);

  // 制約呼び出し
  virtual void visit(boost::shared_ptr<hydla::symbolic_expression::ConstraintCaller> node);

  // プログラム呼び出し
  virtual void visit(boost::shared_ptr<hydla::symbolic_expression::ProgramCaller> node);

  // 制約式
  virtual void visit(boost::shared_ptr<hydla::symbolic_expression::Constraint> node);

  // Ask制約
  virtual void visit(boost::shared_ptr<hydla::symbolic_expression::Ask> node);

  // Tell制約
  virtual void visit(boost::shared_ptr<hydla::symbolic_expression::Tell> node);

  // 比較演算子
  virtual void visit(boost::shared_ptr<hydla::symbolic_expression::Equal> node);

  virtual void visit(boost::shared_ptr<hydla::symbolic_expression::UnEqual> node);

  virtual void visit(boost::shared_ptr<hydla::symbolic_expression::Less> node);

  virtual void visit(boost::shared_ptr<hydla::symbolic_expression::LessEqual> node);

  virtual void visit(boost::shared_ptr<hydla::symbolic_expression::Greater> node);

  virtual void visit(boost::shared_ptr<hydla::symbolic_expression::GreaterEqual> node);

  // 論理演算子
  virtual void visit(boost::shared_ptr<hydla::symbolic_expression::LogicalAnd> node);

  virtual void visit(boost::shared_ptr<hydla::symbolic_expression::LogicalOr> node);
 
  virtual void visit(boost::shared_ptr<hydla::symbolic_expression::Not> node);
  
  // 制約階層定義演算子
  virtual void visit(boost::shared_ptr<hydla::symbolic_expression::Weaker> node);

  virtual void visit(boost::shared_ptr<hydla::symbolic_expression::Parallel> node);

  // 時相演算子
  virtual void visit(boost::shared_ptr<hydla::symbolic_expression::Always> node);
  // Print
  virtual void visit(boost::shared_ptr<hydla::symbolic_expression::Print> node);
  // True
  virtual void visit(boost::shared_ptr<hydla::symbolic_expression::True> node);

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
  symbolic_expression::node_sptr new_child_;

  /**
   * ORノードとANDノード間の交換がおこなわれたかどうか
   */
  bool swapped_;
};

} //namespace simulator
} //namespace hydla 

#endif // _INCLUDED_HYDLA_SIMULATOR_ASK_DISJUNCTION_FORMATTER_H_

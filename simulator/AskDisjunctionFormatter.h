#ifndef _INCLUDED_HYDLA_SIMULATOR_ASK_DISJUNCTION_FORMATTER_H_
#define _INCLUDED_HYDLA_SIMULATOR_ASK_DISJUNCTION_FORMATTER_H_

#include "Node.h"
#include "ParseTree.h"
#include "TreeVisitor.h"

namespace hydla {
namespace simulator {

template<typename NodeFactory>
class AskDisjunctionFormatter : 
  public hydla::parse_tree::TreeVisitor
{
public:
  typedef NodeFactory                  node_factory_t;
  typedef hydla::parse_tree::node_sptr node_sptr;
  typedef boost::shared_ptr<hydla::parse_tree::LogicalOr>  logical_or_sptr;
  typedef boost::shared_ptr<hydla::parse_tree::LogicalAnd> logical_and_sptr;

  AskDisjunctionFormatter()
  {}

  virtual ~AskDisjunctionFormatter()
  {}
  
  void format(const boost::shared_ptr<hydla::parse_tree::ParseTree>& pt)
  {
     pt->dispatch(this);
  }

  // 制約呼び出し
  virtual void visit(boost::shared_ptr<hydla::parse_tree::ConstraintCaller> node)
  {
    dispatch_unary_node(node);
  }

  // プログラム呼び出し
  virtual void visit(boost::shared_ptr<hydla::parse_tree::ProgramCaller> node)
  {    
    dispatch_unary_node(node);
  }

  // 制約式
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Constraint> node)
  {    
    dispatch_unary_node(node);
  }

  // Ask制約
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Ask> node)
  {
    accept(node->get_guard());
    if(new_child_) {
      node->set_guard(new_child_);
      new_child_.reset();
    }
  }

  // Tell制約
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Tell> node)
  {
    // do nothing
  }

  // 比較演算子
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Equal> node)
  {
    // do nothing
  }

  virtual void visit(boost::shared_ptr<hydla::parse_tree::UnEqual> node)
  {
    // do nothing
  }

  virtual void visit(boost::shared_ptr<hydla::parse_tree::Less> node)
  {
    // do nothing
  }

  virtual void visit(boost::shared_ptr<hydla::parse_tree::LessEqual> node)
  {
    // do nothing
  }

  virtual void visit(boost::shared_ptr<hydla::parse_tree::Greater> node)
  {
    // do nothing
  }

  virtual void visit(boost::shared_ptr<hydla::parse_tree::GreaterEqual> node)
    
  {
    // do nothing
  }

  // 論理演算子
  virtual void visit(boost::shared_ptr<hydla::parse_tree::LogicalAnd> node)
  {
    boost::shared_ptr<hydla::parse_tree::BinaryNode> n(node);

    // andの左子ノードがorであった場合
    logical_or_sptr lhs_child = 
      boost::dynamic_pointer_cast<hydla::parse_tree::LogicalOr>(n->get_lhs());
    if(lhs_child) {
      logical_or_sptr node_or(create_logical_or());

      logical_and_sptr lhs_and(create_logical_and());
      node_or->set_lhs(lhs_and);
      lhs_and->set_lhs(lhs_child->get_lhs());
      lhs_and->set_rhs(node->get_rhs());

      logical_and_sptr rhs_and(create_logical_and());
      node_or->set_rhs(rhs_and);
      rhs_and->set_lhs(lhs_child->get_rhs());
      rhs_and->set_rhs(n->get_rhs());

      n = node_or;
    }
    dispatch_lhs(n);        


    // andの右子ノードがorであった場合
    logical_or_sptr rhs_child = 
      boost::dynamic_pointer_cast<hydla::parse_tree::LogicalOr>(n->get_rhs());
    if(rhs_child) {
      logical_or_sptr node_or(create_logical_or());

      logical_and_sptr lhs_and(create_logical_and());
      node_or->set_lhs(lhs_and);
      lhs_and->set_lhs(rhs_child->get_lhs());
      lhs_and->set_rhs(node->get_lhs());

      logical_and_sptr rhs_and(create_logical_and());
      node_or->set_rhs(rhs_and);
      rhs_and->set_lhs(rhs_child->get_rhs());
      rhs_and->set_rhs(n->get_lhs());

      n = node_or;
    }
    dispatch_rhs(n);        

    new_child_ = n;
  }

  virtual void visit(boost::shared_ptr<hydla::parse_tree::LogicalOr> node)
  {
    dispatch_binary_node(node);
  }
  
  // 制約階層定義演算子
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Weaker> node)
  {
    dispatch_binary_node(node);
  }

  virtual void visit(boost::shared_ptr<hydla::parse_tree::Parallel> node)
  {
    dispatch_binary_node(node);
  }

  // 時相演算子
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Always> node)
  {
    dispatch_unary_node(node);
  }

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

  boost::shared_ptr<hydla::parse_tree::LogicalAnd>
    create_logical_and() const
  {
    return node_factory_t()(hydla::parse_tree::LogicalAnd());
  }

  boost::shared_ptr<hydla::parse_tree::LogicalOr>
    create_logical_or() const
  {
    return node_factory_t()(hydla::parse_tree::LogicalOr());
  }

  /**
   * 新しい子ノード
   * accept後、これに値が入っている場合はノードの値を交換する
   */
  node_sptr new_child_;
};

} //namespace simulator
} //namespace hydla 

#endif // _INCLUDED_HYDLA_SIMULATOR_ASK_DISJUNCTION_FORMATTER_H_
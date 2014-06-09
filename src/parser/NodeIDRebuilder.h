#pragma once

#include "Node.h"
#include "BaseNodeVisitor.h"
#include "ParseTree.h"

namespace hydla { 
namespace parser {

class NodeIDRebuilder : 
  public hydla::symbolic_expression::BaseNodeVisitor
{
public:
  typedef hydla::symbolic_expression::node_sptr node_sptr;

  NodeIDRebuilder()
  {}

  virtual ~NodeIDRebuilder()
  {}

  /**
   * ノードIDをすべて新たに登録しなおす
   * ParseTree側でノードID表を初期済みであると仮定している
   */
  void rebuild(hydla::parse_tree::ParseTree* pt)
  {
    parse_tree_ = pt;
    pt->dispatch(this);
  }

  /// 因子ノードの呼び出し
  virtual void visit(boost::shared_ptr<hydla::symbolic_expression::FactorNode> node)
  {
    rebuild_node(node);
  }
  
  /// 1つの子ノードを持つノードの呼び出し
  virtual void visit(boost::shared_ptr<hydla::symbolic_expression::UnaryNode> node)
  {
    rebuild_node(node);
    accept(node->get_child());
  }

  /// 2つの子ノードを持つノードの呼び出し
  virtual void visit(boost::shared_ptr<hydla::symbolic_expression::BinaryNode> node)
  {    
    rebuild_node(node);
    accept(node->get_lhs());
    accept(node->get_rhs());
  }

private:  
  template<typename T>
  void rebuild_node(const T& node)
  {
    node->set_id(0);
    parse_tree_->register_node(node);
  }

  hydla::parse_tree::ParseTree* parse_tree_;
};

} //namespace parser
} //namespace hydla


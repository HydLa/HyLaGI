#pragma once

#include <vector>

#include "Node.h"
#include "BaseNodeVisitor.h"
#include "ParseTree.h"

namespace hydla { 
namespace parser {

class NodeIDUpdater : 
  public hydla::symbolic_expression::BaseNodeVisitor
{
public:
  typedef hydla::symbolic_expression::node_sptr node_sptr;
  typedef std::set<hydla::symbolic_expression::node_id_t> node_id_list_t;

  NodeIDUpdater();

  virtual ~NodeIDUpdater();

  /**
   * ノードIDが付いていないノードに対して新たにIDを付与する
   */
  void update(hydla::parse_tree::ParseTree* pt);

  /// 因子ノードの呼び出し
  virtual void visit(boost::shared_ptr<hydla::symbolic_expression::FactorNode> node);
  
  /// 1つの子ノードを持つノードの呼び出し
  virtual void visit(boost::shared_ptr<hydla::symbolic_expression::UnaryNode> node);

  /// 2つの子ノードを持つノードの呼び出し
  virtual void visit(boost::shared_ptr<hydla::symbolic_expression::BinaryNode> node);
  
  /// 任意数の子ノードを持つノードの呼び出し
  virtual void visit(boost::shared_ptr<hydla::symbolic_expression::ArbitraryNode> node);

private:  

  /**
   * ノードのIDを更新する
   */
  template<typename T>
  void update_node_id(const T& n)
  {
    hydla::symbolic_expression::node_id_t id = n->get_id();
    if(id == 0) {
      parse_tree_->register_node(n);
      node_id_list_.insert(n->get_id());
    }
    else {
      parse_tree_->update_node(id, n);
      node_id_list_.insert(id);
    }
  }


  hydla::parse_tree::ParseTree* parse_tree_;

  node_id_list_t node_id_list_;
};

} //namespace parser
} //namespace hydla


#ifndef _INCLUDED_HYDLA_PARSER_NODE_ID_UPDATER_H_
#define _INCLUDED_HYDLA_PARSER_NODE_ID_UPDATER_H_

#include <vector>

#include "Node.h"
#include "BaseNodeVisitor.h"
#include "ParseTree.h"

namespace hydla { 
namespace parser {

class NodeIDUpdater : 
  public hydla::parse_tree::BaseNodeVisitor
{
public:
  typedef hydla::parse_tree::node_sptr node_sptr;
  typedef std::vector<hydla::parse_tree::node_id_t> node_id_list_t;

  NodeIDUpdater();

  virtual ~NodeIDUpdater();

  /**
   * ノードIDが付いていないノードに対して新たにIDを付与する
   */
  void update(hydla::parse_tree::ParseTree* pt);

  /// 因子ノードの呼び出し
  virtual void visit(boost::shared_ptr<hydla::parse_tree::FactorNode> node);
  
  /// 1つの子ノードを持つノードの呼び出し
  virtual void visit(boost::shared_ptr<hydla::parse_tree::UnaryNode> node);

  /// 2つの子ノードを持つノードの呼び出し
  virtual void visit(boost::shared_ptr<hydla::parse_tree::BinaryNode> node);

private:  

  /**
   * ノードのIDを更新する
   */
  template<typename T>
  void update_node_id(const T& n)
  {
    hydla::parse_tree::node_id_t id = n->get_id();
    if(id == 0) {
      parse_tree_->register_node(n);
      node_id_list_.push_back(n->get_id());
    }
    else {
      parse_tree_->update_node(id, n);
      node_id_list_.push_back(id);
    }
  }


  hydla::parse_tree::ParseTree* parse_tree_;

  node_id_list_t node_id_list_;
};

} //namespace parser
} //namespace hydla

#endif //_INCLUDED_HYDLA_PARSER_NODE_ID_UPDATER_H_

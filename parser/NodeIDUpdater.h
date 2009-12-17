#ifndef _INCLUDED_HYDLA_PARSER_NODE_ID_UPDATER_H_
#define _INCLUDED_HYDLA_PARSER_NODE_ID_UPDATER_H_

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

  NodeIDUpdater()
  {}

  virtual ~NodeIDUpdater()
  {}

  /**
   * 
   */
  void update(hydla::parse_tree::ParseTree* pt)
  {
    parse_tree_ = pt;
    pt->dispatch(this);
  }

  /// ���q�m�[�h�̌Ăяo��
  virtual void visit(boost::shared_ptr<hydla::parse_tree::FactorNode> node)
  {
    update_node_id(node);
  }
  
  /// 1�̎q�m�[�h�����m�[�h�̌Ăяo��
  virtual void visit(boost::shared_ptr<hydla::parse_tree::UnaryNode> node)
  {
    update_node_id(node);
    accept(node->get_child());
  }

  /// 2�̎q�m�[�h�����m�[�h�̌Ăяo��
  virtual void visit(boost::shared_ptr<hydla::parse_tree::BinaryNode> node)
  {    
    update_node_id(node);
    accept(node->get_lhs());
    accept(node->get_rhs());
  }

private:  
  hydla::parse_tree::ParseTree* parse_tree_;

  /**
   * �m�[�h��ID���X�V����
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
};

} //namespace parser
} //namespace hydla

#endif //_INCLUDED_HYDLA_PARSER_NODE_ID_UPDATER_H_

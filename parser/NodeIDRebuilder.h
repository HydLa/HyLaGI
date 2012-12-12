#ifndef _INCLUDED_HYDLA_PARSER_NODE_ID_REBUILDER_H_
#define _INCLUDED_HYDLA_PARSER_NODE_ID_REBUILDER_H_

#include "Node.h"
#include "BaseNodeVisitor.h"
#include "ParseTree.h"

namespace hydla { 
namespace parser {

class NodeIDRebuilder : 
  public hydla::parse_tree::BaseNodeVisitor
{
public:
  typedef hydla::parse_tree::node_sptr node_sptr;

  NodeIDRebuilder()
  {}

  virtual ~NodeIDRebuilder()
  {}

  /**
   * �m�[�hID�����ׂĐV���ɓo�^���Ȃ���
   * ParseTree���Ńm�[�hID�\�������ς݂ł���Ɖ��肵�Ă���
   */
  void rebuild(hydla::parse_tree::ParseTree* pt)
  {
    parse_tree_ = pt;
    pt->dispatch(this);
  }

  /// ���q�m�[�h�̌Ăяo��
  virtual void visit(boost::shared_ptr<hydla::parse_tree::FactorNode> node)
  {
    rebuild_node(node);
  }
  
  /// 1�̎q�m�[�h�����m�[�h�̌Ăяo��
  virtual void visit(boost::shared_ptr<hydla::parse_tree::UnaryNode> node)
  {
    rebuild_node(node);
    accept(node->get_child());
  }

  /// 2�̎q�m�[�h�����m�[�h�̌Ăяo��
  virtual void visit(boost::shared_ptr<hydla::parse_tree::BinaryNode> node)
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

#endif //_INCLUDED_HYDLA_PARSER_NODE_ID_REBUILDER_H_

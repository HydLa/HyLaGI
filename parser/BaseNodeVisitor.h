#ifndef _INCLUDED_HYDLA_PARSE_TREE_BASE_NODE_VISITOR_H_
#define _INCLUDED_HYDLA_PARSE_TREE_BASE_NODE_VISITOR_H_

#include <boost/shared_ptr.hpp>

#include "Node.h"

namespace hydla { 
namespace parse_tree {
  
/**
 * ParseTree�̃m�[�h�W���ɑ΂���Visitor�N���X
 */
class BaseNodeVisitor {
public:
  BaseNodeVisitor();

  virtual ~BaseNodeVisitor();

  /**
   * Node�N���X��accept�֐��Ăяo���p�w���p�֐�
   */
  template<class T>
  void accept(const T& n)
  {
    n->accept(n, this);
  }

  /// ���q�m�[�h�̌Ăяo��
  virtual void visit(boost::shared_ptr<FactorNode> node);
  
  /// 1�̎q�m�[�h�����m�[�h�̌Ăяo��
  virtual void visit(boost::shared_ptr<UnaryNode> node);

  /// 2�̎q�m�[�h�����m�[�h�̌Ăяo��
  virtual void visit(boost::shared_ptr<BinaryNode> node);
};

} //namespace parse_tree
} //namespace hydla

#endif //_INCLUDED_HYDLA_PARSE_TREE_BASE_NODE_VISITOR_H_

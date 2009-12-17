#ifndef _INCLUDED_HYDLA_SIMULATOR_INIT_NODE_REMOVER_H_
#define _INCLUDED_HYDLA_SIMULATOR_INIT_NODE_REMOVER_H_

#include <boost/shared_ptr.hpp>

#include "Node.h"
#include "ParseTree.h"
#include "TreeVisitor.h"

namespace hydla {
namespace simulator {

/**
 * tell�m�[�h���W�߂�r�W�^�[�N���X
 */
class InitNodeRemover : public parse_tree::TreeVisitor {
public:
  typedef hydla::parse_tree::node_sptr node_sptr;

  InitNodeRemover();
  virtual ~InitNodeRemover();

  /**
   *
   */
  void apply(hydla::parse_tree::ParseTree* pt);

  // ����
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Constraint> node);

  // Ask����
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Ask> node);

  // Tell����
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Tell> node);

  // �_����
  virtual void visit(boost::shared_ptr<hydla::parse_tree::LogicalAnd> node);
  
  // �������Z�q
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Always> node);

  // ���W���[���̎㍇��
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Weaker> node);

  // ���W���[���̕��񍇐�
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Parallel> node);
   
  // ����Ăяo��
  virtual void visit(boost::shared_ptr<hydla::parse_tree::ConstraintCaller> node);
  
  // �v���O�����Ăяo��
  virtual void visit(boost::shared_ptr<hydla::parse_tree::ProgramCaller> node);

private:
  node_sptr child_;

  void unary_node(boost::shared_ptr<hydla::parse_tree::UnaryNode> node);
  void binary_node(boost::shared_ptr<hydla::parse_tree::BinaryNode> node);
};

} //namespace simulator
} //namespace hydla 

#endif // _INCLUDED_HYDLA_SIMULATOR_INIT_NODE_REMOVER_H_

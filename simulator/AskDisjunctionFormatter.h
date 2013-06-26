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

  // ����Ăяo��
  virtual void visit(boost::shared_ptr<hydla::parse_tree::ConstraintCaller> node);

  // �v���O�����Ăяo��
  virtual void visit(boost::shared_ptr<hydla::parse_tree::ProgramCaller> node);

  // ����
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Constraint> node);

  // Ask����
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Ask> node);

  // Tell����
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Tell> node);

  // ��r���Z�q
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Equal> node);

  virtual void visit(boost::shared_ptr<hydla::parse_tree::UnEqual> node);

  virtual void visit(boost::shared_ptr<hydla::parse_tree::Less> node);

  virtual void visit(boost::shared_ptr<hydla::parse_tree::LessEqual> node);

  virtual void visit(boost::shared_ptr<hydla::parse_tree::Greater> node);

  virtual void visit(boost::shared_ptr<hydla::parse_tree::GreaterEqual> node);

  // �_�����Z�q
  virtual void visit(boost::shared_ptr<hydla::parse_tree::LogicalAnd> node);

  virtual void visit(boost::shared_ptr<hydla::parse_tree::LogicalOr> node);
 
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Not> node);
  
  // ����K�w��`���Z�q
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Weaker> node);

  virtual void visit(boost::shared_ptr<hydla::parse_tree::Parallel> node);

  // �������Z�q
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
   * �t�H�[�}�b�g�ΏۂƂȂ�ParseTree
   */
  hydla::parse_tree::ParseTree* pt_;

  /**
   * �V�����q�m�[�h
   * accept��A����ɒl�������Ă���ꍇ�̓m�[�h�̒l����������
   */
  node_sptr new_child_;

  /**
   * OR�m�[�h��AND�m�[�h�Ԃ̌����������Ȃ�ꂽ���ǂ���
   */
  bool swapped_;
};

} //namespace simulator
} //namespace hydla 

#endif // _INCLUDED_HYDLA_SIMULATOR_ASK_DISJUNCTION_FORMATTER_H_

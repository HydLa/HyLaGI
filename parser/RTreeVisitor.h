#ifndef _INCLUDED_HYDLA_VCS_REDUCE_TREE_VISITOR_H_
#define _INCLUDED_HYDLA_VCS_REDUCE_TREE_VISITOR_H_

#include <boost/shared_ptr.hpp>
#include <string>

#include "Node.h"
#include "TreeVisitor.h"
#include "ParseTree.h"

namespace hydla {
namespace parse_tree {

/**
 * ParseTree�̃m�[�h�W���ɑ΂���Visitor�N���X
 */
class RTreeVisitor :
  public hydla::parse_tree::TreeVisitor
{
public:
//�Ăяo���֐�
  std::string caller_;
  std::string expr_;
  //�R���X�g���N�^
  RTreeVisitor(int a);
  //�R���X�g���N�^ ��reduce_output�֐��p
  RTreeVisitor(std::string caller);
//  RTreeVisitor();

  virtual ~RTreeVisitor();

  /**
   * Node�N���X��accept�֐��Ăяo���p�w���p�֐�
   */
  template<class T>
  void accept(const T& n)
  {
    n->accept(n, this);
  }
  
  //c++������K
  int x;
  void sandbox();

  // ���񎮂�l���ǂ߂�string�ɂ��ĕԂ�
  virtual std::string get_expr(const node_sptr& node);
  //accept���O���ōs���ꍇ
  virtual std::string get_expr();
  // �K�[�h��l���ǂ߂�string�ɂ��ĕԂ�
  virtual std::string get_guard(const boost::shared_ptr<hydla::parse_tree::Ask>& ask);
  // ask�E�ӂ�string�ŕԂ�
  virtual std::string get_ask_rhs(const boost::shared_ptr<hydla::parse_tree::Ask>& ask);
//  virtual std::string get_expr(const node_sptr& node);

  // �����`
  virtual void visit(boost::shared_ptr<ConstraintDefinition> node);
  
  // �v���O������`
  virtual void visit(boost::shared_ptr<ProgramDefinition> node);

  // ����Ăяo��
  virtual void visit(boost::shared_ptr<ConstraintCaller> node);
  
  // �v���O�����Ăяo��
  virtual void visit(boost::shared_ptr<ProgramCaller> node);

  // ����
  virtual void visit(boost::shared_ptr<Constraint> node);

  // Ask����
  virtual void visit(boost::shared_ptr<Ask> node);

  // Tell����
  virtual void visit(boost::shared_ptr<Tell> node);

  // ��r���Z�q
  virtual void visit(boost::shared_ptr<Equal> node);
  virtual void visit(boost::shared_ptr<UnEqual> node);
  virtual void visit(boost::shared_ptr<Less> node);
  virtual void visit(boost::shared_ptr<LessEqual> node);
  virtual void visit(boost::shared_ptr<Greater> node);
  virtual void visit(boost::shared_ptr<GreaterEqual> node);

  // �_�����Z�q
  virtual void visit(boost::shared_ptr<LogicalAnd> node);
  virtual void visit(boost::shared_ptr<LogicalOr> node);
  
  // �Z�p�񍀉��Z�q
  virtual void visit(boost::shared_ptr<Plus> node);
  virtual void visit(boost::shared_ptr<Subtract> node);
  virtual void visit(boost::shared_ptr<Times> node);
  virtual void visit(boost::shared_ptr<Divide> node);
  
  // �Z�p�P�����Z�q
  virtual void visit(boost::shared_ptr<Negative> node);
  virtual void visit(boost::shared_ptr<Positive> node);
  
  // ����K�w��`���Z�q
  virtual void visit(boost::shared_ptr<Weaker> node);
  virtual void visit(boost::shared_ptr<Parallel> node);

  // �������Z�q
  virtual void visit(boost::shared_ptr<Always> node);
  
  // ����
  virtual void visit(boost::shared_ptr<Differential> node);

  // ���Ɍ�
  virtual void visit(boost::shared_ptr<Previous> node);
  
  // �ϐ�
  virtual void visit(boost::shared_ptr<Variable> node);

  // ����
  virtual void visit(boost::shared_ptr<Number> node);

};

} //namespace hydla
} //namespace parse_tree

#endif //_INCLUDED_HYDLA_VCS_REDUCE_TREE_VISITOR_H_

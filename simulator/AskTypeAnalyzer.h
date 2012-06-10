#ifndef _INCLUDED_HYDLA_SIMULATOR_ASK_TYPE_ANALYZER_H_
#define _INCLUDED_HYDLA_SIMULATOR_ASK_TYPE_ANALYZER_H_

#include "TypedAsk.h"
#include "ParseTree.h"
#include "TreeVisitor.h"

namespace hydla {
namespace simulator {
  
class AskTypeAnalyzer : 
  public hydla::parse_tree::TreeVisitor
{
public:
  typedef hydla::parse_tree::node_sptr node_sptr;

  AskTypeAnalyzer();

  virtual ~AskTypeAnalyzer();
 
  /**
   * Ask�������͂��C�^�t���������Ȃ�
   */
  void analyze(hydla::parse_tree::ParseTree* pt);

  // �Ăяo��
  virtual void visit(boost::shared_ptr<hydla::parse_tree::ConstraintCaller> node);
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
  
  // �Z�p�񍀉��Z�q
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Plus> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Subtract> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Times> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Divide> node);
  
  // �Z�p�P�����Z�q
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Negative> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Positive> node);
  
  // ����K�w��`���Z�q
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Weaker> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Parallel> node);

  // �������Z�q
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Always> node);
  
  // ����
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Differential> node);

  // ���Ɍ�
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Previous> node);
  
  // �ϐ�
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Variable> node);

  // ����
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Number> node);
  //Print
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Print> node);

private:  
  void dispatch(boost::shared_ptr<hydla::parse_tree::UnaryNode> node);
  void dispatch(boost::shared_ptr<hydla::parse_tree::BinaryNode> node);
    
  /**
   * �V�����q�m�[�h
   * accept��A����ɒl�������Ă���ꍇ�̓m�[�h�̒l����������
   */
  node_sptr new_child_;

  /**
   * �����������Ȃ����m�[�h�̒���prev���񂪑��݂������ǂ���
   */
  bool exist_prev_cons_;
};

} //namespace simulator
} //namespace hydla 

#endif // _INCLUDED_HYDLA_SIMULATOR_ASK_TYPE_ANALYZER_H_

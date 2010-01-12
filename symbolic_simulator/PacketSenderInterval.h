#ifndef _INCLUDED_HYDLA_PACKET_SENDER_INTERVAL_H_
#define _INCLUDED_HYDLA_PACKET_SENDER_INTERVAL_H_

#include "Node.h"
#include "TreeVisitor.h"
#include "mathlink_helper.h"
#include "ParseTree.h"
#include <map>
#include "ConstraintStoreBuilderInterval.h"


namespace hydla {
namespace symbolic_simulator {

class PacketSenderInterval : public parse_tree::TreeVisitor
{
public:

  PacketSenderInterval(MathLink& ml, bool debug_mode);

  virtual ~PacketSenderInterval();

  void put_vars();

  void put_cs(ConstraintStoreInterval constraint_store);

  void put_cs_vars(ConstraintStoreInterval constraint_store);

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

  // ����
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Differential> node);

  // ���Ɍ�
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Previous> node);
  
  // �ϐ�
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Variable> node);

  // ����
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Number> node);


private:
  MathLink& ml_;
  std::set<std::pair<std::string, int> > vars_;
  std::string vars_str_;
  /// Differential�m�[�h������ʂ�����
  int differential_count_;
  /// Prev�m�[�h�̉��ɂ��邩�ǂ���
  // �i�ʏ�ϐ��Ȃ�1�Aprev�ϐ�����-1�Ȃǂɂ��邩�H�j
  bool in_prev_;
  /// �f�o�b�O�o�͂����邩�ǂ���
  bool debug_mode_;

};

} //namespace symbolic_simulator
} // namespace hydla

#endif //_INCLUDED_HYDLA_PACKET_SENDER_INTERVAL_H_


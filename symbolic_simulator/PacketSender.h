#ifndef _INCLUDED_HYDLA_PACKET_SENDER_H_
#define _INCLUDED_HYDLA_PACKET_SENDER_H_

#include <map>

#include "Node.h"
#include "TreeVisitor.h"
#include "mathlink_helper.h"
#include "ParseTree.h"


namespace hydla {
namespace symbolic_simulator {

enum now_phase_t { NP_POINT_PHASE, NP_INTERVAL_PHASE };

class PacketSender : public hydla::parse_tree::TreeVisitor
{
public:
  typedef std::pair<std::string, int> var_info_t;
  typedef std::set<var_info_t>::const_iterator const_iterator;

  typedef hydla::parse_tree::node_sptr node_sptr;

  // Mathematica�ɑ���ۂɕϐ����ɂ���ړ��� "usrVar"
  static const std::string var_prefix;

  // var_info_t�ɑ΂���֗��֐�
  static const std::string get_var_name(const var_info_t vi)
  {
    return vi.first;
  }

  // var_info_t�ɑ΂���֗��֐�
  static const int get_var_differential_count(const var_info_t vi)
  {
    return std::abs(vi.second)-1;
  }

  // var_info_t�ɑ΂���֗��֐�
  static const bool is_var_prev(const var_info_t vi)
  {
    return (vi.second < 0);
  }

  PacketSender(MathLink& ml, now_phase_t phase=NP_POINT_PHASE);

  virtual ~PacketSender();

  const_iterator begin() const { return vars_.begin(); }

  const_iterator end() const { return vars_.end(); }

  void put_node(const node_sptr& node);

  void put_vars();

  //void put_cs(ConstraintStoreInterval constraint_store);

  //void put_cs_vars(ConstraintStoreInterval constraint_store);

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

  // pair<�ϐ���, (prev�ϐ��Ȃ�-1*)������+1>
  std::set<std::pair<std::string, int> > vars_;

  std::string vars_str_;

  // Differential�m�[�h������ʂ�����
  int differential_count_;

  /// Prev�m�[�h�̉��ɂ��邩�ǂ����i�ʏ�ϐ��Ȃ�1�Aprev�ϐ�����-1�Ȃǂɂ��邩�H�j
  bool in_prev_;

  now_phase_t phase_; // ���݂̃t�F�[�Y

  std::string debug_string_; // �f�o�b�O�o�͗p�ꎞ�ϐ�

};

} // namespace symbolic_simulator
} // namespace hydla



//#include "Node.h"
//#include "TreeVisitor.h"
//#include "mathlink_helper.h"
//#include "ParseTree.h"
//#include <map>
//#include "ConstraintStoreBuilderPoint.h"
//
//
//namespace hydla {
//namespace symbolic_simulator {
//
//class PacketSender : public parse_tree::TreeVisitor
//{
//public:
//
//  PacketSender(MathLink& ml, bool debug_mode);
//
//  virtual ~PacketSender();
//
//  void put_vars();
//
//  void put_cs(ConstraintStore constraint_store);
//
//  void put_cs_vars(ConstraintStore constraint_store);
//
//  // Ask����
//  virtual void visit(boost::shared_ptr<hydla::parse_tree::Ask> node);
//
//  // Tell����
//  virtual void visit(boost::shared_ptr<hydla::parse_tree::Tell> node);
//
//  // ��r���Z�q
//  virtual void visit(boost::shared_ptr<hydla::parse_tree::Equal> node);
//  virtual void visit(boost::shared_ptr<hydla::parse_tree::UnEqual> node);
//  virtual void visit(boost::shared_ptr<hydla::parse_tree::Less> node);
//  virtual void visit(boost::shared_ptr<hydla::parse_tree::LessEqual> node);
//  virtual void visit(boost::shared_ptr<hydla::parse_tree::Greater> node);
//  virtual void visit(boost::shared_ptr<hydla::parse_tree::GreaterEqual> node);
//
//  // �_�����Z�q
//  virtual void visit(boost::shared_ptr<hydla::parse_tree::LogicalAnd> node);
//  virtual void visit(boost::shared_ptr<hydla::parse_tree::LogicalOr> node);
//
//  // �Z�p�񍀉��Z�q
//  virtual void visit(boost::shared_ptr<hydla::parse_tree::Plus> node);
//  virtual void visit(boost::shared_ptr<hydla::parse_tree::Subtract> node);
//  virtual void visit(boost::shared_ptr<hydla::parse_tree::Times> node);
//  virtual void visit(boost::shared_ptr<hydla::parse_tree::Divide> node);
//
//  // �Z�p�P�����Z�q
//  virtual void visit(boost::shared_ptr<hydla::parse_tree::Negative> node);
//  virtual void visit(boost::shared_ptr<hydla::parse_tree::Positive> node);
//
//  // ����
//  virtual void visit(boost::shared_ptr<hydla::parse_tree::Differential> node);
//
//  // ���Ɍ�
//  virtual void visit(boost::shared_ptr<hydla::parse_tree::Previous> node);
//  
//  // �ϐ�
//  virtual void visit(boost::shared_ptr<hydla::parse_tree::Variable> node);
//
//  // ����
//  virtual void visit(boost::shared_ptr<hydla::parse_tree::Number> node);
//
//
//private:
//  MathLink& ml_;
//  std::set<std::pair<std::string, int> > vars_;
//  std::string vars_str_;
//  /// Differential�m�[�h������ʂ�����
//  int differential_count_;
//  /// Prev�m�[�h�̉��ɂ��邩�ǂ���
//  // �i�ʏ�ϐ��Ȃ�1�Aprev�ϐ�����-1�Ȃǂɂ��邩�H�j
//  bool in_prev_;
//  /// �f�o�b�O�o�͂����邩�ǂ���
//  bool debug_mode_;
//
//};
//
//} //namespace symbolic_simulator
//} // namespace hydla

#endif //_INCLUDED_HYDLA_PACKET_SENDER_H_


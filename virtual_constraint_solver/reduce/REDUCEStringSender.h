#ifndef _INCLUDED_HYDLA_VCS_REDUCE_REDUCE_STRING_SENDER_H_
#define _INCLUDED_HYDLA_VCS_REDUCE_REDUCE_STRING_SENDER_H_

#include <boost/shared_ptr.hpp>
#include <string>
#include <ostream>

#include "REDUCELink.h"

// TODO:�Ȃ�Ƃ�����
#include "../mathematica/PacketSender.h"
using namespace hydla::vcs::mathematica;


namespace hydla {
namespace vcs {
namespace reduce {

/**
 * ParseTree�̃m�[�h�W���ɑ΂���Visitor�N���X
 */
class REDUCEStringSender :
  public hydla::parse_tree::TreeVisitor
{
public:

  typedef std::map<std::string, int> max_diff_map_t;

  // TODO:���Ƃ�����
  /**
   * �ϐ��f�[�^
   * (�ϐ����C ������,  prev�ϐ����ǂ���)
   */
  typedef PacketSender::var_info_t          var_info_t;
  typedef PacketSender::var_info_list_t     var_info_list_t;
  typedef PacketSender::vars_const_iterator vars_const_iterator;
  typedef PacketSender::node_sptr           node_sptr;

  // REDUCE�ɑ���ۂɕϐ����ɂ���ړ��� "usrvar"
  static const std::string var_prefix;
  // REDUCE�ɑ���ۂɒ萔���ɂ���ړ���
  static const std::string par_prefix;
  // ��W����\��REDUCE���͗p������ "{}"
  static const std::string empty_list_string;

  REDUCEStringSender();
  REDUCEStringSender(REDUCELink& cl);

  virtual ~REDUCEStringSender();

  vars_const_iterator vars_begin() const { return vars_.begin(); }

  vars_const_iterator vars_end() const { return vars_.end(); }

  /**
   * �^����ꂽ�m�[�h�̑��M�������Ȃ�
   *
   * �m�[�h�̑��M�������Ȃ��ۂ͒���visit�֐����Ă΂��ɁC
   * �K�����̊֐����o�R���邱��
   */
  void put_node(const node_sptr& node, bool ignore_prev = false, bool entailed = true);

  /**
   * �ϐ��̑��M
   */
  void put_var(const var_info_t var, bool init_var = false);

  /**
   * put_node�̍ۂɑ��M���ꂽ�ϐ��Q�̑��M�������Ȃ�
   */
  void put_vars(bool ignore_prev = false);

  /**
   * put_node�̍ۂɑ��M���ꂽ�ϐ��Q�̃f�[�^���������C����������
   */
  void clear();

  /**
   * �ϐ��̍ő�����񐔂����Ƃ߂�
   */
  void create_max_diff_map(max_diff_map_t& max_diff_map);


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
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Power> node);

  // �Z�p�P�����Z�q
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Negative> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Positive> node);

  // ����
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Differential> node);

  // ���Ɍ�
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Previous> node);

  // �ے�
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Not> node);

  // �O�p�֐�
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Sin> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Cos> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Tan> node);
  // �t�O�p�֐�
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Asin> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Acos> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Atan> node);
  // �~����
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Pi> node);
  // �ΐ�
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Log> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Ln> node);
  // ���R�ΐ��̒�
  virtual void visit(boost::shared_ptr<hydla::parse_tree::E> node);
  // �C�ӂ̕�����
  virtual void visit(boost::shared_ptr<hydla::parse_tree::ArbitraryFactor> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::ArbitraryUnary> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::ArbitraryBinary> node);

  // �ϐ�
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Variable> node);

  // ����
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Number> node);

  // �L���萔
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Parameter> node);

  // t
  virtual void visit(boost::shared_ptr<hydla::parse_tree::SymbolicT> node);


private:
  REDUCELink* cl_;

protected:
  /// ���M���ꂽ�ϐ��̈ꗗ
  var_info_list_t vars_;

  // Differential�m�[�h������ʂ�����
  int differential_count_;

  /// Prev�m�[�h�̉��ɂ��邩�ǂ���
  int in_prev_;

  // prev����𖳎����邩�ǂ���
  bool ignore_prev_;

  // not��K�p���邩�ǂ���
  bool apply_not_;

};

} //namespace reduce
} //namespace vcs
} //namespace hydla

#endif //_INCLUDED_HYDLA_VCS_REDUCE_REDUCE_STRING_SENDER_H_

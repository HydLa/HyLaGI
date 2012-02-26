#ifndef _INCLUDED_HYDLA_VCS_MATHEMATICA_PACKET_SENDER_H_
#define _INCLUDED_HYDLA_VCS_MATHEMATICA_PACKET_SENDER_H_

#include <map>

#include <boost/tuple/tuple.hpp>
#include <boost/tuple/tuple_comparison.hpp>
#include <boost/tuple/tuple_io.hpp>

#include "DefaultTreeVisitor.h"
#include "mathlink_helper.h"
#include "ParseTree.h"

namespace hydla {
namespace vcs {
namespace mathematica {

class PacketSender : 
  public hydla::parse_tree::DefaultTreeVisitor
{
public:
  
  enum now_phase_t 
  { 
    NP_POINT_PHASE, 
    NP_INTERVAL_PHASE,
  };

  enum VariableArg {
    VA_Prev,
    VA_None,  
    VA_Time,  //������prev�������ɂȂ�
    VA_Zero,
  };

  typedef std::map<std::string, int> max_diff_map_t;
  /**
   * �ϐ��f�[�^
   * (�ϐ����C ������,  prev�ϐ����ǂ���)
   */
  typedef boost::tuple<std::string, int, bool> var_info_t;

  typedef std::set<var_info_t>                 var_info_list_t;
  typedef var_info_list_t::const_iterator      vars_const_iterator;
  typedef hydla::parse_tree::node_sptr         node_sptr;

  // Mathematica�ɑ���ۂɕϐ����ɂ���ړ��� "usrVar"
  static const std::string var_prefix;
  // Mathematica�ɑ���ۂɒ萔���ɂ���ړ���
  static const std::string par_prefix;

  PacketSender();
  PacketSender(MathLink& ml);

  virtual ~PacketSender();
  
  
  /// �ÓI�����o�̏��������s��
  static void initialize();

  vars_const_iterator vars_begin() const { return vars_.begin(); }

  vars_const_iterator vars_end() const { return vars_.end(); }

  /**
   * �^����ꂽ�m�[�h�̑��M�������Ȃ�
   *
   * �m�[�h�̑��M�������Ȃ��ۂ͒���visit�֐����Ă΂��ɁC
   * �K�����̊֐����o�R���邱��
   */

  void put_node(const node_sptr& node, VariableArg variable_arg);
  /**
   * ���X�g�`���ő��M����
   */
  void put_nodes(const std::vector<node_sptr> &constraints, VariableArg variable_arg);

  /**
   * �ϐ��̑��M
   */
  void put_var(const var_info_t var, VariableArg variable_arg);

  /**
   * put_node�̍ۂɑ��M���ꂽ�ϐ��Q�̑��M�������Ȃ�
   */
  void put_vars(VariableArg variable_arg);
  
  
  /**
   * ��2�̋L���萔��
   */
  void put_par(const std::string &name);
  void put_pars();



  /**
   * put_node�̍ۂɑ��M���ꂽ�ϐ��Q�̃f�[�^���������C����������
   */
  void clear();

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
  
  // �R�}���h��
  virtual void visit(boost::shared_ptr<hydla::parse_tree::PrintPP> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::PrintIP> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Scan> node);

  // �Z�p�P�����Z�q
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Negative> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Positive> node);

  // ����
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Differential> node);

  // ���Ɍ�
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Previous> node);

  // �ے�
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Not> node);
  
  // �֐�
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Function> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::UnsupportedFunction> node);
  
  
  // �~����
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Pi> node);
  // ���R�ΐ��̒�
  virtual void visit(boost::shared_ptr<hydla::parse_tree::E> node);
  
    
  // �ϐ�
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Variable> node);

  // ����
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Number> node);

  // �L���萔
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Parameter> node);

  // t
  virtual void visit(boost::shared_ptr<hydla::parse_tree::SymbolicT> node);

private:
  MathLink* ml_;
  static std::map<std::string, std::pair<std::string, int> > function_name_map_;

protected:
  /// ���M���ꂽ�ϐ��̈ꗗ
  var_info_list_t vars_;
  std::set<std::string> pars_;

  // Differential�m�[�h������ʂ�����
  int differential_count_;

  /// Prev�m�[�h�̉��ɂ��邩�ǂ���
  int in_prev_;

  /// �ϐ��̈����Ƃ��đ��镨
  VariableArg variable_arg_;

  // prev����𖳎����邩�ǂ���
  bool ignore_prev_;
};

} // namespace mathematica
} // namespace simulator
} // namespace hydla 

#endif // _INCLUDED_HYDLA_VCS_MATHEMATICA_PACKET_SENDER_H_

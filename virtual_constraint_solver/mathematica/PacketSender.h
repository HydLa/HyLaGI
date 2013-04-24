#ifndef _INCLUDED_HYDLA_VCS_MATHEMATICA_PACKET_SENDER_H_
#define _INCLUDED_HYDLA_VCS_MATHEMATICA_PACKET_SENDER_H_

#include <map>

#include <boost/tuple/tuple.hpp>
#include <boost/tuple/tuple_comparison.hpp>
#include <boost/tuple/tuple_io.hpp>

#include <boost/bimap/bimap.hpp>

#include "DefaultTreeVisitor.h"
#include "mathlink_helper.h"
#include "ParseTree.h"
#include "MathematicaVCS.h"

namespace hydla {
namespace vcs {
namespace mathematica {


enum now_phase_t 
{ 
  NP_POINT_PHASE, 
  NP_INTERVAL_PHASE,
};

enum VariableArg {
  VA_Prev,
  VA_None,  
  VA_Time,
  VA_Zero,
};

extern const std::string var_prefix;
extern const std::string par_prefix;

class PacketSender : 
  public hydla::parse_tree::DefaultTreeVisitor, hydla::simulator::ValueVisitor
{
public:
  
  typedef std::map<std::string, int> max_diff_map_t;
  /**
   * �ϐ��f�[�^
   * (�ϐ����C �����񐔁C���M�`��)
   */
  typedef boost::tuple<std::string, int, VariableArg>       var_info_t;
  typedef std::set<var_info_t>                 var_info_list_t;

  /**
   * �L���萔�f�[�^
   * (���̕ϐ����C �����񐔁Cid)
   */
  typedef boost::tuple<std::string, int, int>       par_info_t;
  typedef std::set<par_info_t>                 par_info_list_t;
  typedef par_info_list_t::const_iterator      pars_const_iterator;
  typedef var_info_list_t::const_iterator      vars_const_iterator;
  typedef hydla::parse_tree::node_sptr         node_sptr;

  // Mathematica�ɑ���ۂɕϐ����ɂ���ړ��� "usrVar"
  // Mathematica�ɑ���ۂɒ萔���ɂ���ړ���

  PacketSender(MathLink& ml);

  virtual ~PacketSender();
  
  
  /// �ÓI�����o�̏��������s��
  static void initialize();

  vars_const_iterator vars_begin() const { return vars_.begin(); }

  vars_const_iterator vars_end() const { return vars_.end(); }


  void put_value(value_t, VariableArg var);

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
  void put_var(const var_info_t var);
  void put_var(const std::string& variable_name, const int& diff_count, VariableArg variable_arg);

  /**
   * put_node�̍ۂɑ��M���ꂽ�ϐ��Q�̑��M�������Ȃ�
   * �����l�����M���ꂽ�ꍇ�́C���̔����������M������̂Ƃ���D
   */
  void put_vars();
  
  
  /**
   * ��2�̋L���萔��
   */
  void put_par(const par_info_t par);
  void put_par(const std::string& name, const int& diff_count, const int& id);
  void put_pars();



  /**
   * put_node�̍ۂɑ��M���ꂽ�ϐ��Q�̃f�[�^���������C����������
   */
  void clear();

  void create_max_diff_map(max_diff_map_t& max_diff_map);

  virtual void visit(hydla::simulator::symbolic::SymbolicValue& value);

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
  
  // ������
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Infinity> node);
  
  typedef std::pair<std::string, int> function_t;
  typedef boost::bimaps::bimap<function_t, function_t > function_map_t;
  static function_map_t function_map_;
private:
  MathLink* ml_;
  /// ���M���ꂽ�ϐ��̈ꗗ
  var_info_list_t vars_;
  par_info_list_t pars_;

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

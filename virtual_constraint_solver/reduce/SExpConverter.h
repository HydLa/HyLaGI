#ifndef _INCLUDED_HYDLA_VCS_REDUCE_S_EXP_CONVERTER_H_
#define _INCLUDED_HYDLA_VCS_REDUCE_S_EXP_CONVERTER_H_


//S����SymbolicValue�Ƃ����C���̕ϊ���S������N���X�D


#include "../SymbolicVirtualConstraintSolver.h"
#include "../../parser/SExpParser.h"

using namespace hydla::parser;

namespace hydla {
namespace vcs {
namespace reduce {


class SExpConverter:
  public hydla::parse_tree::TreeVisitor
{
  public:
  typedef hydla::vcs::SymbolicVirtualConstraintSolver::value_t       value_t;
  typedef hydla::vcs::SymbolicVirtualConstraintSolver::value_range_t value_range_t;
  typedef hydla::parser::SExpParser::const_tree_iter_t               const_tree_iter_t;
  typedef hydla::parse_tree::node_sptr                               node_sptr;

  typedef enum{
    NODE_PLUS,
    NODE_SUBTRACT,
    NODE_TIMES,
    NODE_DIVIDE,
    NODE_POWER,
    NODE_DIFFERENTIAL,
    NODE_PREVIOUS,
    NODE_SQRT,
    NODE_NEGATIVE
  }nodeType;

  typedef node_sptr (function_for_node)(SExpParser &sp, const_tree_iter_t iter, const nodeType &);
  typedef function_for_node *p_function_for_node;
  typedef struct tag_function_and_node{
    p_function_for_node function;
    nodeType node;
    tag_function_and_node(p_function_for_node func, nodeType nod):function(func), node(nod){}
  }function_and_node;
  //Mathematica������Ə���&�m�[�h�̑Ή��֌W
  typedef std::map<std::string, function_and_node> string_map_t;
  static string_map_t string_map_;

  SExpConverter();

  virtual ~SExpConverter();

  //������
  static void initialize();

  //�e�m�[�h�ɑΉ����鏈���D�i���F�֐��j
  static function_for_node for_derivative;
  static function_for_node for_unary_node;
  static function_for_node for_binary_node;

  // S���Ƃ���value�ɕϊ�����
  static value_t convert_s_exp_to_symbolic_value(SExpParser &sp, const_tree_iter_t iter);

/*
  //�֌W���Z�q�̕�����\����Ԃ�
  static std::string get_relation_math_string(value_range_t::Relation rel);
  
  //�����ɑΉ��t����ꂽ�֌W��Ԃ�
  static value_range_t::Relation get_relation_from_code(const int &relop_code);
  
  //�l���L���萔��p�����\���ɂ���
  static void set_parameter_on_value(value_t &val, const std::string &par_name);
*/  
  //value�Ƃ���REDUCE�p�̕�����ɕϊ�����D(t)�Ƃ�(0)�Ƃ����Ȃ��̂ŁCPP��p�Ƃ��Ă���
  std::string convert_symbolic_value_to_string(const value_t&);


  private:
  //�ċA�ŌĂяo���Ă�����
  static node_sptr convert_s_exp_to_symbolic_tree(SExpParser &sp, const_tree_iter_t iter);

  //�ϊ����Ɏg��������ϐ�
  std::string string_for_reduce_;
  //�ϊ����Ɏg��������
  int differential_count_;
  //�ϊ����Ɏg�����Ɍ�����
  int in_prev_;



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
};

} // namespace reduce
} // namespace vcs
} // namespace hydla 

#endif //_INCLUDED_HYDLA_VCS_REDUCE_S_EXP_CONVERTER_H_

#ifndef _INCLUDED_HYDLA_VCS_MATHEMATICA_EXPRESSION_CONVERTER_H_
#define _INCLUDED_HYDLA_VCS_MATHEMATICA_EXPRESSION_CONVERTER_H_


//Mathematica�������SymbolicValue�Ƃ����C���̕ϊ���S������N���X�DSymbolicValueRange�������D


#include "../SymbolicVirtualConstraintSolver.h"
#include "PacketSender.h"
#include <map>
#include "TreeVisitor.h"
#include "ParseTree.h"

namespace hydla {
namespace vcs {
namespace mathematica {


class MathematicaExpressionConverter
{
  private:
  typedef hydla::vcs::SymbolicVirtualConstraintSolver::value_t value_t;
  typedef hydla::vcs::SymbolicVirtualConstraintSolver::value_range_t value_range_t;
  public:
  typedef enum{
    NODE_PLUS,
    NODE_SUBTRACT,
    NODE_TIMES,
    NODE_DIVIDE,
    NODE_POWER,
    NODE_DIFFERENTIAL,
    NODE_PREVIOUS,
    NODE_SQRT
  }nodeType;
  
  MathematicaExpressionConverter(){}
  virtual ~MathematicaExpressionConverter(){}

  typedef hydla::parse_tree::node_sptr node_sptr;
  typedef node_sptr (function_for_node)(const std::string &expr, std::string::size_type &now, const nodeType &);
  typedef function_for_node *p_function_for_node;
  typedef struct tag_function_and_node{
    p_function_for_node function;
    nodeType node;
    tag_function_and_node(p_function_for_node func, nodeType nod):function(func), node(nod){}
  }function_and_node;
  //Mathematica������Ə���&�m�[�h�̑Ή��֌W
  typedef std::map<std::string, function_and_node> string_map_t;
  static string_map_t string_map_;

  //������
  static void initialize();

  //�e�m�[�h�ɑΉ����鏈���D�i���F�֐��j
  static function_for_node for_derivative;
  static function_for_node for_unary_node;
  static function_for_node for_binary_node;

  //������Ƃ���value�ɕϊ�����
  static value_t convert_math_string_to_symbolic_value(const std::string &expr);

  //�֌W���Z�q�̕�����\����Ԃ�
  static std::string get_relation_math_string(value_range_t::Relation rel);
  
  //�����ɑΉ��t����ꂽ�֌W��Ԃ�
  static value_range_t::Relation get_relation_from_code(const int &relop_code);
  
  //�l���L���萔��p�����\���ɂ���
  static void set_parameter_on_value(value_t &val, const std::string &par_name);
  
  private:
  //�ċA�ŌĂяo���Ă�����
  static node_sptr convert_math_string_to_symbolic_tree(const std::string &expr, std::string::size_type &now);
  
};

} // namespace mathematica
} // namespace vcs
} // namespace hydla 

#endif //_INCLUDED_HYDLA_VCS_MATHEMATICA_EXPRESSION_CONVERTER_H_

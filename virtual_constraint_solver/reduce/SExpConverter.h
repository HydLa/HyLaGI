#ifndef _INCLUDED_HYDLA_VCS_REDUCE_S_EXP_CONVERTER_H_
#define _INCLUDED_HYDLA_VCS_REDUCE_S_EXP_CONVERTER_H_


//S����SymbolicValue�Ƃ����C���̕ϊ���S������N���X�D


#include "../SymbolicVirtualConstraintSolver.h"
#include "Node.h"
#include "../../parser/SExpParser.h"

namespace hydla {
namespace vcs {
namespace reduce {


class SExpConverter{
  public:
  typedef hydla::vcs::SymbolicVirtualConstraintSolver::value_t       value_t;
  typedef hydla::vcs::SymbolicVirtualConstraintSolver::value_range_t value_range_t;
  typedef hydla::parser::SExpParser::const_tree_iter_t               const_tree_iter_t;
  typedef hydla::parse_tree::node_sptr                               node_sptr;

  // S���Ƃ���value�ɕϊ�����
  value_t convert_s_exp_to_symbolic_value(const_tree_iter_t iter);

/*
  //�֌W���Z�q�̕�����\����Ԃ�
  static std::string get_relation_math_string(value_range_t::Relation rel);
  
  //�����ɑΉ��t����ꂽ�֌W��Ԃ�
  static value_range_t::Relation get_relation_from_code(const int &relop_code);
  
  //�l���L���萔��p�����\���ɂ���
  static void set_parameter_on_value(value_t &val, const std::string &par_name);
  
  //value�Ƃ���mathematica�p�̕�����ɕϊ�����D(t)�Ƃ�(0)�Ƃ����Ȃ��̂ŁCPP��p�Ƃ��Ă���
  std::string convert_symbolic_value_to_math_string(const value_t&);


  private:
  //�ċA�ŌĂяo���Ă�����
  static node_sptr convert_s_exp_to_symbolic_tree(const std::string &expr, std::string::size_type &now);
*/

};

} // namespace reduce
} // namespace vcs
} // namespace hydla 

#endif //_INCLUDED_HYDLA_VCS_REDUCE_S_EXP_CONVERTER_H_

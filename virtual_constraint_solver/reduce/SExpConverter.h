#ifndef _INCLUDED_HYDLA_VCS_REDUCE_S_EXP_CONVERTER_H_
#define _INCLUDED_HYDLA_VCS_REDUCE_S_EXP_CONVERTER_H_


//S式⇔SymbolicValueという，式の変換を担当するクラス．


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

  // S式とってvalueに変換する
  value_t convert_s_exp_to_symbolic_value(const_tree_iter_t iter);

/*
  //関係演算子の文字列表現を返す
  static std::string get_relation_math_string(value_range_t::Relation rel);
  
  //数字に対応付けられた関係を返す
  static value_range_t::Relation get_relation_from_code(const int &relop_code);
  
  //値を記号定数を用いた表現にする
  static void set_parameter_on_value(value_t &val, const std::string &par_name);
  
  //valueとってmathematica用の文字列に変換する．(t)とか(0)とかつけないので，PP専用としておく
  std::string convert_symbolic_value_to_math_string(const value_t&);


  private:
  //再帰で呼び出していく方
  static node_sptr convert_s_exp_to_symbolic_tree(const std::string &expr, std::string::size_type &now);
*/

};

} // namespace reduce
} // namespace vcs
} // namespace hydla 

#endif //_INCLUDED_HYDLA_VCS_REDUCE_S_EXP_CONVERTER_H_

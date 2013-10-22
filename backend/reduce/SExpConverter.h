#ifndef _INCLUDED_HYDLA_VCS_REDUCE_S_EXP_CONVERTER_H_
#define _INCLUDED_HYDLA_VCS_REDUCE_S_EXP_CONVERTER_H_

#include "../../simulator/DefaultParameter.h"
#include "../SymbolicVirtualConstraintSolver.h"
#include "sexp/SExpParseTree.h"

using namespace hydla::parser;

namespace hydla {
namespace vcs {
namespace reduce {


/**
 * S式⇔SymbolicValueという，式の変換を担当するクラス．
 * ドメイン駆動設計におけるサービス
 * TODO: 適切な名前は？
 */
class SExpConverter
{
  public:
  typedef hydla::vcs::SymbolicVirtualConstraintSolver::value_t       value_t;
  typedef hydla::vcs::SymbolicVirtualConstraintSolver::value_range_t value_range_t;
  typedef hydla::vcs::SymbolicVirtualConstraintSolver::variable_t    variable_t;
  typedef hydla::vcs::SymbolicVirtualConstraintSolver::parameter_t   parameter_t;
  typedef hydla::parser::SExpParseTree::const_tree_iter_t               const_tree_iter_t;
  typedef hydla::parse_tree::node_sptr                               node_sptr;

  SExpConverter();
  virtual ~SExpConverter();

  /** （vairable）=（node）の形のノードを返す */
  static node_sptr make_equal(const variable_t &variable, const node_sptr& node, const bool& prev, const bool& init_var = false);

  /** （vairable）=（node）の形のノードを返す */
  static node_sptr make_equal(hydla::simulator::DefaultParameter &variable, const node_sptr& node, const bool& prev, const bool& init_var = false);
  
  /** 値を記号定数を用いた表現にする */
  static void set_parameter_on_value(value_t &val, const parameter_t &par);
  
  /** valと関係演算子を元に、rangeを設定する */
  static void set_range(const value_t &val, value_range_t &range, const int& relop);

};

} // namespace reduce
} // namespace vcs
} // namespace hydla 

#endif //_INCLUDED_HYDLA_VCS_REDUCE_S_EXP_CONVERTER_H_

#ifndef _INCLUDED_HYDLA_VCS_MATHEMATICA_EXPRESSION_CONVERTER_H_
#define _INCLUDED_HYDLA_VCS_MATHEMATICA_EXPRESSION_CONVERTER_H_


//Mathematica文字列⇔SymbolicValueという，式の変換を担当するクラス．SymbolicValueRangeも少し．


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
  typedef hydla::vcs::SymbolicVirtualConstraintSolver::variable_t variable_t;
  typedef hydla::vcs::SymbolicVirtualConstraintSolver::parameter_t parameter_t;
  
  public:
  
  MathematicaExpressionConverter(){}
  virtual ~MathematicaExpressionConverter(){}

  typedef hydla::parse_tree::node_sptr node_sptr;

  //初期化
  static void initialize();

  /**
   * 受信してvalueに変換する
   */
  static node_sptr receive_and_make_symbolic_value(MathLink &ml);

  //valと関係演算子を元に、rangeを設定する
  static void set_range(const value_t &val, value_range_t &range, const int& relop);
};

} // namespace mathematica
} // namespace vcs
} // namespace hydla 

#endif //_INCLUDED_HYDLA_VCS_MATHEMATICA_EXPRESSION_CONVERTER_H_

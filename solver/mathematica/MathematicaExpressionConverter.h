#ifndef _INCLUDED_HYDLA_SOLVER_MATHEMATICA_MATHEMATICA_EXPRESSION_CONVERTER_H_
#define _INCLUDED_HYDLA_SOLVER_MATHEMATICA_MATHEMATICA_EXPRESSION_CONVERTER_H_


//Mathematica文字列⇔SymbolicValueという，式の変換を担当するクラス．ValueRangeも少し．

#include "../SymbolicSolver.h"
#include "PacketSender.h"
#include <map>
#include "TreeVisitor.h"
#include "ParseTree.h"

namespace hydla {
namespace solver {
namespace mathematica {


class MathematicaExpressionConverter
{
  private:
  typedef hydla::solver::SymbolicSolver::value_t value_t;
  typedef hydla::solver::SymbolicSolver::value_range_t value_range_t;
  typedef hydla::solver::SymbolicSolver::variable_t variable_t;
  typedef hydla::solver::SymbolicSolver::parameter_t parameter_t;
  MathematicaExpressionConverter(){}
  
  public:
  
  virtual ~MathematicaExpressionConverter(){}

  typedef hydla::parse_tree::node_sptr node_sptr;

  //初期化
  static void initialize();

  /**
   * 受信してvalueに変換する
   */
  static value_t receive_and_make_symbolic_value(MathLink &ml);
  static node_sptr make_tree(MathLink &ml);

  //valと関係演算子を元に、rangeを設定する
  static void set_range(const value_t &val, value_range_t &range, const int& relop);
};

} // namespace mathematica
} // namespace solver
} // namespace hydla 

#endif //include guard

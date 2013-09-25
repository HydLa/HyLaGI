#ifndef _INCLUDED_HYDLA_VCS_REDUCE_S_EXP_EXPRESSION_CONVERTER_H_
#define _INCLUDED_HYDLA_VCS_REDUCE_S_EXP_EXPRESSION_CONVERTER_H_

#include "../../../simulator/DefaultParameter.h"
#include "../../SymbolicVirtualConstraintSolver.h"
#include "SExpAST.h"

using namespace hydla::parser;

namespace hydla {
namespace vcs {
namespace reduce {


/**
 * S式⇔SymbolicValueという，式の変換を担当するクラス．
 * ドメイン駆動設計におけるサービス
 */
class SExpExpressionConverter
{
public:
  typedef hydla::vcs::SymbolicVirtualConstraintSolver::value_t       value_t;
  typedef hydla::vcs::SymbolicVirtualConstraintSolver::value_range_t value_range_t;
  typedef hydla::vcs::SymbolicVirtualConstraintSolver::variable_t    variable_t;
  typedef hydla::vcs::SymbolicVirtualConstraintSolver::parameter_t   parameter_t;
  typedef hydla::parser::SExpAST::const_tree_iter_t                  const_tree_iter_t;
  typedef hydla::parse_tree::node_sptr                               node_sptr;

  SExpExpressionConverter();
  virtual ~SExpExpressionConverter();

  /** （vairable）=（node）の形のノードを返す */
  static node_sptr make_equal(const variable_t &variable, const node_sptr& node, const bool& prev, const bool& init_var = false);

  /** （vairable）=（node）の形のノードを返す */
  static node_sptr make_equal(hydla::simulator::DefaultParameter &variable, const node_sptr& node, const bool& prev, const bool& init_var = false);
  
  /** 値を記号定数を用いた表現にする */
  static void set_parameter_on_value(value_t &val, const parameter_t &par);
  
  /** valと関係演算子を元に、rangeを設定する */
  static void set_range(const value_t &val, value_range_t &range, const int& relop);

  /**
   * string_map_の初期化
   * ノードと文字列の対応関係を作っておく
   */
  static void initialize();

  static int get_derivative_count(const_tree_iter_t iter);

  /** 
   * S式とってstringに変換する
   * @param isFirst 再帰の冒頭か、ユーザはデフォルト引数を使用する
   */
  static std::string to_string(const_tree_iter_t iter, bool isFirst = true);

  /** S式とってvalue_tに変換する */
  static value_t to_value(const_tree_iter_t iter);

  /** リスト操作を模した関数 */
  static const_tree_iter_t car(const_tree_iter_t iter); 
  static const_tree_iter_t cadr(const_tree_iter_t iter); 
  static const_tree_iter_t caddr(const_tree_iter_t iter); 
  static const_tree_iter_t cadddr(const_tree_iter_t iter); 

private:

  typedef enum{
    NODE_PLUS,
    NODE_SUBTRACT,
    NODE_TIMES,
    NODE_DIVIDE,
    NODE_POWER,
    NODE_DIFFERENTIAL,
    NODE_PREVIOUS,
    NODE_SQRT,
    NODE_NEGATIVE,
  }nodeType;

  typedef node_sptr (function_for_node)(const_tree_iter_t iter, const nodeType &);
  /** 関数ポインタの型 */
  typedef function_for_node *p_function_for_node;

  typedef struct tag_function_and_node{
    p_function_for_node function;
    nodeType node;
    tag_function_and_node(p_function_for_node func, nodeType nod):function(func), node(nod){}
  }function_and_node;
  typedef std::map<std::string, function_and_node> string_map_t;

  /** 各ノードに対応する処理．（注：関数）*/
  static function_for_node for_derivative;
  static function_for_node for_unary_node;
  static function_for_node for_binary_node;

  /** 再帰で呼び出していく方 */
  static node_sptr to_symbolic_tree(const_tree_iter_t iter);

  /** Mathematica文字列と処理&ノードの対応関係 */
  static string_map_t string_map_;
};

} // namespace reduce
} // namespace vcs
} // namespace hydla 

#endif //_INCLUDED_HYDLA_VCS_REDUCE_S_EXP_EXPRESSION_CONVERTER_H_

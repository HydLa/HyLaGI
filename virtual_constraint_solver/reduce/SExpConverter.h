#ifndef _INCLUDED_HYDLA_VCS_REDUCE_S_EXP_CONVERTER_H_
#define _INCLUDED_HYDLA_VCS_REDUCE_S_EXP_CONVERTER_H_


//S式⇔SymbolicValueという，式の変換を担当するクラス．


#include "../SymbolicVirtualConstraintSolver.h"
#include "../../simulator/DefaultParameter.h"
#include "../../parser/SExpParser.h"

using namespace hydla::parser;

namespace hydla {
namespace vcs {
namespace reduce {


class SExpConverter
{
  public:
  typedef hydla::vcs::SymbolicVirtualConstraintSolver::value_t       value_t;
  typedef hydla::vcs::SymbolicVirtualConstraintSolver::value_range_t value_range_t;
  typedef hydla::vcs::SymbolicVirtualConstraintSolver::variable_t    variable_t;
  typedef hydla::vcs::SymbolicVirtualConstraintSolver::parameter_t   parameter_t;
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
    NODE_NEGATIVE,
  }nodeType;

  typedef node_sptr (function_for_node)(SExpParser &sp, const_tree_iter_t iter, const nodeType &);
  typedef function_for_node *p_function_for_node;
  typedef struct tag_function_and_node{
    p_function_for_node function;
    nodeType node;
    tag_function_and_node(p_function_for_node func, nodeType nod):function(func), node(nod){}
  }function_and_node;
  //Mathematica文字列と処理&ノードの対応関係
  typedef std::map<std::string, function_and_node> string_map_t;
  static string_map_t string_map_;

  SExpConverter();

  virtual ~SExpConverter();

  //初期化
  static void initialize();

  //各ノードに対応する処理．（注：関数）
  static function_for_node for_derivative;
  static function_for_node for_unary_node;
  static function_for_node for_binary_node;

  // S式とってvalueに変換する
  static value_t convert_s_exp_to_symbolic_value(SExpParser &sp, const_tree_iter_t iter);

  static void add_parameter(variable_t &variable, parameter_t &parameter);
  static void clear_parameter_map();

  /**
   * （vairable）=（node）の形のノードを返す
   */
  static node_sptr make_equal(const variable_t &variable, const node_sptr& node, const bool& prev, const bool& init_var = false);

   /**
   * （vairable）=（node）の形のノードを返す
   */
  static node_sptr make_equal(hydla::simulator::DefaultParameter &variable, const node_sptr& node, const bool& prev, const bool& init_var = false);
  
  //値を記号定数を用いた表現にする
  static void set_parameter_on_value(value_t &val, const parameter_t &par);
  
  //valと関係演算子を元に、rangeを設定する
  static void set_range(const value_t &val, value_range_t &range, const int& relop);


  private:
  //再帰で呼び出していく方
  static node_sptr convert_s_exp_to_symbolic_tree(SExpParser &sp, const_tree_iter_t iter);

  static std::map<variable_t, parameter_t> variable_parameter_map_;
};

} // namespace reduce
} // namespace vcs
} // namespace hydla 

#endif //_INCLUDED_HYDLA_VCS_REDUCE_S_EXP_CONVERTER_H_

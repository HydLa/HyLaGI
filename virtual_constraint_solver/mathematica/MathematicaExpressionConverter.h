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
  //Mathematica文字列と処理&ノードの対応関係
  typedef std::map<std::string, function_and_node> string_map_t;
  static string_map_t string_map_;

  //初期化
  static void initialize();


  //文字列とってvalueに変換する
  static value_t convert_math_string_to_symbolic_value(const std::string &expr);
  
  static void add_parameter_name(std::string variable_name, std::string parameter_name);
  static void clear_parameter_name();

  /**
   * （vairable）=（node）の形のノードを返す
   */
  static node_sptr make_equal(const variable_t &variable, const node_sptr& node, const bool& prev);

  
  //値を記号定数を用いた表現にする
  static void set_parameter_on_value(value_t &val, const std::string &par_name);
  
  
  //valと関係演算子を元に、rangeを設定する
  static void set_range(const value_t &val, value_range_t &range, const int& relop);
  
  
  private:
  
  //各ノードに対応する処理．（注：関数）
  static function_for_node for_derivative;
  static function_for_node for_unary_node;
  static function_for_node for_binary_node;
  
  //再帰で呼び出していく方
  static node_sptr convert_math_string_to_symbolic_tree(const std::string &expr, std::string::size_type &now);
  
  static std::map<std::string, std::string> variable_parameter_map_;
};

} // namespace mathematica
} // namespace vcs
} // namespace hydla 

#endif //_INCLUDED_HYDLA_VCS_MATHEMATICA_EXPRESSION_CONVERTER_H_

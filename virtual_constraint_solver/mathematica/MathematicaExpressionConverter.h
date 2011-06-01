#ifndef _INCLUDED_HYDLA_VCS_MATHEMATICA_EXPRESSION_CONVERTER_H_
#define _INCLUDED_HYDLA_VCS_MATHEMATICA_EXPRESSION_CONVERTER_H_


//Mathematica文字列⇔SymbolicValueという，式の変換を担当するクラス．SymbolicValueRangeも少し．


#include "../SymbolicVirtualConstraintSolver.h"
#include "Node.h"
#include <map>
#include "TreeVisitor.h"
#include "ParseTree.h"

namespace hydla {
namespace vcs {
namespace mathematica {


class MathematicaExpressionConverter:
  public hydla::parse_tree::TreeVisitor
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

  //各ノードに対応する処理．（注：関数）
  static function_for_node for_derivative;
  static function_for_node for_unary_node;
  static function_for_node for_binary_node;

  //文字列とってvalueに変換する
  static value_t convert_math_string_to_symbolic_value(const std::string &expr);

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
  static node_sptr convert_math_string_to_symbolic_tree(const std::string &expr, std::string::size_type &now);

  //変換時に使う文字列変数
  std::string string_for_math_string_;
  //変換時に使う微分回数
  int differential_count_;
  //変換時に使う左極限判定
  int in_prev_;
  //変換時に使う前PP判定
  int in_prev_point_;



  // 比較演算子
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Equal> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::UnEqual> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Less> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::LessEqual> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Greater> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::GreaterEqual> node);

  // 論理演算子
  virtual void visit(boost::shared_ptr<hydla::parse_tree::LogicalAnd> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::LogicalOr> node);

  // 算術二項演算子
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Plus> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Subtract> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Times> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Divide> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Power> node);

  // 算術単項演算子
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Negative> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Positive> node);

  // 微分
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Differential> node);

  // 左極限
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Previous> node);
  
  // 直前のPPの値
  virtual void visit(boost::shared_ptr<hydla::parse_tree::PreviousPoint> node);
  
  // 変数
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Variable> node);

  // 数字
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Number> node);

  // 記号定数
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Parameter> node);

  // t
  virtual void visit(boost::shared_ptr<hydla::parse_tree::SymbolicT> node);
};

} // namespace mathematica
} // namespace vcs
} // namespace hydla 

#endif //_INCLUDED_HYDLA_VCS_MATHEMATICA_EXPRESSION_CONVERTER_H_

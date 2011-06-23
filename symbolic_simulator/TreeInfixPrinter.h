#ifndef TREE_INFIX__PRINTER_H_
#define TREE_INFIX__PRINTER_H_


//ツリーを中置記法で出力するクラス

#include "Node.h"
#include "TreeVisitor.h"

namespace hydla {
namespace symbolic_simulator {


class TreeInfixPrinter:
  public hydla::parse_tree::TreeVisitor
{
  public:
  typedef enum{
    PAR_NONE,
    PAR_N,
    PAR_N_P_S,
    PAR_N_P_S_T_D_P,
  }needParenthesis;

  typedef hydla::parse_tree::node_sptr node_sptr;

  
  //valueとって文字列に変換する
  std::ostream& print_infix(const node_sptr &, std::ostream&);

  private:
  
  needParenthesis need_par_;
  std::ostream *output_stream_;
  
  void print_binary_node(const hydla::parse_tree::BinaryNode &, const std::string &symbol,
                          const needParenthesis &pre_par = PAR_NONE, const needParenthesis &post_par = PAR_NONE);
  void print_unary_node(const hydla::parse_tree::UnaryNode &, const std::string &pre, const std::string &post);



  // 制約定義
  virtual void visit(boost::shared_ptr<hydla::parse_tree::ConstraintDefinition> node);
  
  // プログラム定義
  virtual void visit(boost::shared_ptr<hydla::parse_tree::ProgramDefinition> node);

  // 制約呼び出し
  virtual void visit(boost::shared_ptr<hydla::parse_tree::ConstraintCaller> node);
  
  // プログラム呼び出し
  virtual void visit(boost::shared_ptr<hydla::parse_tree::ProgramCaller> node);

  // 制約式
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Constraint> node);

  // Ask制約
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Ask> node);

  // Tell制約
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Tell> node);


  // 比較演算子
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Equal> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::UnEqual> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Less> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::LessEqual> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Greater> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::GreaterEqual> node);

  
  // 制約階層定義演算子
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Weaker> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Parallel> node);

  // 時相演算子
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Always> node);


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
  
  
  // 否定
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Not> node);
  
  // 変数
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Variable> node);

  // 数字
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Number> node);

  // 記号定数
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Parameter> node);

  // t
  virtual void visit(boost::shared_ptr<hydla::parse_tree::SymbolicT> node);
};

} // namespace symbolic_simulator
} // namespace hydla 

#endif //_INCLUDED_HYDLA_VCS_MATHEMATICA_EXPRESSION_CONVERTER_H_

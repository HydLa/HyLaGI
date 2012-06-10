#ifndef _INCLUDED_HYDLA_SIMULATOR_ASK_TYPE_ANALYZER_H_
#define _INCLUDED_HYDLA_SIMULATOR_ASK_TYPE_ANALYZER_H_

#include "TypedAsk.h"
#include "ParseTree.h"
#include "TreeVisitor.h"

namespace hydla {
namespace simulator {
  
class AskTypeAnalyzer : 
  public hydla::parse_tree::TreeVisitor
{
public:
  typedef hydla::parse_tree::node_sptr node_sptr;

  AskTypeAnalyzer();

  virtual ~AskTypeAnalyzer();
 
  /**
   * Ask制約を解析し，型付けをおこなう
   */
  void analyze(hydla::parse_tree::ParseTree* pt);

  // 呼び出し
  virtual void visit(boost::shared_ptr<hydla::parse_tree::ConstraintCaller> node);
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

  // 論理演算子
  virtual void visit(boost::shared_ptr<hydla::parse_tree::LogicalAnd> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::LogicalOr> node);
  
  // 算術二項演算子
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Plus> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Subtract> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Times> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Divide> node);
  
  // 算術単項演算子
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Negative> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Positive> node);
  
  // 制約階層定義演算子
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Weaker> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Parallel> node);

  // 時相演算子
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Always> node);
  
  // 微分
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Differential> node);

  // 左極限
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Previous> node);
  
  // 変数
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Variable> node);

  // 数字
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Number> node);
  //Print
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Print> node);

private:  
  void dispatch(boost::shared_ptr<hydla::parse_tree::UnaryNode> node);
  void dispatch(boost::shared_ptr<hydla::parse_tree::BinaryNode> node);
    
  /**
   * 新しい子ノード
   * accept後、これに値が入っている場合はノードの値を交換する
   */
  node_sptr new_child_;

  /**
   * 検索をおこなったノードの中にprev制約が存在したかどうか
   */
  bool exist_prev_cons_;
};

} //namespace simulator
} //namespace hydla 

#endif // _INCLUDED_HYDLA_SIMULATOR_ASK_TYPE_ANALYZER_H_

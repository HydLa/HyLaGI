#ifndef _INCLUDED_HYDLA_CONSISTENCY_CHECKER_H_
#define _INCLUDED_HYDLA_CONSISTENCY_CHECKER_H_

#include "Node.h"
#include "TreeVisitor.h"
#include "mathlink_helper.h"
#include "ParseTree.h"
#include <map>
#include "Types.h"

namespace hydla {
namespace symbolic_simulator {

class ConsistencyChecker : public parse_tree::TreeVisitor {
public:
  ConsistencyChecker(MathLink& ml);

  virtual ~ConsistencyChecker();

  bool is_consistent(hydla::simulator::collected_tells_t& collected_tells);

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

  // 微分
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Differential> node);

  // 左極限
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Previous> node);
  
  // 変数
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Variable> node);

  // 数字
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Number> node);


private:
  MathLink& ml_;
  std::multimap<std::string, int> vars_;
  // 微分方程式の中にいるかどうか（PointPhaseでは不要？）
  bool in_differential_equality_;
  // Differentialノードを何回通ったか（何階微分か）
  int differential_count_;
  // Prevノードの下にいるかどうか
  // （通常変数なら1、prev変数だと-1などにするか？）
  bool in_prev_;

};

} //namespace symbolic_simulator
} // namespace hydla

#endif //_INCLUDED_HYDLA_CONSISTENCY_CHECKER_H__


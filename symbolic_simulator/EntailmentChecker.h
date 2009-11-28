#ifndef _INCLUDED_HYDLA_ENTAILMENT_CHECKER_H_
#define _INCLUDED_HYDLA_ENTAILMENT_CHECKER_H_

#include "Node.h"
#include "TreeVisitor.h"
#include "mathlink_helper.h"
#include "ParseTree.h"
#include <map>
#include "Types.h"
#include <boost/shared_ptr.hpp>


using namespace hydla::parse_tree;

namespace hydla {
namespace symbolic_simulator {

class EntailmentChecker : public parse_tree::TreeVisitor {
public:
  EntailmentChecker(MathLink& ml);

  virtual ~EntailmentChecker();

  bool check_entailment(
    boost::shared_ptr<hydla::parse_tree::Ask> negative_ask,
    hydla::simulator::collected_tells_t& collected_tells);

  // Ask制約
  virtual void visit(boost::shared_ptr<Ask> node);

  // Tell制約
  virtual void visit(boost::shared_ptr<Tell> node);

  // 比較演算子
  virtual void visit(boost::shared_ptr<Equal> node);
  virtual void visit(boost::shared_ptr<UnEqual> node);
  virtual void visit(boost::shared_ptr<Less> node);
  virtual void visit(boost::shared_ptr<LessEqual> node);
  virtual void visit(boost::shared_ptr<Greater> node);
  virtual void visit(boost::shared_ptr<GreaterEqual> node);

  // 論理演算子
  virtual void visit(boost::shared_ptr<LogicalAnd> node);
  virtual void visit(boost::shared_ptr<LogicalOr> node);

  // 算術二項演算子
  virtual void visit(boost::shared_ptr<Plus> node);
  virtual void visit(boost::shared_ptr<Subtract> node);
  virtual void visit(boost::shared_ptr<Times> node);
  virtual void visit(boost::shared_ptr<Divide> node);

  // 算術単項演算子
  virtual void visit(boost::shared_ptr<Negative> node);
  virtual void visit(boost::shared_ptr<Positive> node);

  // 微分
  virtual void visit(boost::shared_ptr<Differential> node);

  // 左極限
  virtual void visit(boost::shared_ptr<Previous> node);
  
  // 変数
  virtual void visit(boost::shared_ptr<Variable> node);

  // 数字
  virtual void visit(boost::shared_ptr<Number> node);


private:
  MathLink& ml_;
  std::multimap<std::string, int> vars_;
  // 微分方程式の中にいるかどうか（PointPhaseでは不要？）
  bool in_differential_equality_;
  // Differentialノードを何回通ったか（何階微分か）
  int differential_count_;
  // Prevノードの下にいるかどうか
  bool in_prev_;
  // ask制約のガードの中にいるかどうか
  bool in_guard_;

};

} //namespace symbolic_simulator
} // namespace hydla

#endif //_INCLUDED_HYDLA_ENTAILMENT_CHECKER_H__


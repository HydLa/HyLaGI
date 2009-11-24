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

  // Tell§–ñ
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Tell> node);

  // ”äŠr‰‰Zq
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Equal> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::UnEqual> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Less> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::LessEqual> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Greater> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::GreaterEqual> node);

  // ˜_—‰‰Zq
  virtual void visit(boost::shared_ptr<hydla::parse_tree::LogicalAnd> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::LogicalOr> node);

  // Zp“ñ€‰‰Zq
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Plus> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Subtract> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Times> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Divide> node);

  // Zp’P€‰‰Zq
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Negative> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Positive> node);

  // ”÷•ª
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Differential> node);

  // ¶‹ÉŒÀ
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Previous> node);
  
  // •Ï”
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Variable> node);

  // ”š
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Number> node);


private:
  MathLink& ml_;
  std::map<std::string, int> vars_;
  int in_differential_equality_;
  int in_differential_;

};

} //namespace symbolic_simulator
} // namespace hydla

#endif //_INCLUDED_HYDLA_CONSISTENCY_CHECKER_H__


#ifndef _INCLUDED_HYDLA_COLLECT_TELL_VISITOR_H_
#define _INCLUDED_HYDLA_COLLECT_TELL_VISITOR_H_

#include "Node.h"
#include "TreeVisitor.h"
#include "mathlink_helper.h"
#include "ParseTree.h"
#include <set>

using namespace hydla::parse_tree;

namespace hydla {
namespace symbolic_simulator {

class CollectTellVisitor : public parse_tree::TreeVisitor {
public:
  CollectTellVisitor(ParseTree& parse_tree, MathLink& ml);

  virtual ~CollectTellVisitor();

  bool is_consistent();

  // ’è‹`
  virtual void visit(boost::shared_ptr<ConstraintDefinition> node);
  virtual void visit(boost::shared_ptr<ProgramDefinition> node);

  // ŒÄ‚Ño‚µ
  virtual void visit(boost::shared_ptr<ConstraintCaller> node);
  virtual void visit(boost::shared_ptr<ProgramCaller> node);     

  // §–ñ®
  virtual void visit(boost::shared_ptr<Constraint> node);

  // Ask§–ñ
  virtual void visit(boost::shared_ptr<Ask> node);

  // Tell§–ñ
  virtual void visit(boost::shared_ptr<Tell> node);

  // ”äŠr‰‰Zq
  virtual void visit(boost::shared_ptr<Equal> node);
  virtual void visit(boost::shared_ptr<UnEqual> node);
  virtual void visit(boost::shared_ptr<Less> node);
  virtual void visit(boost::shared_ptr<LessEqual> node);
  virtual void visit(boost::shared_ptr<Greater> node);
  virtual void visit(boost::shared_ptr<GreaterEqual> node);

  // ˜_—‰‰Zq
  virtual void visit(boost::shared_ptr<LogicalAnd> node);
  virtual void visit(boost::shared_ptr<LogicalOr> node);

  // Zp“ñ€‰‰Zq
  virtual void visit(boost::shared_ptr<Plus> node);
  virtual void visit(boost::shared_ptr<Subtract> node);
  virtual void visit(boost::shared_ptr<Times> node);
  virtual void visit(boost::shared_ptr<Divide> node);

  // Zp’P€‰‰Zq
  virtual void visit(boost::shared_ptr<Negative> node);
  virtual void visit(boost::shared_ptr<Positive> node);

  // §–ñŠK‘w’è‹`‰‰Zq
  virtual void visit(boost::shared_ptr<Weaker> node);
  virtual void visit(boost::shared_ptr<Parallel> node);

  // ‘Š‰‰Zq
  virtual void visit(boost::shared_ptr<Always> node);

  // ”÷•ª
  virtual void visit(boost::shared_ptr<Differential> node);

  // ¶‹ÉŒÀ
  virtual void visit(boost::shared_ptr<Previous> node);
  
  // •Ï”
  virtual void visit(boost::shared_ptr<Variable> node);

  // ”š
  virtual void visit(boost::shared_ptr<Number> node);


private:
  MathLink& ml_;
  ParseTree& parse_tree_;
  std::set<std::string> vars_;
  int in_differential_equality_;
  int in_differential_;

};

} //namespace symbolic_simulator
} // namespace hydla

#endif //_INCLUDED_HYDLA_COLLECT_TELL_VISITOR_H__


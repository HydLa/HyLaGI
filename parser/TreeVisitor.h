#ifndef _INCLUDED_HYDLA_PARSE_TREE_TREE_VISITOR_H_
#define _INCLUDED_HYDLA_PARSE_TREE_TREE_VISITOR_H_

#include <boost/shared_ptr.hpp>

#include "Node.h"

namespace hydla { 
namespace parse_tree {
  
/**
 * ParseTree‚Ìƒm[ƒhW‡‚É‘Î‚·‚éVisitorƒNƒ‰ƒX
 */
class TreeVisitor {
public:
  TreeVisitor();

  virtual ~TreeVisitor();

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

};

} //namespace parse_tree
} //namespace hydla

#endif //_INCLUDED_HYDLA_PARSE_TREE_TREE_VISITOR_H_

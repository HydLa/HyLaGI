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
  TreeVisitor()
  {}

  virtual ~TreeVisitor()
  {}

  // ’è‹`
  virtual void visit(ConstraintDefinition* node)  {}
  virtual void visit(ProgramDefinition* node)     {}

  // ŒÄ‚Ño‚µ
  virtual void visit(ConstraintCaller* node)      {}
  virtual void visit(ProgramCaller* node)         {}

  // §–ñ®
  virtual void visit(Constraint* node)            {}

  // Ask§–ñ
  virtual void visit(Ask* node)                   {}

  // Tell§–ñ
  virtual void visit(Tell* node)                  {}

  // ”äŠr‰‰Zq
  virtual void visit(Equal* node)                 {}
  virtual void visit(UnEqual* node)               {}
  virtual void visit(Less* node)                  {}
  virtual void visit(LessEqual* node)             {}
  virtual void visit(Greater* node)               {}
  virtual void visit(GreaterEqual* node)          {}

  // ˜_—‰‰Zq
  virtual void visit(LogicalAnd* node)            {}
  virtual void visit(LogicalOr* node)             {}
  
  // Zp“ñ€‰‰Zq
  virtual void visit(Plus* node)                  {}
  virtual void visit(Subtract* node)              {}
  virtual void visit(Times* node)                 {}
  virtual void visit(Divide* node)                {}
  
  // Zp’P€‰‰Zq
  virtual void visit(Negative* node)              {}
  virtual void visit(Positive* node)              {}
  
  // §–ñŠK‘w’è‹`‰‰Zq
  virtual void visit(Weaker* node)                {}
  virtual void visit(Parallel* node)              {}

  // ‘Š‰‰Zq
  virtual void visit(Always* node)                {}
  
  // ”÷•ª
  virtual void visit(Differential* node)          {}

  // ¶‹ÉŒÀ
  virtual void visit(Previous* node)              {}
  
  // •Ï”
  virtual void visit(Variable* node)              {}

  // ”š
  virtual void visit(Number* node)                {}

};

} //namespace parse_tree
} //namespace hydla

#endif //_INCLUDED_HYDLA_PARSE_TREE_TREE_VISITOR_H_
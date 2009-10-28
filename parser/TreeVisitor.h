#ifndef _INCLUDED_HYDLA_PARSE_TREE_TREE_VISITOR_H_
#define _INCLUDED_HYDLA_PARSE_TREE_TREE_VISITOR_H_

#include <boost/shared_ptr.hpp>

#include "Node.h"

namespace hydla { 
namespace parse_tree {
  
/**
 * ParseTreeฬm[hWษฮท้VisitorNX
 */
class TreeVisitor {
public:
  TreeVisitor()
  {}

  virtual ~TreeVisitor()
  {}

  // ่`
  virtual void visit(ConstraintDefinition* node)  {}
  virtual void visit(ProgramDefinition* node)     {}

  // ฤัoต
  virtual void visit(ConstraintCaller* node)      {}
  virtual void visit(ProgramCaller* node)         {}

  // ง๑ฎ
  virtual void visit(Constraint* node)            {}

  // Askง๑
  virtual void visit(Ask* node)                   {}

  // Tellง๑
  virtual void visit(Tell* node)                  {}

  // ไrZq
  virtual void visit(Equal* node)                 {}
  virtual void visit(UnEqual* node)               {}
  virtual void visit(Less* node)                  {}
  virtual void visit(LessEqual* node)             {}
  virtual void visit(Greater* node)               {}
  virtual void visit(GreaterEqual* node)          {}

  // _Zq
  virtual void visit(LogicalAnd* node)            {}
  virtual void visit(LogicalOr* node)             {}
  
  // Zp๑Zq
  virtual void visit(Plus* node)                  {}
  virtual void visit(Subtract* node)              {}
  virtual void visit(Times* node)                 {}
  virtual void visit(Divide* node)                {}
  
  // ZpPZq
  virtual void visit(Negative* node)              {}
  virtual void visit(Positive* node)              {}
  
  // ง๑Kw่`Zq
  virtual void visit(Weaker* node)                {}
  virtual void visit(Parallel* node)              {}

  // Zq
  virtual void visit(Always* node)                {}
  
  // ๗ช
  virtual void visit(Differential* node)          {}

  // ถษภ
  virtual void visit(Previous* node)              {}
  
  // ฯ
  virtual void visit(Variable* node)              {}

  // 
  virtual void visit(Number* node)                {}

};

} //namespace parse_tree
} //namespace hydla

#endif //_INCLUDED_HYDLA_PARSE_TREE_TREE_VISITOR_H_
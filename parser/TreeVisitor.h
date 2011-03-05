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
  TreeVisitor();

  virtual ~TreeVisitor();

  /**
   * NodeNXฬacceptึฤัoตpwpึ
   */
  template<class T>
  void accept(const T& n)
  {
    n->accept(n, this);
  }

  // ง๑่`
  virtual void visit(boost::shared_ptr<ConstraintDefinition> node);
  
  // vO่`
  virtual void visit(boost::shared_ptr<ProgramDefinition> node);

  // ง๑ฤัoต
  virtual void visit(boost::shared_ptr<ConstraintCaller> node);
  
  // vOฤัoต
  virtual void visit(boost::shared_ptr<ProgramCaller> node);

  // ง๑ฎ
  virtual void visit(boost::shared_ptr<Constraint> node);

  // Askง๑
  virtual void visit(boost::shared_ptr<Ask> node);

  // Tellง๑
  virtual void visit(boost::shared_ptr<Tell> node);

  // ไrZq
  virtual void visit(boost::shared_ptr<Equal> node);
  virtual void visit(boost::shared_ptr<UnEqual> node);
  virtual void visit(boost::shared_ptr<Less> node);
  virtual void visit(boost::shared_ptr<LessEqual> node);
  virtual void visit(boost::shared_ptr<Greater> node);
  virtual void visit(boost::shared_ptr<GreaterEqual> node);

  // _Zq
  virtual void visit(boost::shared_ptr<LogicalAnd> node);
  virtual void visit(boost::shared_ptr<LogicalOr> node);
  
  // Zp๑Zq
  virtual void visit(boost::shared_ptr<Plus> node);
  virtual void visit(boost::shared_ptr<Subtract> node);
  virtual void visit(boost::shared_ptr<Times> node);
  virtual void visit(boost::shared_ptr<Divide> node);
  virtual void visit(boost::shared_ptr<Power> node);
  
  // ZpPZq
  virtual void visit(boost::shared_ptr<Negative> node);
  virtual void visit(boost::shared_ptr<Positive> node);
  
  // ง๑Kw่`Zq
  virtual void visit(boost::shared_ptr<Weaker> node);
  virtual void visit(boost::shared_ptr<Parallel> node);

  // Zq
  virtual void visit(boost::shared_ptr<Always> node);
  
  // ๗ช
  virtual void visit(boost::shared_ptr<Differential> node);

  // ถษภ
  virtual void visit(boost::shared_ptr<Previous> node);
  
  // ฯ
  virtual void visit(boost::shared_ptr<Variable> node);

  // 
  virtual void visit(boost::shared_ptr<Number> node);
  
  // L่
  virtual void visit(boost::shared_ptr<Parameter> node);
  
  // tiิjDภฑเหฤ
  virtual void visit(boost::shared_ptr<SymbolicT> node);
};

} //namespace parse_tree
} //namespace hydla

#endif //_INCLUDED_HYDLA_PARSE_TREE_TREE_VISITOR_H_

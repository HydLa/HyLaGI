#ifndef _INCLUDED_HYDLA_PARSE_TREE_TREE_VISITOR_H_
#define _INCLUDED_HYDLA_PARSE_TREE_TREE_VISITOR_H_

#include <boost/shared_ptr.hpp>

#include "Node.h"

namespace hydla { 
namespace parse_tree {
  
/**
 * ParseTreeÌm[hWÉÎ·éVisitorNX
 */
class TreeVisitor {
public:
  TreeVisitor();

  virtual ~TreeVisitor();

  /**
   * NodeNXÌacceptÖÄÑoµpwpÖ
   */
  template<class T>
  void accept(const T& n)
  {
    n->accept(n, this);
  }

  // §ñè`
  virtual void visit(boost::shared_ptr<ConstraintDefinition> node);
  
  // vOè`
  virtual void visit(boost::shared_ptr<ProgramDefinition> node);

  // §ñÄÑoµ
  virtual void visit(boost::shared_ptr<ConstraintCaller> node);
  
  // vOÄÑoµ
  virtual void visit(boost::shared_ptr<ProgramCaller> node);

  // §ñ®
  virtual void visit(boost::shared_ptr<Constraint> node);

  // Ask§ñ
  virtual void visit(boost::shared_ptr<Ask> node);

  // Tell§ñ
  virtual void visit(boost::shared_ptr<Tell> node);

  // ärZq
  virtual void visit(boost::shared_ptr<Equal> node);
  virtual void visit(boost::shared_ptr<UnEqual> node);
  virtual void visit(boost::shared_ptr<Less> node);
  virtual void visit(boost::shared_ptr<LessEqual> node);
  virtual void visit(boost::shared_ptr<Greater> node);
  virtual void visit(boost::shared_ptr<GreaterEqual> node);

  // _Zq
  virtual void visit(boost::shared_ptr<LogicalAnd> node);
  virtual void visit(boost::shared_ptr<LogicalOr> node);
  
  // ZpñZq
  virtual void visit(boost::shared_ptr<Plus> node);
  virtual void visit(boost::shared_ptr<Subtract> node);
  virtual void visit(boost::shared_ptr<Times> node);
  virtual void visit(boost::shared_ptr<Divide> node);
  virtual void visit(boost::shared_ptr<Power> node);
  
  // ZpPZq
  virtual void visit(boost::shared_ptr<Negative> node);
  virtual void visit(boost::shared_ptr<Positive> node);
  
  // §ñKwè`Zq
  virtual void visit(boost::shared_ptr<Weaker> node);
  virtual void visit(boost::shared_ptr<Parallel> node);

  // Zq
  virtual void visit(boost::shared_ptr<Always> node);
  
  // ÷ª
  virtual void visit(boost::shared_ptr<Differential> node);

  // ¶ÉÀ
  virtual void visit(boost::shared_ptr<Previous> node);
  
  //Print
  virtual void visit(boost::shared_ptr<Print> node);
  virtual void visit(boost::shared_ptr<PrintPP> node);
  virtual void visit(boost::shared_ptr<PrintIP> node);
    
  virtual void visit(boost::shared_ptr<Scan> node);
  virtual void visit(boost::shared_ptr<Exit> node);
  virtual void visit(boost::shared_ptr<Abort> node);
  
  // Ûè
  virtual void visit(boost::shared_ptr<Not> node);
  
  // ~ü¦
  virtual void visit(boost::shared_ptr<Pi> node);
  // ©RÎÌê
  virtual void visit(boost::shared_ptr<E> node);
  
  //CÓÌ¶ñ
  virtual void visit(boost::shared_ptr<ArbitraryNode> node);

  // Ö
  virtual void visit(boost::shared_ptr<Function> node);
  virtual void visit(boost::shared_ptr<UnsupportedFunction> node);

  // Ï
  virtual void visit(boost::shared_ptr<Variable> node);

  // 
  virtual void visit(boost::shared_ptr<Number> node);
  
  // Lè
  virtual void visit(boost::shared_ptr<Parameter> node);
  
  // tiÔj
  virtual void visit(boost::shared_ptr<SymbolicT> node);
  
  // ³Àå
  virtual void visit(boost::shared_ptr<Infinity> node);
};

} //namespace parse_tree
} //namespace hydla

#endif //_INCLUDED_HYDLA_PARSE_TREE_TREE_VISITOR_H_

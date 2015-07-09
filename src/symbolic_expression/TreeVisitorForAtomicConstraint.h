#pragma once

#include "DefaultTreeVisitor.h"

namespace hydla { 
namespace symbolic_expression {

/**
 * 原子制約に対して
 */
class TreeVisitorForAtomicConstraint: public DefaultTreeVisitor {
public:
  TreeVisitorForAtomicConstraint();

  virtual ~TreeVisitorForAtomicConstraint();

  // 比較演算子
  virtual void visit(boost::shared_ptr<Equal> node);
  virtual void visit(boost::shared_ptr<UnEqual> node);
  virtual void visit(boost::shared_ptr<Less> node);
  virtual void visit(boost::shared_ptr<LessEqual> node);
  virtual void visit(boost::shared_ptr<Greater> node);
  virtual void visit(boost::shared_ptr<GreaterEqual> node);
  
  virtual void visit(boost::shared_ptr<True> node);
  virtual void visit(boost::shared_ptr<False> node);

  virtual void visit_atomic_constraint(boost::shared_ptr<Node> node) = 0;
};

} //namespace symbolic_expression
} //namespace hydla


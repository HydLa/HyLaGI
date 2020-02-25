#include "TreeVisitorForAtomicConstraint.h"


namespace hydla { 
namespace symbolic_expression {

TreeVisitorForAtomicConstraint::TreeVisitorForAtomicConstraint()
{}

TreeVisitorForAtomicConstraint::~TreeVisitorForAtomicConstraint()
{}


// 比較演算子
void TreeVisitorForAtomicConstraint::visit(std::shared_ptr<Equal> node)
{visit_atomic_constraint(node);}
void TreeVisitorForAtomicConstraint::visit(std::shared_ptr<UnEqual> node)       
{visit_atomic_constraint(node);}
void TreeVisitorForAtomicConstraint::visit(std::shared_ptr<Less> node)
{visit_atomic_constraint(node);}
void TreeVisitorForAtomicConstraint::visit(std::shared_ptr<LessEqual> node)
{visit_atomic_constraint(node);}
void TreeVisitorForAtomicConstraint::visit(std::shared_ptr<Greater> node)
{visit_atomic_constraint(node);}
void TreeVisitorForAtomicConstraint::visit(std::shared_ptr<GreaterEqual> node)  
{visit_atomic_constraint(node);}
void TreeVisitorForAtomicConstraint::visit(std::shared_ptr<True> node)
{visit_atomic_constraint(node);}
void TreeVisitorForAtomicConstraint::visit(std::shared_ptr<False> node)
{visit_atomic_constraint(node);}

} //namespace symbolic_expression
} //namespace hydla

#include "AtomicConstraintFinder.h"
#include "Logger.h"

using namespace std;

namespace hydla {
namespace simulator {

AtomicConstraintFinder::AtomicConstraintFinder() {}

AtomicConstraintFinder::~AtomicConstraintFinder() {}

void AtomicConstraintFinder::visit_node(std::shared_ptr<symbolic_expression::Node> node) {
  accept(node);
}

void AtomicConstraintFinder::clear() {
  constraints_.clear();
}

bool AtomicConstraintFinder::include_constraint(const constraint_t &con) const {
  for(auto c : constraints_) {
    // same_structで頑張る
    if(c->is_same_struct(*con,true)) return true;
  }
  return false;
}

constraints_t AtomicConstraintFinder::get_all_constraint_set() const { return constraints_; }

// 比較演算子
void AtomicConstraintFinder::visit(std::shared_ptr<symbolic_expression::Equal> node) { constraints_.insert(node); }
void AtomicConstraintFinder::visit(std::shared_ptr<symbolic_expression::UnEqual> node) { constraints_.insert(node); }
void AtomicConstraintFinder::visit(std::shared_ptr<symbolic_expression::Less> node) { constraints_.insert(node); }
void AtomicConstraintFinder::visit(std::shared_ptr<symbolic_expression::LessEqual> node) { constraints_.insert(node); }
void AtomicConstraintFinder::visit(std::shared_ptr<symbolic_expression::Greater> node) { constraints_.insert(node); }
void AtomicConstraintFinder::visit(std::shared_ptr<symbolic_expression::GreaterEqual> node) { constraints_.insert(node); }

} //namespace simulator
} //namespace hydla 

#include "BaseNodeVisitor.h"

#include <assert.h>

namespace hydla {
namespace symbolic_expression {

BaseNodeVisitor::BaseNodeVisitor() {}

BaseNodeVisitor::~BaseNodeVisitor() {}

void BaseNodeVisitor::visit(std::shared_ptr<FactorNode> node) { assert(0); }
void BaseNodeVisitor::visit(std::shared_ptr<UnaryNode> node) { assert(0); }
void BaseNodeVisitor::visit(std::shared_ptr<BinaryNode> node) { assert(0); }
void BaseNodeVisitor::visit(std::shared_ptr<VariadicNode> node) { assert(0); }

} // namespace symbolic_expression
} // namespace hydla

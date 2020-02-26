#include "VariableRenamer.h"
#include "Logger.h"

using namespace std;
using namespace hydla::symbolic_expression;

namespace hydla {
namespace simulator {

VariableRenamer::VariableRenamer() {}

VariableRenamer::~VariableRenamer() {}

void VariableRenamer::rename_exists(
    std::shared_ptr<symbolic_expression::Node> node, int phase_num) {
  phase_num_ = phase_num;
  diff_cnt_ = 0;
  accept(node);
}

void VariableRenamer::clear() { exists_variable_map_.clear(); }

map<Variable, Variable> VariableRenamer::get_renamed_variable_map() const {
  return exists_variable_map_;
}

void VariableRenamer::visit(
    std::shared_ptr<symbolic_expression::Exists> node) {
  string varname =
      std::dynamic_pointer_cast<hydla::symbolic_expression::Variable>(
          node->get_variable())
          ->get_name();
  exists_variable_set_.insert(varname);
  dispatch_rhs(node);
  exists_variable_set_.erase(varname);
}

// 変数
void VariableRenamer::visit(
    std::shared_ptr<symbolic_expression::Variable> node) {
  string varname = node->get_name();
  auto it = exists_variable_set_.find(varname);
  if (it != exists_variable_set_.end()) {
    string new_name = varname + to_string(phase_num_);
    new_child_ = hydla::symbolic_expression::node_sptr(
        new hydla::symbolic_expression::Variable(new_name));
    Variable new_var = Variable(new_name, diff_cnt_);
    exists_variable_map_[Variable(varname, diff_cnt_)] = new_var;
  }
}

void VariableRenamer::visit(
    std::shared_ptr<symbolic_expression::Differential> node) {
  diff_cnt_++;
  dispatch_child(node);
  diff_cnt_--;
}

#define DEFINE_DEFAULT_VISIT_ARBITRARY(NODE_NAME)                              \
  void VariableRenamer::visit(std::shared_ptr<symbolic_expression::NODE_NAME> node) {             \
    for (int i = 0; i < node->get_arguments_size(); i++) {                     \
      accept(node->get_argument(i));                                           \
      if (new_child_) {                                                        \
        node->set_argument((new_child_), i);                                   \
        new_child_.reset();                                                    \
      }                                                                        \
    }                                                                          \
  }

#define DEFINE_DEFAULT_VISIT_BINARY(NODE_NAME)                                 \
  void VariableRenamer::visit(std::shared_ptr<symbolic_expression::NODE_NAME> node) {             \
    dispatch_lhs(node);                                                        \
    dispatch_rhs(node);                                                        \
  }

#define DEFINE_DEFAULT_VISIT_UNARY(NODE_NAME)                                  \
  void VariableRenamer::visit(std::shared_ptr<symbolic_expression::NODE_NAME> node) {             \
    dispatch_child(node);                                                      \
  }

#define DEFINE_DEFAULT_VISIT_FACTOR(NODE_NAME)                                 \
  void VariableRenamer::visit(std::shared_ptr<symbolic_expression::NODE_NAME> node) {}

DEFINE_DEFAULT_VISIT_ARBITRARY(Function)
DEFINE_DEFAULT_VISIT_ARBITRARY(UnsupportedFunction)

DEFINE_DEFAULT_VISIT_UNARY(Negative)
DEFINE_DEFAULT_VISIT_UNARY(Positive)

DEFINE_DEFAULT_VISIT_UNARY(ConstraintDefinition)
DEFINE_DEFAULT_VISIT_UNARY(ProgramDefinition)
DEFINE_DEFAULT_VISIT_UNARY(ConstraintCaller)
DEFINE_DEFAULT_VISIT_UNARY(ProgramCaller)
DEFINE_DEFAULT_VISIT_UNARY(Constraint)

DEFINE_DEFAULT_VISIT_BINARY(Ask)
DEFINE_DEFAULT_VISIT_UNARY(Tell)

DEFINE_DEFAULT_VISIT_BINARY(Plus)
DEFINE_DEFAULT_VISIT_BINARY(Subtract)
DEFINE_DEFAULT_VISIT_BINARY(Times)
DEFINE_DEFAULT_VISIT_BINARY(Divide)
DEFINE_DEFAULT_VISIT_BINARY(Power)

DEFINE_DEFAULT_VISIT_UNARY(Previous)

DEFINE_DEFAULT_VISIT_BINARY(Less)
DEFINE_DEFAULT_VISIT_BINARY(LessEqual)
DEFINE_DEFAULT_VISIT_BINARY(Greater)
DEFINE_DEFAULT_VISIT_BINARY(GreaterEqual)
DEFINE_DEFAULT_VISIT_BINARY(Equal)
DEFINE_DEFAULT_VISIT_BINARY(UnEqual)

DEFINE_DEFAULT_VISIT_BINARY(LogicalOr)
DEFINE_DEFAULT_VISIT_BINARY(LogicalAnd)

DEFINE_DEFAULT_VISIT_BINARY(Weaker)
DEFINE_DEFAULT_VISIT_BINARY(Parallel)

DEFINE_DEFAULT_VISIT_UNARY(Always)

DEFINE_DEFAULT_VISIT_FACTOR(Float)
DEFINE_DEFAULT_VISIT_FACTOR(True)
DEFINE_DEFAULT_VISIT_FACTOR(False)

DEFINE_DEFAULT_VISIT_FACTOR(Pi)
DEFINE_DEFAULT_VISIT_FACTOR(E)
DEFINE_DEFAULT_VISIT_FACTOR(Parameter)
DEFINE_DEFAULT_VISIT_FACTOR(SymbolicT)
DEFINE_DEFAULT_VISIT_FACTOR(Number)
DEFINE_DEFAULT_VISIT_FACTOR(SVtimer)
DEFINE_DEFAULT_VISIT_FACTOR(Infinity)

DEFINE_DEFAULT_VISIT_FACTOR(Print)
DEFINE_DEFAULT_VISIT_FACTOR(PrintIP)
DEFINE_DEFAULT_VISIT_FACTOR(PrintPP)
DEFINE_DEFAULT_VISIT_FACTOR(Scan)
DEFINE_DEFAULT_VISIT_FACTOR(Exit)
DEFINE_DEFAULT_VISIT_FACTOR(Abort)

} // namespace simulator
} // namespace hydla

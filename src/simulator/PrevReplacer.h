#pragma once

#include <set>
#include <sstream>

#include <memory>

#include "DefaultTreeVisitor.h"
#include "Node.h"
#include "PhaseResult.h"
#include "Simulator.h"

namespace hydla {
namespace simulator {

/**
 * A class to replace prevs with parameters.
 * (and introduce parameter)
 */
class PrevReplacer : public symbolic_expression::DefaultTreeVisitor {
  typedef symbolic_expression::node_sptr node_sptr;

public:
  PrevReplacer(PhaseResult &phase, Simulator &simulator, backend::Backend *b,
               bool affine);

  virtual ~PrevReplacer();

  bool replace_value(value_t &val);
  void replace_node(symbolic_expression::node_sptr &exp);
  ConstraintStore get_parameter_constraint() const;

  virtual void
  visit(std::shared_ptr<symbolic_expression::ConstraintDefinition> node);
  virtual void
  visit(std::shared_ptr<symbolic_expression::ProgramDefinition> node);
  virtual void
  visit(std::shared_ptr<symbolic_expression::ConstraintCaller> node);
  virtual void visit(std::shared_ptr<symbolic_expression::ProgramCaller> node);
  virtual void visit(std::shared_ptr<symbolic_expression::Constraint> node);

  virtual void visit(std::shared_ptr<symbolic_expression::Ask> node);
  virtual void visit(std::shared_ptr<symbolic_expression::Exists> node);
  virtual void visit(std::shared_ptr<symbolic_expression::Tell> node);

  virtual void visit(std::shared_ptr<symbolic_expression::LogicalAnd> node);
  virtual void visit(std::shared_ptr<symbolic_expression::LogicalOr> node);

  virtual void visit(std::shared_ptr<symbolic_expression::Weaker> node);
  virtual void visit(std::shared_ptr<symbolic_expression::Parallel> node);
  virtual void visit(std::shared_ptr<symbolic_expression::Always> node);
  virtual void visit(std::shared_ptr<symbolic_expression::Float> node);
  virtual void visit(std::shared_ptr<symbolic_expression::True> node);
  virtual void visit(std::shared_ptr<symbolic_expression::False> node);

  virtual void visit(std::shared_ptr<symbolic_expression::Print> node);
  virtual void visit(std::shared_ptr<symbolic_expression::PrintPP> node);
  virtual void visit(std::shared_ptr<symbolic_expression::PrintIP> node);
  virtual void visit(std::shared_ptr<symbolic_expression::Scan> node);
  virtual void visit(std::shared_ptr<symbolic_expression::Exit> node);
  virtual void visit(std::shared_ptr<symbolic_expression::Abort> node);

  virtual void visit(std::shared_ptr<symbolic_expression::Plus> node);
  virtual void visit(std::shared_ptr<symbolic_expression::Subtract> node);
  virtual void visit(std::shared_ptr<symbolic_expression::Times> node);
  virtual void visit(std::shared_ptr<symbolic_expression::Divide> node);
  virtual void visit(std::shared_ptr<symbolic_expression::Power> node);

  virtual void visit(std::shared_ptr<symbolic_expression::Less> node);
  virtual void visit(std::shared_ptr<symbolic_expression::LessEqual> node);
  virtual void visit(std::shared_ptr<symbolic_expression::Greater> node);
  virtual void visit(std::shared_ptr<symbolic_expression::GreaterEqual> node);
  virtual void visit(std::shared_ptr<symbolic_expression::Equal> node);
  virtual void visit(std::shared_ptr<symbolic_expression::UnEqual> node);

  virtual void visit(std::shared_ptr<symbolic_expression::Negative> node);
  virtual void visit(std::shared_ptr<symbolic_expression::Positive> node);

  virtual void visit(std::shared_ptr<symbolic_expression::Differential> node);

  virtual void visit(std::shared_ptr<symbolic_expression::Function> node);
  virtual void
  visit(std::shared_ptr<symbolic_expression::UnsupportedFunction> node);

  virtual void visit(std::shared_ptr<symbolic_expression::Variable> node);

  virtual void visit(std::shared_ptr<symbolic_expression::Pi> node);
  virtual void visit(std::shared_ptr<symbolic_expression::E> node);
  virtual void visit(std::shared_ptr<symbolic_expression::Number> node);
  virtual void visit(std::shared_ptr<symbolic_expression::Parameter> node);
  virtual void visit(std::shared_ptr<symbolic_expression::SymbolicT> node);
  virtual void visit(std::shared_ptr<symbolic_expression::Infinity> node);
  virtual void visit(std::shared_ptr<symbolic_expression::SVtimer> node);
  virtual void visit(std::shared_ptr<symbolic_expression::Previous> node);

private:
  ConstraintStore parameter_constraint;
  int differential_cnt;
  bool in_prev;
  PhaseResult &prev_phase;
  Simulator &simulator;
  bool replaced;
  backend::Backend *backend;
  bool affine_mode = false;

  symbolic_expression::node_sptr new_child;

  template <class C, const symbolic_expression::node_sptr &(C::*getter)() const,
            void (C::*setter)(const symbolic_expression::node_sptr &child)>
  void dispatch(C *n) {
    accept((n->*getter)());
    if (new_child) {
      (n->*setter)(new_child);
      new_child.reset();
    }
  }

  template <class NodeType> void dispatch_child(NodeType &node) {
    dispatch<symbolic_expression::UnaryNode,
             &symbolic_expression::UnaryNode::get_child,
             &symbolic_expression::UnaryNode::set_child>(node.get());
  }

  template <class NodeType> void dispatch_rhs(NodeType &node) {
    dispatch<symbolic_expression::BinaryNode,
             &symbolic_expression::BinaryNode::get_rhs,
             &symbolic_expression::BinaryNode::set_rhs>(node.get());
  }

  template <class NodeType> void dispatch_lhs(NodeType &node) {
    dispatch<symbolic_expression::BinaryNode,
             &symbolic_expression::BinaryNode::get_lhs,
             &symbolic_expression::BinaryNode::set_lhs>(node.get());
  }
};

} // namespace simulator
} // namespace hydla

#pragma once

#include <set>
#include <sstream>

#include <boost/shared_ptr.hpp>

#include "DefaultTreeVisitor.h"
#include "Node.h"
#include "Variable.h"

#include "Value.h"

namespace hydla {
namespace simulator {

/**
 * 変数名の書き換えをするクラス(存在量化子の変数名の書き換えに使ってる)
 */
class VariableRenamer : public symbolic_expression::DefaultTreeVisitor {

public:
  VariableRenamer();

  virtual ~VariableRenamer();

  void rename_exists(std::shared_ptr<symbolic_expression::Node> node,
                     int phase_num);

  void clear();

  //　書き換え前の変数名 -> 書き換え後の変数名 のmap
  std::map<Variable, Variable> get_renamed_variable_map() const;

  virtual void
  visit(std::shared_ptr<hydla::symbolic_expression::Variable> node);
  virtual void
  visit(std::shared_ptr<hydla::symbolic_expression::Exists> node);

  virtual void
  visit(std::shared_ptr<symbolic_expression::ConstraintDefinition> node);
  virtual void
  visit(std::shared_ptr<symbolic_expression::ProgramDefinition> node);
  virtual void
  visit(std::shared_ptr<symbolic_expression::ConstraintCaller> node);
  virtual void
  visit(std::shared_ptr<symbolic_expression::ProgramCaller> node);
  virtual void visit(std::shared_ptr<symbolic_expression::Constraint> node);

  virtual void visit(std::shared_ptr<symbolic_expression::Ask> node);
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
  virtual void visit(std::shared_ptr<symbolic_expression::Pi> node);
  virtual void visit(std::shared_ptr<symbolic_expression::E> node);
  virtual void visit(std::shared_ptr<symbolic_expression::Number> node);
  virtual void visit(std::shared_ptr<symbolic_expression::Parameter> node);
  virtual void visit(std::shared_ptr<symbolic_expression::SymbolicT> node);
  virtual void visit(std::shared_ptr<symbolic_expression::Infinity> node);
  virtual void visit(std::shared_ptr<symbolic_expression::SVtimer> node);
  virtual void visit(std::shared_ptr<symbolic_expression::Previous> node);

private:
  int phase_num_;
  int diff_cnt_;
  std::map<Variable, Variable> exists_variable_map_;
  std::set<std::string> exists_variable_set_;

  symbolic_expression::node_sptr new_child_;

  template <class C, const symbolic_expression::node_sptr &(C::*getter)() const,
            void (C::*setter)(const symbolic_expression::node_sptr &child)>
  void dispatch(C *n) {
    accept((n->*getter)());
    if (new_child_) {
      (n->*setter)(new_child_);
      new_child_.reset();
    }
  }

  template <class NodeType> void dispatch_child(NodeType &node) {
    dispatch<hydla::symbolic_expression::UnaryNode,
             &hydla::symbolic_expression::UnaryNode::get_child,
             &hydla::symbolic_expression::UnaryNode::set_child>(node.get());
  }

  template <class NodeType> void dispatch_rhs(NodeType &node) {
    dispatch<hydla::symbolic_expression::BinaryNode,
             &hydla::symbolic_expression::BinaryNode::get_rhs,
             &hydla::symbolic_expression::BinaryNode::set_rhs>(node.get());
  }

  template <class NodeType> void dispatch_lhs(NodeType &node) {
    dispatch<hydla::symbolic_expression::BinaryNode,
             &hydla::symbolic_expression::BinaryNode::get_lhs,
             &hydla::symbolic_expression::BinaryNode::set_lhs>(node.get());
  }
};

} // namespace simulator
} // namespace hydla

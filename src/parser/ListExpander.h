#pragma once
#include "DefaultTreeVisitor.h"
#include "DefinitionContainer.h"
#include "Node.h"
#include "ParseError.h"
#include "boost/shared_ptr.hpp"
#include <complex>
#include <map>
#include <stack>

namespace hydla {
namespace parser {

class ListExpander : public symbolic_expression::TreeVisitor {
public:
  typedef DefinitionContainer<symbolic_expression::Definition>::
      definition_map_key_t referenced_definition_t;

  ListExpander(
      DefinitionContainer<symbolic_expression::ConstraintDefinition> &,
      DefinitionContainer<symbolic_expression::ProgramDefinition> &,
      DefinitionContainer<symbolic_expression::ExpressionListDefinition> &,
      DefinitionContainer<symbolic_expression::ProgramListDefinition> &);
  virtual ~ListExpander(){};

  symbolic_expression::node_sptr
  circular_check(referenced_definition_t def_type,
                 std::shared_ptr<symbolic_expression::Definition> definition);

  std::shared_ptr<symbolic_expression::ExpressionList> expand_list(
      std::shared_ptr<symbolic_expression::ConditionalExpressionList>);
  std::shared_ptr<symbolic_expression::ProgramList>
      expand_list(std::shared_ptr<symbolic_expression::ConditionalProgramList>);
  std::shared_ptr<symbolic_expression::VariadicNode>
      expand_list(std::shared_ptr<symbolic_expression::Range>);
  std::shared_ptr<symbolic_expression::VariadicNode>
      expand_list(std::shared_ptr<symbolic_expression::Union>);
  std::shared_ptr<symbolic_expression::VariadicNode>
      expand_list(std::shared_ptr<symbolic_expression::Intersection>);
  symbolic_expression::node_sptr
      expand_list(std::shared_ptr<symbolic_expression::ExpressionListElement>);
  symbolic_expression::node_sptr
      expand_list(std::shared_ptr<symbolic_expression::ProgramListElement>);
  std::shared_ptr<symbolic_expression::Number>
      expand_list(std::shared_ptr<symbolic_expression::SizeOfList>);
  symbolic_expression::node_sptr
      expand_list(std::shared_ptr<symbolic_expression::SumOfList>);
  symbolic_expression::node_sptr
      expand_list(std::shared_ptr<symbolic_expression::MulOfList>);

  virtual void visit(std::shared_ptr<symbolic_expression::Previous>);
  virtual void visit(std::shared_ptr<symbolic_expression::Equal>);
  virtual void
      visit(std::shared_ptr<symbolic_expression::ExpressionListDefinition>);
  virtual void visit(std::shared_ptr<symbolic_expression::Parameter>);
  virtual void visit(std::shared_ptr<symbolic_expression::True>);
  virtual void
      visit(std::shared_ptr<symbolic_expression::ProgramListDefinition>);
  virtual void visit(std::shared_ptr<symbolic_expression::PrintIP>);
  virtual void
      visit(std::shared_ptr<symbolic_expression::ConstraintDefinition>);
  virtual void visit(std::shared_ptr<symbolic_expression::Not>);
  virtual void visit(std::shared_ptr<symbolic_expression::LessEqual>);
  virtual void visit(std::shared_ptr<symbolic_expression::Always>);
  virtual void visit(std::shared_ptr<symbolic_expression::Negative>);
  virtual void visit(std::shared_ptr<symbolic_expression::Abort>);
  virtual void visit(std::shared_ptr<symbolic_expression::Less>);
  virtual void visit(std::shared_ptr<symbolic_expression::Pi>);
  virtual void visit(std::shared_ptr<symbolic_expression::Print>);
  virtual void visit(std::shared_ptr<symbolic_expression::Function>);
  virtual void visit(std::shared_ptr<symbolic_expression::GreaterEqual>);
  virtual void visit(std::shared_ptr<symbolic_expression::Scan>);
  virtual void visit(std::shared_ptr<symbolic_expression::UnsupportedFunction>);
  virtual void visit(std::shared_ptr<symbolic_expression::LogicalOr>);
  virtual void visit(std::shared_ptr<symbolic_expression::Greater>);
  virtual void visit(std::shared_ptr<symbolic_expression::SVtimer>);
  virtual void visit(std::shared_ptr<symbolic_expression::Tell>);
  virtual void visit(std::shared_ptr<symbolic_expression::DifferentVariable>);
  virtual void visit(std::shared_ptr<symbolic_expression::Differential>);
  virtual void visit(std::shared_ptr<symbolic_expression::SymbolicT>);
  virtual void visit(std::shared_ptr<symbolic_expression::ProgramDefinition>);
  virtual void visit(std::shared_ptr<symbolic_expression::Constraint>);
  virtual void visit(std::shared_ptr<symbolic_expression::PrintPP>);
  virtual void visit(std::shared_ptr<symbolic_expression::Ask>);
  virtual void visit(std::shared_ptr<symbolic_expression::Exists>);
  virtual void visit(std::shared_ptr<symbolic_expression::EachElement>);
  virtual void visit(std::shared_ptr<symbolic_expression::Infinity>);
  virtual void visit(std::shared_ptr<symbolic_expression::Positive>);
  virtual void visit(std::shared_ptr<symbolic_expression::Exit>);
  virtual void visit(std::shared_ptr<symbolic_expression::LogicalAnd>);
  virtual void visit(std::shared_ptr<symbolic_expression::UnEqual>);
  virtual void visit(std::shared_ptr<symbolic_expression::E>);
  virtual void visit(std::shared_ptr<symbolic_expression::False>);
  virtual void visit(std::shared_ptr<symbolic_expression::Float>);
  virtual void visit(std::shared_ptr<symbolic_expression::ImaginaryUnit>);

  virtual void visit(std::shared_ptr<symbolic_expression::Weaker>);
  virtual void visit(std::shared_ptr<symbolic_expression::Parallel>);
  virtual void visit(std::shared_ptr<symbolic_expression::Number>);
  virtual void visit(std::shared_ptr<symbolic_expression::ExpressionList>);
  virtual void visit(std::shared_ptr<symbolic_expression::ProgramList>);
  virtual void
      visit(std::shared_ptr<symbolic_expression::ConditionalExpressionList>);
  virtual void
      visit(std::shared_ptr<symbolic_expression::ConditionalProgramList>);
  virtual void visit(std::shared_ptr<symbolic_expression::Union>);
  virtual void visit(std::shared_ptr<symbolic_expression::Intersection>);
  virtual void visit(std::shared_ptr<symbolic_expression::Range>);
  virtual void visit(std::shared_ptr<symbolic_expression::Variable>);
  virtual void visit(std::shared_ptr<symbolic_expression::Plus>);
  virtual void visit(std::shared_ptr<symbolic_expression::Subtract>);
  virtual void visit(std::shared_ptr<symbolic_expression::Times>);
  virtual void visit(std::shared_ptr<symbolic_expression::Divide>);
  virtual void visit(std::shared_ptr<symbolic_expression::Power>);
  virtual void visit(std::shared_ptr<symbolic_expression::ProgramCaller>);
  virtual void visit(std::shared_ptr<symbolic_expression::ConstraintCaller>);
  virtual void
      visit(std::shared_ptr<symbolic_expression::ExpressionListElement>);
  virtual void visit(std::shared_ptr<symbolic_expression::ProgramListElement>);
  virtual void visit(std::shared_ptr<symbolic_expression::ProgramListCaller>);
  virtual void
      visit(std::shared_ptr<symbolic_expression::ExpressionListCaller>);
  virtual void visit(std::shared_ptr<symbolic_expression::SizeOfList>);
  virtual void visit(std::shared_ptr<symbolic_expression::SumOfList>);
  virtual void visit(std::shared_ptr<symbolic_expression::MulOfList>);

  std::set<std::shared_ptr<symbolic_expression::ExpressionListDefinition>>
  get_called_expression_list_definition();
  std::set<std::shared_ptr<symbolic_expression::ProgramListDefinition>>
  get_called_program_list_definition();

private:
  void
  expand_conditional_list(std::shared_ptr<symbolic_expression::VariadicNode>,
                          symbolic_expression::node_sptr,
                          std::shared_ptr<symbolic_expression::VariadicNode>,
                          int);

  bool in_list_element = false;

  std::stack<std::set<referenced_definition_t>> referenced_definition;

  std::set<std::shared_ptr<symbolic_expression::ExpressionListDefinition>>
      called_expression_list_definition;
  std::set<std::shared_ptr<symbolic_expression::ProgramListDefinition>>
      called_program_list_definition;

  std::map<symbolic_expression::node_sptr, symbolic_expression::node_sptr>
      local_variable_map;
  symbolic_expression::node_sptr new_child;
  DefinitionContainer<symbolic_expression::ConstraintDefinition> constraint_def;
  DefinitionContainer<symbolic_expression::ProgramDefinition> program_def;
  DefinitionContainer<symbolic_expression::ExpressionListDefinition>
      expression_list_def;
  DefinitionContainer<symbolic_expression::ProgramListDefinition>
      program_list_def;
};

} // namespace parser
} // namespace hydla

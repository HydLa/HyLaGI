#pragma once
#include "boost/shared_ptr.hpp"
#include "Node.h"
#include "DefaultTreeVisitor.h"
#include "DefinitionContainer.h"
#include "ParseError.h"
#include <map>
#include <stack>
#include <complex>

namespace hydla{
namespace parser{

class ListExpander : public symbolic_expression::TreeVisitor{
public:

  typedef DefinitionContainer<symbolic_expression::Definition>::definition_map_key_t referenced_definition_t;

  ListExpander(
    DefinitionContainer<symbolic_expression::ConstraintDefinition>&,
    DefinitionContainer<symbolic_expression::ProgramDefinition>&,
    DefinitionContainer<symbolic_expression::ExpressionListDefinition>&,
    DefinitionContainer<symbolic_expression::ProgramListDefinition>&
  );
  virtual ~ListExpander(){};

  symbolic_expression::node_sptr circular_check(
      referenced_definition_t def_type,
      boost::shared_ptr<symbolic_expression::Definition> definition);

  boost::shared_ptr<symbolic_expression::ExpressionList> expand_list(boost::shared_ptr<symbolic_expression::ConditionalExpressionList>);
  boost::shared_ptr<symbolic_expression::ProgramList> expand_list(boost::shared_ptr<symbolic_expression::ConditionalProgramList>);
  boost::shared_ptr<symbolic_expression::VariadicNode> expand_list(boost::shared_ptr<symbolic_expression::Range>);
  boost::shared_ptr<symbolic_expression::VariadicNode> expand_list(boost::shared_ptr<symbolic_expression::Union>);
  boost::shared_ptr<symbolic_expression::VariadicNode> expand_list(boost::shared_ptr<symbolic_expression::Intersection>);
  symbolic_expression::node_sptr expand_list(boost::shared_ptr<symbolic_expression::ExpressionListElement>);
  symbolic_expression::node_sptr expand_list(boost::shared_ptr<symbolic_expression::ProgramListElement>);
  boost::shared_ptr<symbolic_expression::Number> expand_list(boost::shared_ptr<symbolic_expression::SizeOfList>);
  symbolic_expression::node_sptr expand_list(boost::shared_ptr<symbolic_expression::SumOfList>);

  virtual void visit(boost::shared_ptr<symbolic_expression::Previous>);
  virtual void visit(boost::shared_ptr<symbolic_expression::Equal>);
  virtual void visit(boost::shared_ptr<symbolic_expression::ExpressionListDefinition>);
  virtual void visit(boost::shared_ptr<symbolic_expression::Parameter>);
  virtual void visit(boost::shared_ptr<symbolic_expression::True>);
  virtual void visit(boost::shared_ptr<symbolic_expression::ProgramListDefinition>);
  virtual void visit(boost::shared_ptr<symbolic_expression::PrintIP>);
  virtual void visit(boost::shared_ptr<symbolic_expression::ConstraintDefinition>);
  virtual void visit(boost::shared_ptr<symbolic_expression::Not>);
  virtual void visit(boost::shared_ptr<symbolic_expression::LessEqual>);
  virtual void visit(boost::shared_ptr<symbolic_expression::Always>);
  virtual void visit(boost::shared_ptr<symbolic_expression::Negative>);
  virtual void visit(boost::shared_ptr<symbolic_expression::Abort>);
  virtual void visit(boost::shared_ptr<symbolic_expression::Less>);
  virtual void visit(boost::shared_ptr<symbolic_expression::Pi>);
  virtual void visit(boost::shared_ptr<symbolic_expression::Print>);
  virtual void visit(boost::shared_ptr<symbolic_expression::Function>);
  virtual void visit(boost::shared_ptr<symbolic_expression::GreaterEqual>);
  virtual void visit(boost::shared_ptr<symbolic_expression::Scan>);
  virtual void visit(boost::shared_ptr<symbolic_expression::UnsupportedFunction>);
  virtual void visit(boost::shared_ptr<symbolic_expression::LogicalOr>);
  virtual void visit(boost::shared_ptr<symbolic_expression::Greater>);
  virtual void visit(boost::shared_ptr<symbolic_expression::SVtimer>);
  virtual void visit(boost::shared_ptr<symbolic_expression::Tell>);
  virtual void visit(boost::shared_ptr<symbolic_expression::DifferentVariable>);
  virtual void visit(boost::shared_ptr<symbolic_expression::Differential>);
  virtual void visit(boost::shared_ptr<symbolic_expression::SymbolicT>);
  virtual void visit(boost::shared_ptr<symbolic_expression::ProgramDefinition>);
  virtual void visit(boost::shared_ptr<symbolic_expression::Constraint>);
  virtual void visit(boost::shared_ptr<symbolic_expression::PrintPP>);
  virtual void visit(boost::shared_ptr<symbolic_expression::Ask>);
  virtual void visit(boost::shared_ptr<symbolic_expression::EachElement>);
  virtual void visit(boost::shared_ptr<symbolic_expression::Infinity>);
  virtual void visit(boost::shared_ptr<symbolic_expression::Positive>);
  virtual void visit(boost::shared_ptr<symbolic_expression::Exit>);
  virtual void visit(boost::shared_ptr<symbolic_expression::LogicalAnd>);
  virtual void visit(boost::shared_ptr<symbolic_expression::UnEqual>);
  virtual void visit(boost::shared_ptr<symbolic_expression::E>);
  virtual void visit(boost::shared_ptr<symbolic_expression::False>);
  virtual void visit(boost::shared_ptr<symbolic_expression::Float>);
  virtual void visit(boost::shared_ptr<symbolic_expression::ImaginaryUnit>);

  virtual void visit(boost::shared_ptr<symbolic_expression::Weaker>);
  virtual void visit(boost::shared_ptr<symbolic_expression::Parallel>);
  virtual void visit(boost::shared_ptr<symbolic_expression::Number>);
  virtual void visit(boost::shared_ptr<symbolic_expression::ExpressionList>);
  virtual void visit(boost::shared_ptr<symbolic_expression::ProgramList>);
  virtual void visit(boost::shared_ptr<symbolic_expression::ConditionalExpressionList>);
  virtual void visit(boost::shared_ptr<symbolic_expression::ConditionalProgramList>);
  virtual void visit(boost::shared_ptr<symbolic_expression::Union>);
  virtual void visit(boost::shared_ptr<symbolic_expression::Intersection>);
  virtual void visit(boost::shared_ptr<symbolic_expression::Range>);
  virtual void visit(boost::shared_ptr<symbolic_expression::Variable>);
  virtual void visit(boost::shared_ptr<symbolic_expression::Plus>);
  virtual void visit(boost::shared_ptr<symbolic_expression::Subtract>);
  virtual void visit(boost::shared_ptr<symbolic_expression::Times>);
  virtual void visit(boost::shared_ptr<symbolic_expression::Divide>);
  virtual void visit(boost::shared_ptr<symbolic_expression::Power>);
  virtual void visit(boost::shared_ptr<symbolic_expression::ProgramCaller>);
  virtual void visit(boost::shared_ptr<symbolic_expression::ConstraintCaller>);
  virtual void visit(boost::shared_ptr<symbolic_expression::ExpressionListElement>);
  virtual void visit(boost::shared_ptr<symbolic_expression::ProgramListElement>);
  virtual void visit(boost::shared_ptr<symbolic_expression::ProgramListCaller>);
  virtual void visit(boost::shared_ptr<symbolic_expression::ExpressionListCaller>);
  virtual void visit(boost::shared_ptr<symbolic_expression::SizeOfList>);
  virtual void visit(boost::shared_ptr<symbolic_expression::SumOfList>);

  std::set<boost::shared_ptr<symbolic_expression::ExpressionListDefinition> >  get_called_expression_list_definition();
  std::set<boost::shared_ptr<symbolic_expression::ProgramListDefinition> >  get_called_program_list_definition();
private:
  void expand_conditional_list(boost::shared_ptr<symbolic_expression::VariadicNode>, symbolic_expression::node_sptr, boost::shared_ptr<symbolic_expression::VariadicNode>, int);

  bool in_list_element = false;

  std::stack<std::set<referenced_definition_t> > referenced_definition;

  std::set<boost::shared_ptr<symbolic_expression::ExpressionListDefinition> >  called_expression_list_definition;
  std::set<boost::shared_ptr<symbolic_expression::ProgramListDefinition> >  called_program_list_definition;

  std::map<symbolic_expression::node_sptr,symbolic_expression::node_sptr> local_variable_map;
  symbolic_expression::node_sptr new_child;
  DefinitionContainer<symbolic_expression::ConstraintDefinition> constraint_def;
  DefinitionContainer<symbolic_expression::ProgramDefinition> program_def;
  DefinitionContainer<symbolic_expression::ExpressionListDefinition> expression_list_def;
  DefinitionContainer<symbolic_expression::ProgramListDefinition> program_list_def;
};

} // parser
} // hydla

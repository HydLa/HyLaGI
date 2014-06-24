#pragma once

#include <string>
#include <istream>

#include "boost/shared_ptr.hpp"
#include "Node.h"
#include "DefinitionContainer.h"
#include "Lexer.h"

namespace hydla{
  namespace parser{

class Parser{
  typedef hydla::symbolic_expression::node_sptr node_sptr;
public:
  Parser(std::string);
  Parser(std::istream&);
  ~Parser();

  node_sptr parse(node_sptr assertion_node, DefinitionContainer<hydla::symbolic_expression::ConstraintDefinition> &constraint_definition, DefinitionContainer<hydla::symbolic_expression::ProgramDefinition> &program_definition);
  
  bool is_COMMA(Token);
  bool is_WEAKER(Token);
  bool is_LOGICAL_OR(Token);
  bool is_LOGICAL_AND(Token);
  bool is_COMPARE(Token);

  node_sptr hydla_program();
  node_sptr statements();
  node_sptr statement();
  node_sptr parse();
  node_sptr program();
  node_sptr program_priority();
  node_sptr program_factor();
  node_sptr def_statement();
  boost::shared_ptr<hydla::symbolic_expression::ConstraintDefinition> constraint_def();
  boost::shared_ptr<hydla::symbolic_expression::ProgramDefinition> program_def();
  boost::shared_ptr<hydla::symbolic_expression::ConstraintDefinition> constraint_callee();
  boost::shared_ptr<hydla::symbolic_expression::ConstraintCaller> constraint_caller();
  boost::shared_ptr<hydla::symbolic_expression::ProgramDefinition> program_callee();
  boost::shared_ptr<hydla::symbolic_expression::ProgramCaller> program_caller();
  node_sptr module();
  boost::shared_ptr<hydla::symbolic_expression::Constraint> constraint();
  node_sptr logical_or();
  node_sptr logical_and();
  node_sptr always();
  node_sptr conditional_constraint();
  node_sptr atomic_constraint();
  node_sptr guard();
  node_sptr guard_term();
  node_sptr logical_not();
  node_sptr comparison();
  node_sptr command();
  node_sptr expression();
  node_sptr arithmetic();
  node_sptr arith_term();
  node_sptr unary();
  node_sptr compare_expression();
  node_sptr power();
  node_sptr prev();
  node_sptr diff();
  node_sptr factor();
  node_sptr system_variable();
  boost::shared_ptr<hydla::symbolic_expression::UnsupportedFunction> unsupported_function();
  boost::shared_ptr<hydla::symbolic_expression::Function> function();
  boost::shared_ptr<hydla::symbolic_expression::Variable> variable();
  node_sptr parameter();
  boost::shared_ptr<hydla::symbolic_expression::Variable> bound_variable();
  std::string constraint_name();
  std::string program_name();
  std::vector<node_sptr> actual_args();
  std::vector<std::string> formal_args();
  node_sptr assertion();
  node_sptr tautology();
  std::string identifier();
  node_sptr number();

private:
  Lexer lexer;

  std::pair<std::pair<int,int>, std::string> error_info;

  node_sptr parsed_program;
  std::vector<boost::shared_ptr<hydla::symbolic_expression::ProgramDefinition> > program_definitions;
  std::vector<boost::shared_ptr<hydla::symbolic_expression::ConstraintDefinition> > constraint_definitions;
  node_sptr assertion_node;
};

}// namespace parser
}// namespace hydla

#pragma once

#include <string>
#include <istream>

#include "boost/shared_ptr.hpp"
#include "Node.h"
#include "DefinitionContainer.h"
#include "Lexer.h"

namespace hydla{
  namespace parser{

  typedef hydla::symbolic_expression::node_sptr node_sptr;
  typedef std::pair<position_t, std::string> error_info_t;

class Parser{
public:

  Parser(std::string);
  Parser(std::istream&);
  ~Parser();

  node_sptr parse(
      node_sptr& assertion_node, 
      DefinitionContainer<hydla::symbolic_expression::ConstraintDefinition> &constraint_definition, 
      DefinitionContainer<hydla::symbolic_expression::ProgramDefinition> &program_definition
  );
  
  node_sptr hydla_program();
  node_sptr statements();
  node_sptr statement();
  node_sptr parse();
  node_sptr program();
  node_sptr program_priority();
  node_sptr program_factor();
  node_sptr parenthesis_program();
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
  std::vector<node_sptr> actual_args();
  std::vector<std::string> formal_args();
  node_sptr assertion();
  node_sptr tautology();
  node_sptr constant();
  std::string variable_name();
  std::string function_name();
  std::string definition_name();
  node_sptr number();
  
  node_sptr expression_list_element();
  node_sptr program_list_element();
  node_sptr conditional_program_list();
  node_sptr program_list();
  node_sptr program_list_term();
  node_sptr program_list_factor();
  boost::shared_ptr<hydla::symbolic_expression::ProgramListCaller> program_list_caller();
  node_sptr conditional_expression_list();
  node_sptr expression_list();
  node_sptr expression_list_term();
  node_sptr expression_list_factor();
  boost::shared_ptr<hydla::symbolic_expression::ExpressionListCaller> expression_list_caller();
  node_sptr list_condition();
  
  void error_occurred(position_t position, std::string error_message){
    error_info.push_back(error_info_t(position,error_message));
  }

  void error_occurred(error_info_t info){ error_info.push_back(info); }

  bool parse_ended();

private:
  Lexer lexer;

  std::vector<error_info_t> error_info;

  node_sptr parsed_program;
  std::vector<boost::shared_ptr<hydla::symbolic_expression::ProgramDefinition> > program_definitions;
  std::vector<boost::shared_ptr<hydla::symbolic_expression::ConstraintDefinition> > constraint_definitions;
  std::vector<boost::shared_ptr<hydla::symbolic_expression::ExpressionListDefinition> > expression_list_definition;
  std::vector<boost::shared_ptr<hydla::symbolic_expression::ProgramListDefinition> > program_list_definition;
  node_sptr assertion_node;
};

}// namespace parser
}// namespace hydla

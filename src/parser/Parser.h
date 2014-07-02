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
  typedef std::vector<std::string> list_t;
  typedef std::pair<std::string, std::pair<std::string, std::string> > condition_t;
  typedef std::pair<position_t, std::string> error_info_t;

class Parser{
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
  boost::shared_ptr<hydla::symbolic_expression::Variable> variable(std::map<std::string, std::string>);
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
  
  void error_occurred(position_t position, std::string error_message){ error_info.push_back(error_info_t(position,error_message)); }
  void error_occurred(error_info_t info){ error_info.push_back(info); }

  bool parse_ended();

  // list functions
  bool list_definition();
  list_t list(std::string,std::map<std::string, std::string>);

  boost::shared_ptr<hydla::symbolic_expression::Number> non_variable_factor(std::map<std::string, std::string>);
  boost::shared_ptr<hydla::symbolic_expression::Number> non_variable_term(std::map<std::string, std::string>);
  boost::shared_ptr<hydla::symbolic_expression::Number> non_variable_expression(std::map<std::string, std::string>);
  
  list_t list_conditions(std::map<std::string, std::string>, std::string);
  std::string replace_string_by_bound_variables(std::map<std::string, std::string>, std::string);
  list_t list_check(std::string list_name);
  void set_list(std::map<std::string, list_t> list){ list_map = list; }

private:
  Lexer lexer;

  std::vector<error_info_t> error_info;

  // map which save appeared variable list
  std::map<std::string, list_t> list_map;

  node_sptr parsed_program;
  std::vector<boost::shared_ptr<hydla::symbolic_expression::ProgramDefinition> > program_definitions;
  std::vector<boost::shared_ptr<hydla::symbolic_expression::ConstraintDefinition> > constraint_definitions;
  node_sptr assertion_node;
};

}// namespace parser
}// namespace hydla

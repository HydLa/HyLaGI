#pragma once

#include <istream>
#include <stack>
#include <string>
#include <vector>

#include "DefinitionContainer.h"
#include "Lexer.h"
#include "Node.h"
#include "boost/shared_ptr.hpp"

namespace hydla {
namespace parser {

typedef hydla::symbolic_expression::node_sptr node_sptr;
typedef std::pair<position_t, std::string> error_info_t;

class Parser {
public:
#define IS_DEFINED_AS(name, size, defs, ret)                                   \
  for (auto D : defs) {                                                        \
    if (name == D->get_name() && size == D->bound_variable_size()) {           \
      ret = D;                                                                 \
      break;                                                                   \
    }                                                                          \
  }

  Parser();
  Parser(std::string);
  Parser(std::vector<std::string>);
  Parser(std::istream &);
  Parser(std::string, std::vector<std::string>);
  Parser(std::string, std::istream &);
  ~Parser();

  void add_file(std::string s, std::istream &i) { lexer.add_file(s, i); }

  node_sptr parse(
      node_sptr &assertion_node,
      DefinitionContainer<hydla::symbolic_expression::ConstraintDefinition> &,
      DefinitionContainer<hydla::symbolic_expression::ProgramDefinition> &,
      DefinitionContainer<hydla::symbolic_expression::ExpressionListDefinition>
          &,
      DefinitionContainer<hydla::symbolic_expression::ProgramListDefinition> &);

  node_sptr hydla_program();
  node_sptr statements();
  node_sptr statement();
  node_sptr parse();
  node_sptr program();
  node_sptr program_priority();
  node_sptr program_factor();
  node_sptr parenthesis_program();
  boost::shared_ptr<hydla::symbolic_expression::ConstraintDefinition>
  constraint_def();
  boost::shared_ptr<hydla::symbolic_expression::ProgramDefinition>
  program_def();
  boost::shared_ptr<hydla::symbolic_expression::ConstraintDefinition>
  constraint_callee();
  boost::shared_ptr<hydla::symbolic_expression::ConstraintCaller>
  constraint_caller();
  boost::shared_ptr<hydla::symbolic_expression::ProgramDefinition>
  program_callee();
  boost::shared_ptr<hydla::symbolic_expression::ProgramCaller> program_caller();
  node_sptr module();
  boost::shared_ptr<hydla::symbolic_expression::Constraint> constraint();
  node_sptr logical_or();
  node_sptr logical_and();
  node_sptr always();
  node_sptr conditional_constraint();
  node_sptr atomic_constraint();
  node_sptr guard();
  node_sptr exists();
  node_sptr guard_term();
  node_sptr logical_not();
  node_sptr comparison();
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
  boost::shared_ptr<hydla::symbolic_expression::UnsupportedFunction>
  unsupported_function();
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
  boost::shared_ptr<hydla::symbolic_expression::ProgramListCaller>
  program_list_caller();
  boost::shared_ptr<hydla::symbolic_expression::ProgramListDefinition>
  program_list_callee();
  node_sptr nameless_list();
  node_sptr conditional_expression_list();
  node_sptr expression_list();
  node_sptr expression_list_term();
  node_sptr expression_list_factor();
  boost::shared_ptr<hydla::symbolic_expression::ExpressionListCaller>
  expression_list_caller();
  boost::shared_ptr<hydla::symbolic_expression::ExpressionListDefinition>
  expression_list_callee();
  node_sptr list_condition();
  node_sptr size_of_list();
  node_sptr sum_of_list();
  node_sptr mul_of_list();

  void error_occurred(position_t position, std::string error_message) {
    error_tmp.push_back(error_info_t(position, error_message));
  }

  void error_occurred(error_info_t info) { error_tmp.push_back(info); }

  std::ostream &dump_error(std::ostream &s) const {
    for (auto e : error_info) {
      s << e.first.first << " " << e.first.second << " " << e.second;
    }
    return s;
  }

  void list_type_check();
  bool parse_ended();
  node_sptr
      is_defined(boost::shared_ptr<hydla::symbolic_expression::Definition>);

private:
  Lexer lexer;

  bool second_parse = false;
  bool in_conditional_program_list_ = false;

  std::stack<
      std::vector<boost::shared_ptr<hydla::symbolic_expression::ProgramCaller>>>
      local_program_caller_;

  std::vector<error_info_t> error_info;
  std::vector<error_info_t> error_tmp;

  node_sptr parsed_program;
  std::vector<boost::shared_ptr<hydla::symbolic_expression::ProgramDefinition>>
      program_definitions;
  std::vector<
      boost::shared_ptr<hydla::symbolic_expression::ConstraintDefinition>>
      constraint_definitions;
  std::vector<
      boost::shared_ptr<hydla::symbolic_expression::ExpressionListDefinition>>
      expression_list_definitions;
  std::vector<
      boost::shared_ptr<hydla::symbolic_expression::ProgramListDefinition>>
      program_list_definitions;
  node_sptr assertion_node;

  std::vector<boost::shared_ptr<hydla::symbolic_expression::ProgramDefinition>>
      tmp_program_definitions;
  std::vector<
      boost::shared_ptr<hydla::symbolic_expression::ConstraintDefinition>>
      tmp_constraint_definitions;
  std::vector<
      boost::shared_ptr<hydla::symbolic_expression::ExpressionListDefinition>>
      tmp_expression_list_definitions;
  std::vector<
      boost::shared_ptr<hydla::symbolic_expression::ProgramListDefinition>>
      tmp_program_list_definitions;
};

} // namespace parser
} // namespace hydla

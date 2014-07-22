#include "arithmetic_expr.h"

hydla::symbolic_expression::node_sptr parse_arithmetic_string(const std::string &str)
{
  using hydla::parser::HydLaAST;
  HydLaAST ast;
  ast.parse_string(str, HydLaAST::ARITHMETIC_EXPRESSION);
  using hydla::parser::NodeTreeGenerator;
  NodeTreeGenerator genarator;
  return genarator.generate(ast.get_tree_iterator());
}

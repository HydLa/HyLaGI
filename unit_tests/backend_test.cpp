/**
 * test for backends
 */

#include "test_common.h"
#ifndef DISABLE_BACKEND_TEST

#include "string"
#include "sstream"

#include "Backend.h"
#include "mathematica/MathematicaLink.h"
#include "NodeTreeGenerator.h"
#include "HydLaAST.h"

using namespace std;
using namespace hydla::backend;
using namespace hydla::simulator;
using namespace hydla::backend::mathematica;
using namespace hydla::parser;
using namespace hydla::parse_tree;


node_sptr parse_arithmetic_string(const string &str)
{
  HydLaAST ast;
  ast.parse_string(str, HydLaAST::ARITHMETIC_EXPRESSION);
  NodeTreeGenerator genarator;
  return genarator.generate(ast.get_tree_iterator());
}


BOOST_AUTO_TEST_CASE(apply_time_test){
  hydla::simulator::Opts opts;
  opts.mathlink = "-linkmode launch -linkname 'math -mathlink'";
  Backend backend(new MathematicaLink(opts));

  node_sptr expr_node = parse_arithmetic_string("2");
  expr_node = node_sptr(new Times(expr_node, node_sptr(new SymbolicT()))); // 2 * t
  value_t time_value = parse_arithmetic_string("3");
  constraints_t constraints(1, expr_node);

  constraints_t applied_constraints;
  backend.call("applyTime2Expr", 2, "cstvlt", "cs", &constraints, &time_value, &applied_constraints); // substitute 3 to t
  BOOST_CHECK(applied_constraints.size() == 1 && get_infix_string(applied_constraints[0]) == "6");
}


#endif // DISABLE GUARD

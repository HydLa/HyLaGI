/**
 *
 * ParseTreeの構造比較関数のテストケース
 *
 */

#include "test_common.h"
#ifndef DISABLE_PARSE_TREE_STRUCT_TEST

#include <boost/test/auto_unit_test.hpp>
#include <boost/regex.hpp>

#include <sstream>

#include "HydLaAST.h"
#include "NodeFactory.h"
#include "ParseTreeGenerator.h"
#include "ParseError.h"

using namespace std;
using namespace hydla;
using namespace hydla::parser;
using namespace hydla::parse_tree;
using namespace hydla::parse_error;

using namespace boost;

bool comp_parse_tree_struct(std::string lhs, std::string rhs)
{
  HydLaAST ast;
  ParseTreeGenerator<DefaultNodeFactory> ptg;

  ast.parse_string(lhs);
  boost::shared_ptr<ParseTree> pt_lhs(ptg.generate(ast.get_tree_iterator()));

  ast.parse_string(rhs);
  boost::shared_ptr<ParseTree> pt_rhs(ptg.generate(ast.get_tree_iterator()));

  return pt_lhs->is_same_tree_struct(*pt_rhs);
}

BOOST_AUTO_TEST_CASE(parse_tree_struct_test)
{
  // 同一構造
  BOOST_CHECK(comp_parse_tree_struct("x=1.", "x=1."));
  BOOST_CHECK(comp_parse_tree_struct("x=1*3-2/4+h-, y'=4 & z=zzz.", "x=1*3-2/4+h-, y'=4 & z=zzz."));
  BOOST_CHECK(comp_parse_tree_struct("A<=>x=1.B<=>y=2.A,B.", "A<=>x=1.B<=>y=2.A,B."));
  BOOST_CHECK(comp_parse_tree_struct("B<=>y=2.A<=>x=1.A,B.", "A<=>x=1.B<=>y=2.A,B."));

  // 終端ノードNumberの具体値の差異
  BOOST_CHECK(!comp_parse_tree_struct("x=1.", "x=2."));

  // 終端ノードVariableの具体値の差異
  BOOST_CHECK(!comp_parse_tree_struct("x=1.", "y=1."));

  // Symmetry
  BOOST_CHECK(!comp_parse_tree_struct("x=1.", "1=x."));
  BOOST_CHECK(!comp_parse_tree_struct("x=1+2.", "x=2+1."));
  BOOST_CHECK(!comp_parse_tree_struct("A<=>x=1.B<=>y=2.A,B.", "A<=>x=1.B<=>y=2.B,A."));
}

#endif // DISABLE_PARSE_TREE_STRUCT_TEST

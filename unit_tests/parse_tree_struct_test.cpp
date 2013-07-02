/**
 *
 * ParseTreeの構造比較関数のテストケース
 *
 */

#include "test_common.h"
#ifndef DISABLE_PARSE_TREE_STRUCT_TEST

#include <sstream>

#include <boost/regex.hpp>

#include "DefaultNodeFactory.h"
#include "ParseTree.h"
#include "ParseError.h"

using namespace std;
using namespace hydla;
using namespace hydla::parser;
using namespace hydla::parse_tree;
using namespace hydla::parse_error;

using namespace boost;

bool comp_same(std::string lhs, std::string rhs, bool exactly)
{
  ParseTree pt_lhs;
  pt_lhs.parse_string<DefaultNodeFactory>(lhs);

  ParseTree pt_rhs;
  pt_rhs.parse_string<DefaultNodeFactory>(rhs);

  return pt_lhs.is_same_struct(pt_rhs, exactly);
}


BOOST_AUTO_TEST_CASE(parse_tree_exactly_same_struct_test)
{

#define TEST_EXACTLY_SAME(L, R) \
  BOOST_CHECK(comp_same(L, R, true));

#define TEST_NOT_EXACTLY_SAME(L, R) \
  BOOST_CHECK(!comp_same(L, R, true));

  // 同一構造
  TEST_EXACTLY_SAME("x=1.", "x=1.");
  TEST_EXACTLY_SAME("x=1*3-2/4+h-, y'=4 & z=zzz.", "x=1*3-2/4+h-, y'=4 & z=zzz.");
  TEST_EXACTLY_SAME("A<=>x=1.B<=>y=2.A,B.", "A<=>x=1.B<=>y=2.A,B.");
  TEST_EXACTLY_SAME("B<=>y=2.A<=>x=1.A,B.", "A<=>x=1.B<=>y=2.A,B.");

  // 終端ノードNumberの具体値の差異
  TEST_NOT_EXACTLY_SAME("x=1.", "x=2.");

  // 終端ノードVariableの具体値の差異
  TEST_NOT_EXACTLY_SAME("x=1.", "y=1.");

  // Symmetry
  TEST_NOT_EXACTLY_SAME("x=1.", "1=x.");
  TEST_NOT_EXACTLY_SAME("x=1+2.", "x=2+1.");
  TEST_NOT_EXACTLY_SAME("x=1+2+3.", "x=2+1+3.");
  TEST_NOT_EXACTLY_SAME("x=1-2-3.", "x=2-1-3.");
  TEST_NOT_EXACTLY_SAME("x=1 => y=2.", "y=2 => x=1.");
  TEST_NOT_EXACTLY_SAME("A<=>x=1.B<=>y=2.A,B.", "A<=>x=1.B<=>y=2.B,A.");
  TEST_NOT_EXACTLY_SAME("A<=>x=1.B<=>y=2.A<<B.", "A<=>x=1.B<=>y=2.B<<A.");

    
  TEST_NOT_EXACTLY_SAME("(x=1 & y=2) & z=3.", "x=1 & (y=2 & z=3).");
}

BOOST_AUTO_TEST_CASE(parse_tree_same_struct_test)
{
#define TEST_SAME(L, R) \
  BOOST_CHECK(comp_same(L, R, false));

#define TEST_NOT_SAME(L, R) \
  BOOST_CHECK(!comp_same(L, R, false));

  // 同一構造
  TEST_SAME("x=1.", "x=1.");
  
  TEST_SAME("x=1*3-2/4+h-, y'=4 & z=zzz.", "x=1*3-2/4+h-, y'=4 & z=zzz.");
  TEST_SAME("A<=>x=1.B<=>y=2.A,B.", "A<=>x=1.B<=>y=2.A,B.");
  TEST_SAME("B<=>y=2.A<=>x=1.A,B.", "A<=>x=1.B<=>y=2.A,B.");

  // 終端ノードNumberの具体値の差異
  TEST_NOT_SAME("x=1.", "x=2.");

  // 終端ノードVariableの具体値の差異
  TEST_NOT_SAME("x=1.", "y=1.");

  // Symmetry
  TEST_SAME("x=1.", "1=x.");
  TEST_SAME("x=1+2.", "x=2+1.");
  TEST_SAME("x=1+2+3.", "x=2+1+3.");
  TEST_NOT_SAME("x=1-2-3.", "x=2-1-3.");
  TEST_NOT_SAME("x=1 => y=2.", "y=2 => x=1.");
  TEST_SAME("A<=>x=1.B<=>y=2.A,B.", "A<=>x=1.B<=>y=2.B,A.");
  TEST_NOT_SAME("A<=>x=1.B<=>y=2.A<<B.", "A<=>x=1.B<=>y=2.B<<A.");


  // 式変形により同一とみなせる
  TEST_SAME("(x=1 & y=2) & z=3.", "x=1 & (y=2 & z=3).");
  TEST_SAME("x=1 & (y=2 & z=3).", "(x=1 & y=2) & z=3.");
  TEST_SAME("((x=1 & y=2) & z=3) & w=4.", "x=1 & (y=2 & (z=3 & w=4)).");
  TEST_SAME("((x=1 & y=2) & z=3) & w=4.", "x=1 & ((y=2 & z=3) & w=4).");
  TEST_SAME("x=1 | (y=2 & z=3) => a=1.", "(y=2 & z=3) | x=1 => a=1.");
  TEST_NOT_SAME("x=1 & x=1 & x=2.", "x=2 & x=1 & x=3.");
  TEST_SAME(
    "(a=1 & c=3) | (a=1 & d=4) | (b=2 & c=3) | (b=2 & d=4) => z=1.", 
    "c=3 & a=1 | (d=4 & 1=a | c=3 & b=2) | d=4 & 2=b => z=1."); 
}

#endif // DISABLE_PARSE_TREE_STRUCT_TEST

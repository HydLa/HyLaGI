/**
 *
 * askのガード条件内の論理積に関するテスト
 *
 */

#include "test_common.h"
#ifndef DISABLE_AST_GUARD_DISJUNCTION_TEST

#include <iostream>

#include "ParseTree.h"

#include "AskDisjunctionFormatter.h"
#include "AskDisjunctionSplitter.h"

using namespace std;
using namespace hydla;
using namespace hydla::parser;
using namespace hydla::parse_tree;
using namespace hydla::simulator;

using namespace boost;

bool comp_formatted_struct(std::string lhs, std::string rhs)
{
  AskDisjunctionFormatter adf;

  ParseTree pt_lhs;
  pt_lhs.parse_string<DefaultNodeFactory>(lhs); 
  adf.format(&pt_lhs);
  
  ParseTree pt_rhs;
  pt_rhs.parse_string<DefaultNodeFactory>(rhs); 

  return pt_lhs.is_same_struct(pt_rhs, false);
}

bool comp_splitted_struct(std::string lhs, std::string rhs)
{
  AskDisjunctionSplitter ads;

  ParseTree pt_lhs;
  pt_lhs.parse_string<DefaultNodeFactory>(lhs); 
  ads.split(&pt_lhs);
  
  ParseTree pt_rhs;
  pt_rhs.parse_string<DefaultNodeFactory>(rhs); 

  return pt_lhs.is_same_struct(pt_rhs, false);
}

BOOST_AUTO_TEST_CASE(ask_guard_disjunction_format_test)
{
  // 変化なし
  BOOST_CHECK(comp_formatted_struct("x=1=>z=1.", "x=1=>z=1."));  
  BOOST_CHECK(comp_formatted_struct("a=1 | b=2 | c=3 => z=1.", "a=1 | b=2 | c=3 => z=1."));
  BOOST_CHECK(comp_formatted_struct("a=1 & b=2 & c=3 => z=1.", "a=1 & b=2 & c=3 => z=1."));
  BOOST_CHECK(comp_formatted_struct("x=1 | y=2 =>z=1.", "x=1 | y=2 =>z=1."));
  BOOST_CHECK(comp_formatted_struct("(a=1 & b=2) | c=3 => z=1.", "(a=1 & b=2) | c=3 => z=1."));
  BOOST_CHECK(comp_formatted_struct("a=1 | (b=2 & c=3) => z=1.", "a=1 | (b=2 & c=3) => z=1."));

  // 展開
  BOOST_CHECK(comp_formatted_struct(
    "(a=1 | b=2) & c=3 => z=1.", 
    "a=1 & c=3 | b=2 & c=3 => z=1."));
  BOOST_CHECK(comp_formatted_struct(
    "c=3 & (a=1 | b=2) => z=1.", 
    "a=1 & c=3 | b=2 & c=3 => z=1."));

  
  BOOST_CHECK(comp_formatted_struct(
    "(a=1 | b=2) & (c=3 | d=4) => z=1.", 
    "(a=1 & c=3) | (a=1 & d=4) | (b=2 & c=3) | (b=2 & d=4) => z=1."));
  BOOST_CHECK(comp_formatted_struct(
    "(a=1 | b=2) & (c=3 | d=4) & (e=5 | f=6) => z=1.", 
    "(a=1 & c=3 & e=5) | (a=1 & c=3 & f=6) |"
    "(a=1 & d=4 & e=5) | (a=1 & d=4 & f=6) |"
    "(b=2 & c=3 & e=5) | (b=2 & c=3 & f=6) |"
    "(b=2 & d=4 & e=5) | (b=2 & d=4 & f=6) => z=1."));
  BOOST_CHECK(comp_formatted_struct(
    "(a=1 | b=2 | c=3) & (d=4 | e=5) => z=1.", 
    "(a=1 & d=4) | (a=1 & e=5) | (b=2 & d=4) | (b=2 & e=5) | (c=3 & d=4) | (c=3 & e=5) => z=1."));
  BOOST_CHECK(comp_formatted_struct(
    "(a=1 | (b=2 | c=3)) & (d=4 | e=5) => z=1.", 
    "(a=1 & d=4) | (a=1 & e=5) | (b=2 & d=4) | (b=2 & e=5) | (c=3 & d=4) | (c=3 & e=5) => z=1."));
}

BOOST_AUTO_TEST_CASE(ask_guard_disjunction_split_test)
{  
  // 変化なし
  BOOST_CHECK(comp_splitted_struct("x=1=>z=1.", "x=1=>z=1."));  
  BOOST_CHECK(comp_splitted_struct("a=1 & b=2 => z=1.", "a=1 & b=2 => z=1."));
  BOOST_CHECK(comp_splitted_struct("a=1 & b=2 & c=3 => z=1.", "a=1 & b=2 & c=3 => z=1."));
    
  // 分割
  BOOST_CHECK(comp_splitted_struct("a=1 | b=2 => z=1.", 
    "(a=1 => z=1) & (b=2 => z=1)."));

  BOOST_CHECK(comp_splitted_struct("(a=1 | b=2) | c=3 => z=1.", 
    "(a=1 => z=1) & (b=2 => z=1) & (c=3 => z=1)."));
  BOOST_CHECK(comp_splitted_struct("a=1 | (b=2 | c=3) => z=1.", 
    "(a=1 => z=1) & (b=2 => z=1) & (c=3 => z=1)."));

  BOOST_CHECK(comp_splitted_struct("(a=1 | (b=2 | (c=3 | d=4))) => z=1.", 
    "(a=1 => z=1) & (b=2 => z=1) & (c=3 => z=1) & (d=4 => z=1)."));
  BOOST_CHECK(comp_splitted_struct("(((a=1 | b=2) | c=3) | d=4) => z=1.", 
    "(a=1 => z=1) & (b=2 => z=1) & (c=3 => z=1) & (d=4 => z=1)."));
  BOOST_CHECK(comp_splitted_struct("a=1 | (b=2 | c=3) | d=4 => z=1.", 
    "(a=1 => z=1) & (b=2 => z=1) & (c=3 => z=1) & (d=4 => z=1)."));
  BOOST_CHECK(comp_splitted_struct("(a=1 | b=2) | (c=3 | d=4) => z=1.", 
    "(a=1 => z=1) & (b=2 => z=1) & (c=3 => z=1) & (d=4 => z=1)."));

}

#endif // DISABLE_AST_GUARD_DISJUNCTION_TEST

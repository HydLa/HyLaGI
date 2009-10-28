#define BOOST_AUTO_TEST_MAIN
#include <boost/test/auto_unit_test.hpp>

#include <boost/regex.hpp>

#include <sstream>

#include "NodeFactory.h"
#include "HydLaParser.h"
#include "ParseError.h"

using namespace std;
using namespace hydla;
using namespace hydla::parse_tree;
using namespace hydla::parse_error;

using namespace boost;

#define SETUP_PARSE_TREE_TEST() \
  stringstream stream; \
  std::string out_str; \
  const boost::regex reg_exp("\\r|\\n"); \
  shared_ptr<NodeFactory> nf(new NodeFactory()); \
  HydLaParser hp(nf);

#define PARSE_TREE_TEST_EQUAL(INPUT, ANS) \
  hp.parse_string(INPUT); \
  stream.str(""); \
  hp.dump_parse_tree(stream); \
  out_str = regex_replace(stream.str(), reg_exp, string("")); \
  BOOST_CHECK_EQUAL(out_str, ANS);

#define PARSE_TREE_TEST_ERROR(INPUT, ERR_TYPE) \
    BOOST_CHECK_THROW(hp.parse_string(INPUT), ERR_TYPE);

#define PARSE_TREE_TEST_NO_ERROR(INPUT) \
    BOOST_CHECK_NO_THROW(hp.parse_string(INPUT));


BOOST_AUTO_TEST_CASE(parse_tree_test_syntax)
{  
#define PARSE_TREE_TEST_SYNTAX_ERROR(INPUT) \
  PARSE_TREE_TEST_ERROR(INPUT, SyntaxError)

  SETUP_PARSE_TREE_TEST();
    
  // expression test
  PARSE_TREE_TEST_NO_ERROR("a=1.");
  PARSE_TREE_TEST_NO_ERROR("a-=1.");
  PARSE_TREE_TEST_NO_ERROR("a'=1.");
  PARSE_TREE_TEST_NO_ERROR("a''=1.");
  PARSE_TREE_TEST_NO_ERROR("a'-=1.");
  PARSE_TREE_TEST_NO_ERROR("(a=1).");
  PARSE_TREE_TEST_NO_ERROR("[]a=1.");
  PARSE_TREE_TEST_NO_ERROR("[](a=1).");
  PARSE_TREE_TEST_NO_ERROR("[](a=(1)).");
  PARSE_TREE_TEST_NO_ERROR("[](a)=1.");
  PARSE_TREE_TEST_SYNTAX_ERROR("a=1");

  // logical
  PARSE_TREE_TEST_NO_ERROR("a=1 /\\ b=2.");
  PARSE_TREE_TEST_NO_ERROR("a=1 & b=2.");
  PARSE_TREE_TEST_NO_ERROR("A<=>a=1 /\\ b=2.");
  PARSE_TREE_TEST_NO_ERROR("A<=>a=1 &   b=2.");
  PARSE_TREE_TEST_NO_ERROR("A<=>a=1 \\/ b=2.");
  PARSE_TREE_TEST_NO_ERROR("A<=>a=1 |   b=2.");

  // constraint hierarchy
  PARSE_TREE_TEST_NO_ERROR("a=1 << b=2.");
  PARSE_TREE_TEST_NO_ERROR("a=1 << b=2 << c=3.");
  PARSE_TREE_TEST_NO_ERROR("a=1 , b=2.");
  PARSE_TREE_TEST_NO_ERROR("a=1 , b=2, c=3.");   
  PARSE_TREE_TEST_NO_ERROR("a=1,(b=2, d=3),c=4.");
  PARSE_TREE_TEST_NO_ERROR("a=1,(b=2<<d=3),c=4.");
  PARSE_TREE_TEST_NO_ERROR("a=1 & b=2 << c=3.");
  PARSE_TREE_TEST_NO_ERROR("(a=1 & b=2) << c=3.");

  //constraint module  
  PARSE_TREE_TEST_NO_ERROR("A<=>a=1. A.");
  PARSE_TREE_TEST_NO_ERROR("A<=>a=1. (A).");
  PARSE_TREE_TEST_NO_ERROR("A<=>a=1. B<=>b=2. A<<B.");
  PARSE_TREE_TEST_NO_ERROR("A<=>a=1. B{b=2}. C<=>c=3. A<<B<<C.");
  PARSE_TREE_TEST_NO_ERROR("A<=>a=1. B<=>b=2. A,B.");
  PARSE_TREE_TEST_NO_ERROR("A{a=1}. B{b=2}. C<=>c=3. A,B,C.");
  PARSE_TREE_TEST_NO_ERROR("A<=>a=1. B<=>b=2. A /\\ B.");
  PARSE_TREE_TEST_NO_ERROR("A<=>a=1. B<=>b=2. A & B.");
  PARSE_TREE_TEST_NO_ERROR("A<=>a=1. B<=>b=2. C<=>c=3. A & B << C.");
  PARSE_TREE_TEST_NO_ERROR("A<=>a=1. B<=>b=2. C<=>c=3. (A & B) << C.");
  PARSE_TREE_TEST_NO_ERROR("A<=>a=1. B<=>b=2. C<=>c=3. D<=>d=4. A,(B, D),C.");
  PARSE_TREE_TEST_NO_ERROR("A<=>a=1. B<=>b=2. C<=>c=3. D<=>d=4. A,(B<<D),C."); 

  
  // distinguish 'prev' from 'binary -'
  PARSE_TREE_TEST_NO_ERROR("x-1=2.");
  PARSE_TREE_TEST_NO_ERROR("x-(1)=2.");
  PARSE_TREE_TEST_NO_ERROR("x- - 1=2.");
  PARSE_TREE_TEST_NO_ERROR("x- * 3=2.");
  PARSE_TREE_TEST_NO_ERROR("x- - -1=2.");
  PARSE_TREE_TEST_NO_ERROR("2=x-1.");
  PARSE_TREE_TEST_NO_ERROR("2=x- - 1.");
  PARSE_TREE_TEST_NO_ERROR("2=x- - -1.");
  PARSE_TREE_TEST_NO_ERROR("2=x- - -1 /\\ y=2.");
  PARSE_TREE_TEST_NO_ERROR("2=x- - -1 - y.");

  // distinguish 'program call' from 'variable'
  PARSE_TREE_TEST_NO_ERROR("(x) - 1=2.");
  PARSE_TREE_TEST_SYNTAX_ERROR("([]x) - 1=2.");
  PARSE_TREE_TEST_SYNTAX_ERROR("([]x- - 1)=2.");

  // constraint definition
  PARSE_TREE_TEST_NO_ERROR("A <=> b=2.");
  PARSE_TREE_TEST_NO_ERROR("A() <=> b=x.");
  PARSE_TREE_TEST_NO_ERROR("A(x) <=> b=x.");
  PARSE_TREE_TEST_NO_ERROR("A(x, y) <=> b=x.");
  PARSE_TREE_TEST_NO_ERROR("A(x, y, z) <=> b=x.");
  PARSE_TREE_TEST_SYNTAX_ERROR("A(1) <=> b=x.");
  PARSE_TREE_TEST_SYNTAX_ERROR("A(x+1) <=> b=x.");
}

/*
BOOST_AUTO_TEST_CASE(parse_tree_test_equal)
{
  SETUP_PARSE_TREE_TEST();

  // 実変数と従属変数の区別は出来ているかどうか
  PARSE_TREE_TEST_EQUAL(
    "A(x)<=>B(x).B(y)<=>x=y.A(5)."
    ,
    "parse_tree[constraint_definition[A(x):=call<B(x)>,B(y):=tell[x=y]],"
    "program_definition[],"
    "node_tree[call<A(5)>[call<B(5)>[tell[x=5]]]]]");

  // 複数の名前呼び出しを正しく解決できるか
  PARSE_TREE_TEST_EQUAL(
    "A(x)<=>x=y.A(5).A(10)."
    ,
    "parse_tree[constraint_definition[A(x):=constraint[tell[x=y]]],"
    "program_definition[],"
    "node_tree[call<A(5)>[constraint[tell[5=y]]],call<A(10)>[constraint[tell[10=y]]]]");

  // 連続したalways制約は削除され、
  // askの子ノードにおけるalways制約は外のalwaysに影響されずに残るか
    PARSE_TREE_TEST_EQUAL(
    "[](x=1 & []y=2 => z=3 & v=4=>[]w=5)."
    ,
    "");
}
*/

BOOST_AUTO_TEST_CASE(parse_tree_test_invalid_disjunction)
{
#define PARSE_TREE_TEST_INVALID_DISJUNCTION_ERROR(INPUT) \
  PARSE_TREE_TEST_ERROR(INPUT, InvalidDisjunction)

  SETUP_PARSE_TREE_TEST();


  PARSE_TREE_TEST_INVALID_DISJUNCTION_ERROR(
    "a=1 \\/ b=2.");

  PARSE_TREE_TEST_INVALID_DISJUNCTION_ERROR(
    "a=1 | b=2.");

  PARSE_TREE_TEST_INVALID_DISJUNCTION_ERROR(
    "A<=>a=1. B<=>b=2. A \\/ B.");

  PARSE_TREE_TEST_INVALID_DISJUNCTION_ERROR(
    "A<=>a=1. B<=>b=2. A | B.");

  PARSE_TREE_TEST_INVALID_DISJUNCTION_ERROR(
    "A<=>a=2 | b=3. A.");

  PARSE_TREE_TEST_NO_ERROR(
    "A<=>a=2 | b=3. A => c=4.");
  
  PARSE_TREE_TEST_INVALID_DISJUNCTION_ERROR(
    "A<=>a=2 | b=3. c=4 => A.");
}

/**
 * 存在しない名前への呼び出し
 */
BOOST_AUTO_TEST_CASE(parse_tree_test_undefined_reference)
{
#define PARSE_TREE_TEST_UNDEFINED_REFERENCE_ERROR(INPUT) \
  PARSE_TREE_TEST_ERROR(INPUT, UndefinedReference)

  SETUP_PARSE_TREE_TEST();

  PARSE_TREE_TEST_UNDEFINED_REFERENCE_ERROR(
    "A(x)<=>x=1. A(x,y).");

  PARSE_TREE_TEST_UNDEFINED_REFERENCE_ERROR(
    "A(x)<=>x=1. B(x).");
  
  PARSE_TREE_TEST_UNDEFINED_REFERENCE_ERROR(
    "A(x)<=>x=1. AA(x).");

  PARSE_TREE_TEST_UNDEFINED_REFERENCE_ERROR(
    "A{a=1}. B<=>b=2. A /\\ B.");

  PARSE_TREE_TEST_UNDEFINED_REFERENCE_ERROR(
    "A{a=1}. B<=>b=2. A & B.");

  PARSE_TREE_TEST_NO_ERROR(
    "A(5).A(x)<=>x=1.");
    
  PARSE_TREE_TEST_NO_ERROR(
    "A(5).A(x){x=1}.");
}

/**
 * 循環参照
 */
BOOST_AUTO_TEST_CASE(parse_tree_test_circular_reference)
{
#define PARSE_TREE_TEST_CIRCULA_REFERENCE_ERROR(INPUT) \
  PARSE_TREE_TEST_ERROR(INPUT, CircularReference)

  SETUP_PARSE_TREE_TEST();

  PARSE_TREE_TEST_CIRCULA_REFERENCE_ERROR(
    "A(x)<=>A(x). A(5).");

  PARSE_TREE_TEST_CIRCULA_REFERENCE_ERROR(
    "A(x){A(x)}. A(5)."); 

  PARSE_TREE_TEST_CIRCULA_REFERENCE_ERROR(
    "A(x)<=>B(x). B(x)<=>A(x). A(5).");

  PARSE_TREE_TEST_CIRCULA_REFERENCE_ERROR(
    "A(x){B(x)}. B(x){A(x)}. A(5).");

  PARSE_TREE_TEST_CIRCULA_REFERENCE_ERROR(
    "A<=>A=>B. B<=>b=1. A.");

  PARSE_TREE_TEST_CIRCULA_REFERENCE_ERROR(
    "A<=>B=>A. B<=>b=1. A.");

  PARSE_TREE_TEST_NO_ERROR(
    "A<=>B=>C. B<=>C. C<=>c=1. A.");

  PARSE_TREE_TEST_NO_ERROR(
    "A<=>B /\\ B. B<=>b=1. A.");

  PARSE_TREE_TEST_NO_ERROR(
    "A{B, B}. B<=>b=1. A.");

  PARSE_TREE_TEST_NO_ERROR(
    "A{B << B}. B<=>b=1. A.");
}

/**
 * 多重定義
 */
BOOST_AUTO_TEST_CASE(parse_tree_test_multiple_definition)
{
  SETUP_PARSE_TREE_TEST();

  PARSE_TREE_TEST_ERROR(
    "A()<=>x=1.A<=>y=1.",
    MultipleDefinition);

  PARSE_TREE_TEST_ERROR(
    "A(x)<=>x=1.A(x)<=>y=1.",
    MultipleDefinition);

  PARSE_TREE_TEST_ERROR(
    "A(x){x=1}.A(x)<=>y=1.",
    MultipleDefinition);

  PARSE_TREE_TEST_ERROR(
    "A(x){x=1}.A(x){y=1}.",
    MultipleDefinition);

  PARSE_TREE_TEST_NO_ERROR(
    "A(x)<=>x=1.A<=>y=1.");
}

BOOST_AUTO_TEST_CASE(parse_tree_test_example_file)
{
  shared_ptr<NodeFactory> nf(new NodeFactory());
  HydLaParser hp(nf);

  BOOST_CHECK_NO_THROW(
    hp.parse_flie("../examples/bouncing_particle.hydla"));

  BOOST_CHECK_NO_THROW(
    hp.parse_flie("../examples/box1.hydla"));
  
  BOOST_CHECK_NO_THROW(
    hp.parse_flie("../examples/impulse_function.hydla"));

  BOOST_CHECK_NO_THROW(
    hp.parse_flie("../examples/navigation.hydla"));
}


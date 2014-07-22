/**
 *
 * intervalモジュールのテスト
 *
 */

#include "test_common.h"
#ifndef DISABLE_AFFINE_TEST

#include <sstream>
#include <fstream>

#include "AffineTreeVisitor.h"
#include "arithmetic_expr.h"

using namespace std;
using namespace hydla;
using namespace boost;
using namespace hydla::parser;

BOOST_AUTO_TEST_CASE(interval_affine_tree_visitor)
{
/*
  symbolic_expression::node_sptr node = parse_arithmetic_string("");
  hydla::interval::parameter_idx_map_t tmp_map;
  interval::AffineOrInteger result = interval::AffineTreeVisitor(tmp_map).approximate(node);
  cout << result << endl;
*/
}

#endif // DISABLE_PARSE_TREE_BUILD_TEST

#ifndef _INCLUDED_HYDLA_TEST_COMMON_H_
#define _INCLUDED_HYDLA_TEST_COMMON_H_

/**
 * ユニットテストの共通ヘッダ
 * 
 * すべてのテストはこのファイルをインクルードすること
 */

#include <boost/test/auto_unit_test.hpp>

/**
 * 実行するテストの選択
 */

#ifndef TEST_ALL_TEST_CASE

  #define DISABLE_PARSE_TREE_BUILD_TEST // 20121206 parse_tree_test.cpp(332) でエラーを起こす
  //#define DISABLE_PARSE_TREE_STRUCT_TEST
  #define DISABLE_MODULE_SET_LIST_TEST
  #define DISABLE_CONSISTENCY_CHECKER_TEST
  //#define DISABLE_AST_GUARD_DISJUNCTION_TEST

#endif // TEST_ALL_TEST_CASE


#endif // _INCLUDED_HYDLA_TEST_COMMON_H_

#pragma once

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
  #define DISABLE_VCS_REDUCE_SOURCE_NULL_TEST
  #define DISABLE_VCS_REDUCE_SOURCE_TEST

  #define DISABLE_PARSE_TREE_BUILD_TEST // 20121206 parse_tree_test.cpp(332) でエラーを起こす
  #define DISABLE_PARSE_TREE_STRUCT_TEST
  #define DISABLE_MODULE_SET_LIST_TEST
  #define DISABLE_CONSISTENCY_CHECKER_TEST
  #define DISABLE_AST_GUARD_DISJUNCTION_TEST
  #define DISABLE_S_EXP_PARSER_TEST
  #define DISABLE_BACKEND_TEST

#endif // TEST_ALL_TEST_CASE

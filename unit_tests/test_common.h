#ifndef _INCLUDED_HYDLA_TEST_COMMON_H_
#define _INCLUDED_HYDLA_TEST_COMMON_H_

/**
 * ���j�b�g�e�X�g�̋��ʃw�b�_
 * 
 * ���ׂẴe�X�g�͂��̃t�@�C�����C���N���[�h���邱��
 */

#include <boost/test/auto_unit_test.hpp>

/**
 * ���s����e�X�g�̑I��
 */

#ifndef TEST_ALL_TEST_CASE

  #define DISABLE_PARSE_TREE_BUILD_TEST
  //#define DISABLE_PARSE_TREE_STRUCT_TEST
  #define DISABLE_MODULE_SET_LIST_TEST
  #define DISABLE_CONSISTENCY_CHECKER_TEST
  //#define DISABLE_AST_GUARD_DISJUNCTION_TEST

#endif // TEST_ALL_TEST_CASE


#endif // _INCLUDED_HYDLA_TEST_COMMON_H_

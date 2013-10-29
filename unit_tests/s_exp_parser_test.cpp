#include "test_common.h"
#ifndef DISABLE_S_EXP_PARSER_TEST

#include "string"
#include "sstream"

#include "../virtual_constraint_solver/reduce/sexp/SExpAST.h"
#include "../virtual_constraint_solver/reduce/sexp/SExpExpressionConverter.h"

using namespace std;
using namespace hydla::vcs::reduce;

void check(std::string arg){
  SExpAST sp(arg);
}

/**
 * SExpParserのテスト
 */
BOOST_AUTO_TEST_CASE(convert_s_exp_to_symbolic_value_test){
  check("(list (list (df usrvary t 2) 0 -10) (list (df usrvary t) 0 0) (list usrvary 0 10 ))");
  check("(quotient (plus (times 648506250 (sqrt 2) t) (minus (times 48828125 (expt t 2))) (minus 4299827162)) 9765625)");
}
  
#endif // DISABLE_S_EXP_PARSER_TEST

/**
 * vcs_reduce_source_test.redのテスト
 */

#include "test_common.h"
#ifndef DISABLE_VCS_REDUCE_SOURCE_TEST

#include "string"
#include "sstream"

#include "../virtual_constraint_solver/reduce/sexp/SExpAST.h"
#include "../virtual_constraint_solver/reduce/REDUCELink.h"
#include "../virtual_constraint_solver/reduce/REDUCELinkFactory.h"
#include "../virtual_constraint_solver/reduce/vcs_reduce_source.h"

using namespace std;
using namespace hydla::parser;
using namespace hydla::vcs::reduce;

/**
 * @param query REDUCEに計算させるS式
 * @param ans S式の解
 * @return queryをREDUCEで評価した結果がansと一致するか
 */
bool check(string query, string ans, bool isDebug = false){
  REDUCELinkFactory rlf;
  REDUCELink* rl = rlf.createInstance();
  rl->send_string(vcs_reduce_source());
  rl->skip_until_redeval();

  rl->send_string(query);
  if(isDebug){
    rl->read_until_redeval();
  }else{
    rl->skip_until_redeval();
  }

  string s_expr = rl->get_s_expr();
  if(isDebug){
    cout << "ret s_expr: " <<  s_expr << endl;
  }
  delete rl;

  return s_expr == ans;
}

/**
 * exDSolve()
 */
BOOST_AUTO_TEST_CASE(exDSolve_test){
  string query = 
    "depend {ht,v}, t$"
    "expr_:={df(ht,t) = v, df(v,t) = -10 }$"
    "init_:={inithtlhs = 10, initvlhs = 0 }$"
    "vars_:={ht,v,df(ht,t),df(v,t)}$"
    "symbolic redeval '(exDSolve expr_ init_ vars_);";

  BOOST_CHECK(check(query, "(list (equal ht (plus (minus (times 5 (expt t 2))) 10)) (equal v (minus (times 10 t))))"));
}

/**
 * checkConsistencyInterval()
 */
BOOST_AUTO_TEST_CASE(checkConsistencyInterval_test){
  string query = 
    "depend {ht,v}, t$"
    "initVariables__:={inithtlhs,initvlhs}$"
    "cons_:= csVariables__:= {}$"
    "tmpCons_:={df(ht,t) = v, df(v,t) = -10 }$"
    "rconts_:={inithtlhs = 10, initvlhs = 0 }$"
    "pCons_:={}$"
    "vars_:={ht,v,df(ht,t),df(v,t)}$"

    "symbolic redeval '(checkConsistencyInterval cons_ tmpCons_ rconts_ pCons_ vars_);";

  BOOST_CHECK(check(query, "(list 2)"));
}


/**
 * RCS_recovery checkConsistencyInterval() 1回目 のexDSolve
 */
BOOST_AUTO_TEST_CASE(rcsRecovery_exDSolve_test){
  string query = 
    "depend {usrvary}, t$"
    "expr_:= {df(usrvary,t,2) = -10}$"
    "init_:= {initusrvary_1lhs = 0,initusrvarylhs = 10}$"
    "vars_:= {usrvary}$"

    "symbolic redeval '(exDSolve expr_ init_ vars_);";

  BOOST_CHECK(check(query, "(list (equal usrvary (plus (minus (times 5 (expt t 2))) 10)))"));
}

/**
 * RCS_recovery checkConsistencyInterval() 1回目 removeInitCons
 */
BOOST_AUTO_TEST_CASE(removeInitCons_test){
  string query = 
    "depend usrvary,t$"
    "initVariables__:={initusrvary_1lhs,initusrvarylhs}$"
    "rconts_:={initusrvary_1lhs = 0,initusrvarylhs = 10}$"
    "symbolic redeval '(removeInitCons rconts_);";

  BOOST_CHECK(check(query, "(list)"));
}

/**
 * Hyrose_v0.6.3 checkConsistencyInterval() 1回目
 */
BOOST_AUTO_TEST_CASE(old_myCheckConsistencyInterval_test){
  string query =
    "depend usrvary,t$"
    "constraintStore__:= {df(usrvary,t,2) = -10, prev(df(usrvary,t,2)) = -10, prev(df(usrvary,t)) = 0, prev(usrvarg) = 10, prev(usrvary) = 10, usrvarg = 10}$"
    "csVariables__:= {df(usrvary,t,2), prev(df(usrvary,t,2)), prev(df(usrvary,t)), prev(usrvarg), prev(usrvary), usrvarg}$"

    "tmpConstraint__:= guard__:= initConstraint__:= initTmpConstraint__:={initusrvary_1lhs = prev(df(usrvary,t)),initusrvarylhs = prev(usrvary)}$"

    "tmpVariables__:={df(usrvary,t),initusrvary_1lhs,initusrvarylhs,usrvary}$"
    "guardVars__:= parameterStore__:= {}$"
    "initVariables__:={initusrvary_1lhs,initusrvarylhs}$"

    "symbolic redeval '(myCheckConsistencyInterval);";

  BOOST_CHECK(check(query, "(list true false)"));
}

/**
 * RCS_recovery checkConsistencyInterval() 1回目
 */
BOOST_AUTO_TEST_CASE(rcsRecovery_myCheckConsistencyInterval_test){
  string query = 
    "depend usrvary,t$"
    "constraintStore__:= {df(usrvary,t,2) = -10, prev(df(usrvary,t,2)) = -10, prev(df(usrvary,t)) = 0, prev(usrvarg) = 10, prev(usrvary) = 10}$"
    "csVariables__:= {df(usrvary,t,2), df(usrvary,t), prev(df(usrvary,t,2)), prev(df(usrvary,t)), prev(usrvarg), prev(usrvary), usrvary}$"
    "tmpConstraint__:= guard__:= initConstraint__:= initTmpConstraint__:= {initusrvary_1lhs = prev(df(usrvary,t)), initusrvarylhs = prev(usrvary)}$"
    "tmpVariables__:= guardVars__:= {df(usrvary,t,2), prev(df(usrvary,t,2)), prev(df(usrvary,t)), prev(usrvarg), prev(usrvary), usrvary}$"
    "parameterStore__:= {}$"
    "initVariables__:={initusrvary_1lhs,initusrvarylhs}$"

    "symbolic redeval '(myCheckConsistencyInterval);";

  BOOST_CHECK(check(query, "(list true false)"));
}

/**
 * RCS_recovery IP時のcreate_maps()
 */
BOOST_AUTO_TEST_CASE(convertCSToVMInterval_test){
  string query = 
    "depend usrvary,t$"
    "constraintStore__:= {df(usrvary,t,2) = -10, prev(df(usrvary,t,2)) = -10, prev(df(usrvary,t)) = 0, prev(usrvarg) = 10, prev(usrvary) = 10}$"
    "initConstraint__:= {initusrvary_1lhs = prev(df(usrvary,t)),initusrvarylhs = prev(usrvary)}$"
    "parameterStore__:= {}$"
    "csVariables__:= {df(usrvary,t,2), df(usrvary,t), prev(df(usrvary,t,2)), prev(df(usrvary,t)), prev(usrvarg), prev(usrvary), usrvary}$"
    "initVariables__:={initusrvary_1lhs,initusrvarylhs}$"

    "symbolic redeval '(convertCSToVMInterval);";

  BOOST_CHECK(check(query, "(list (list (df usrvary t 2) 0 -10) (list (df usrvary t) 0 (minus (times 10 t))) (list usrvary 0 (plus (minus (times 5 (expt t 2))) 10)))"));
}

/**
 * calculate_next_PP_time()
 */
BOOST_AUTO_TEST_CASE(calculateNextPointPhaseTime_test){
  string query = 
    "depend usrvary,t$"
    "constraintStore__:= {usrvarg = 10,usrvary = 5*(-t**2+2)}$"
    "initConstraint__:= {}$"
    "parameterStore__:= {}$"
    "csVariables__:= {df(usrvary,t,2), df(usrvary,t), usrvarg, usrvary}$"

    // calcNextPointPhaseTime内部どこかで使われる
    "psParameters__:= {}$"

    "maxTime_:= 1$"
    "discCause_:= {usrvary = 0}$"
    "symbolic redeval '(calculateNextPointPhaseTime maxTime_ discCause_);";

  BOOST_CHECK(check(query, "(list (list 1 (list (list)) 1))"));
}

/**
 * addConstraint()
 */
BOOST_AUTO_TEST_CASE(addConstraint_test){
  const string query1 = 
    "depend usrvary,t$"
    "tmpVariables__:=tmpConstraint__:={}$"
    "prevConstraint__:= {prev(df(usrvary,t,2)) = -10,prev(df(usrvary,t)) =  - 10*sqrt(2),prev(usrvarg) = 10,prev(usrvary) = 0};"
    "cons_:= {prev(usrvary) <> 0};"
    "vars_:= {prev(usrvary)};"

    "symbolic redeval '(addConstraint cons_ vars_);";

  BOOST_CHECK(check(query1, "(list (neq 0 0))"));
  
  const string query2 = 
    "depend usrvary,t$"
    "lhs:= {prev(df(usrvary,t))=1, prev(usrvary)=0}$"
    "rhs:= prev(usrvary) neq 0$"
    "symbolic redeval '(myExSub lhs rhs);";
  BOOST_CHECK(check(query2, "(neq 0 0)"));

  const string query3 = 
    "depend usrvary,t$"
    "lhs:= {prev(df(usrvary,t))=1, prev(usrvary)=0}$"
    "rhs:= {prev(usrvary) neq 0}$"
    "symbolic redeval '(myExSub lhs rhs);";
  BOOST_CHECK(check(query3, "(list (neq 0 0))"));

}

BOOST_AUTO_TEST_CASE(calculateNextPointPhaseTimeMain_test){
  const string query = 
    "depend usrvary,t$"
    "maxTime_ := 5$"
    "discCause_ := {usrvary = 0}$"
    "cons_ := {usrvary = parameter_y_0_1 - 5*t**2}$"
    "initCons_ := {initusrvary_1lhs = 0,initusrvarylhs = parameter_y_0_1}$"
    "pCons_ := {parameter_y_0_1 - 9 > 0,parameter_y_0_1 - 11 < 0}$"
    "csVariables__ := vars_ := {df(usrvary,t,2),df(usrvary,t),usrvary}$"
    "psParameters__ := {parameter_y_0_1}$"

    "symbolic redeval '(calculateNextPointPhaseTimeMain maxTime_ discCause_ cons_ initCons_ pCons_ vars_);";

  BOOST_CHECK(check(query, "(list (list (quotient (times (sqrt parameter_y_0_1) (sqrt 5)) 5) (list (list (list parameter_y_0_1 2 9) (list parameter_y_0_1 1 11))) 0))"));
}
   

#endif // DISABLE_VCS_REDUCE_SOURCE_TEST

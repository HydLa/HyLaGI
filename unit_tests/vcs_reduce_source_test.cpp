/**
 * vcs_reduce_source.redのテスト
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
 * RCS_recovery checkConsistencyInterval() 1回目 removeInitCons
 */
BOOST_AUTO_TEST_CASE(removeInitCons_test){
  string query = 
    "depend usrvary,t$"
    // removeinitconsのため
    "initVariables__:={initusrvary_1lhs,initusrvarylhs}$"

    "rconts_:={initusrvary_1lhs = 0,initusrvarylhs = 10}$"
    "symbolic redeval '(removeInitCons rconts_);";

  BOOST_CHECK(check(query, "(list)"));
}

BOOST_AUTO_TEST_CASE(exDSolve_test){
  string query = 
    "depend {ht,v}, t$"
    "cons_:={df(ht,t) = v, df(v,t) = -10 }$"
    "guardCons_:={}$"
    "initCons_:={inithtlhs = 10, initvlhs = 0 }$"
    "vars_:={ht,v,df(ht,t),df(v,t)}$"
    "symbolic redeval '(exDSolve cons_ guardCons_ initCons_ vars_);";

  BOOST_CHECK(check(query, "(list (list) (list (equal ht (plus (minus (times 5 (expt t 2))) 10)) (equal v (minus (times 10 t)))))"));

  const string query_rcs_recovery = 
    "depend {usrvary}, t$"
    "cons_:= {df(usrvary,t,2) = -10}$"
    "guardCons_:={}$"
    "init_:= {initusrvary_1lhs = 0,initusrvarylhs = 10}$"
    "vars_:= {usrvary}$"

    "symbolic redeval '(exDSolve cons_ guardCons_ init_ vars_);";

  BOOST_CHECK(check(query_rcs_recovery, "(list (list) (list (equal usrvary (plus (minus (times 5 (expt t 2))) 10))))"));
}

/**
 * Hyrose_v0.6.3 checkConsistencyInterval() 1回目
 */
BOOST_AUTO_TEST_CASE(checkConsistencyInterval_test){
  string query =
    "depend usrvary,t$"
    // removeinitconsのため
    "initVariables__:={initusrvary_1lhs,initusrvarylhs}$"

    "constraint__:= {df(usrvary,t,2) = -10, prev(df(usrvary,t,2)) = -10, prev(df(usrvary,t)) = 0, prev(usrvarg) = 10, prev(usrvary) = 10, usrvarg = 10}$"
    "variables__:= {df(usrvary,t,2), prev(df(usrvary,t,2)), prev(df(usrvary,t)), prev(usrvarg), prev(usrvary), usrvarg}$"
    "initConstraint__:= initTmpConstraint__:={initusrvary_1lhs = prev(df(usrvary,t)),initusrvarylhs = prev(usrvary)}$"
    "tmpVariables__:={df(usrvary,t),initusrvary_1lhs,initusrvarylhs,usrvary}$"
    "guardVars__:= pConstraint__:= {}$"

    "symbolic redeval '(checkConsistencyInterval);";

  BOOST_CHECK(check(query, "(list true false)"));

  string query_rcs_recovery_cci1 = 
    "depend usrvary,t$"
    // removeinitconsのため
    "initVariables__:={initusrvary_1lhs,initusrvarylhs}$"

    "constraint__:= {df(usrvary,t,2) = -10, prev(df(usrvary,t,2)) = -10, prev(df(usrvary,t)) = 0, prev(usrvarg) = 10, prev(usrvary) = 10}$"
    "variables__:= {df(usrvary,t,2), df(usrvary,t), prev(df(usrvary,t,2)), prev(df(usrvary,t)), prev(usrvarg), prev(usrvary), usrvary}$"
    "initConstraint__:= initTmpConstraint__:= {initusrvary_1lhs = prev(df(usrvary,t)), initusrvarylhs = prev(usrvary)}$"
    "tmpVariables__:= guardVars__:= {df(usrvary,t,2), prev(df(usrvary,t,2)), prev(df(usrvary,t)), prev(usrvarg), prev(usrvary), usrvary}$"
    "pConstraint__:= {}$"

    "symbolic redeval '(checkConsistencyInterval);";

  BOOST_CHECK(check(query_rcs_recovery_cci1, "(list true false)"));
}

/**
 * checkConsistencyIntervalMain()
 */
BOOST_AUTO_TEST_CASE(checkConsistencyIntervalMain_test){
  const string query = 
    "depend {ht,v}, t$"
    // removeinitconsのため
    "initVariables__:={inithtlhs,initvlhs}$"

    "cons_:= {df(ht,t) = v, df(v,t) = -10 }$"
    "variables__:= {}$"
    "guardCons_:={}$"
    "rconts_:={inithtlhs = 10, initvlhs = 0 }$"
    "pCons_:={}$"
    "vars_:={ht,v,df(ht,t),df(v,t)}$"

    "symbolic redeval '(checkConsistencyIntervalMain cons_ guardCons_ rconts_ pCons_ vars_);";

  BOOST_CHECK(check(query, "(list true false)"));

  const string query_bouncing_particle_ccim2 =
    "depend {usrvar_y}, t$"
    // exIneqSolveのため
    "variables__:={df(usrvar_y,t,2), df(usrvar_y,t), prev(df(usrvar_y,t)), prev(usrvar_y), usrvar_y}$"
    "parameters__:={}$"
    // removeinitconsのため
    "initVariables__:={initusrvar_y_1lhs, initusrvar_ylhs}$"

    "cons_:= {df(usrvar_y,t,2) = -10}$"
    "guardCons_:= {usrvar_y = 0}$"
    "initCons_:= {initusrvar_y_1lhs = 0,initusrvar_ylhs = 10}$"
    "pCons_:= {}$"
    "vars_:= {df(usrvar_y,t,2), df(usrvar_y,t), prev(df(usrvar_y,t)), prev(usrvar_y), usrvar_y}$"

    "symbolic redeval '(checkConsistencyIntervalMain cons_ guardCons_ initCons_ pCons_ vars_);";
  BOOST_CHECK(check(query_bouncing_particle_ccim2, "(list false true)"));
}

/**
 * calculate_next_PP_time()
 */
BOOST_AUTO_TEST_CASE(calculateNextPointPhaseTime_test){
  string query = 
    "depend usrvary,t$"
    "constraint__:= {usrvarg = 10,usrvary = 5*(-t**2+2)}$"
    "initConstraint__:= {}$"
    "pConstraint__:= {}$"
    "variables__:= {df(usrvary,t,2), df(usrvary,t), usrvarg, usrvary}$"

    // calcNextPointPhaseTime内部どこかで使われる
    "parameters__:= {}$"

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
    "symbolic redeval '(exSub lhs rhs);";
  BOOST_CHECK(check(query2, "(neq 0 0)"));

  const string query3 = 
    "depend usrvary,t$"
    "lhs:= {prev(df(usrvary,t))=1, prev(usrvary)=0}$"
    "rhs:= {prev(usrvary) neq 0}$"
    "symbolic redeval '(exSub lhs rhs);";
  BOOST_CHECK(check(query3, "(list (neq 0 0))"));

}

BOOST_AUTO_TEST_CASE(findMinTime_test){
  const string query =
    "integAsk_:= (5000*t**2 - 5929*t - 9000 <= 0 and  - 5000*t**2 + 5929*t + 1000 <= 0) and 150*t - 169 = 0$"
    "condDNF_:= {{true}}$"
    "symbolic redeval '(findMinTime integAsk_ condDNF_);";

  // TODO getSqrtIntervalのボトルネック問題を解決する
  // BOOST_CHECK(check(query, "(list)", true));
}

BOOST_AUTO_TEST_CASE(calculateNextPointPhaseTimeMain_test){
  const string query1 = 
    "depend usrvary,t$"
    "maxTime_ := 5$"
    "discCause_ := {usrvary = 0}$"
    "cons_ := {usrvary = parameter_y_0_1 - 5*t**2}$"
    "initCons_ := {initusrvary_1lhs = 0,initusrvarylhs = parameter_y_0_1}$"
    "pCons_ := {parameter_y_0_1 - 9 > 0,parameter_y_0_1 - 11 < 0}$"
    "variables__ := vars_ := {df(usrvary,t,2),df(usrvary,t),usrvary}$"
    "parameters__ := {parameter_y_0_1}$"

    "symbolic redeval '(calculateNextPointPhaseTimeMain maxTime_ discCause_ cons_ initCons_ pCons_ vars_);";

  BOOST_CHECK(check(query1, "(list (list (quotient (times (sqrt parameter_y_0_1) (sqrt 5)) 5) (list (list (list parameter_y_0_1 2 9) (list parameter_y_0_1 1 11))) 0))"));

  // query1の内部で動作する関数
  const string query2 = 
     "parameters__ := {py}$"
     "candidateTCList_ := {{(sqrt(py)*sqrt(5))/5, {{{py,geq,9},{py,leq,11}}}}}$"
     "retTCList_ := {}$"
     "newTC_ := {infinity, {{{py,geq,9},{py,leq,11}}}}$"

     "symbolic redeval '(makeMapAndUnion candidateTCList_ retTCList_ newTC_);";

  BOOST_CHECK(check(query2, "(list (list (quotient (times (sqrt py) (sqrt 5)) 5) (list (list (list py leq 11) (list py geq 9)))))"));

  // query2の内部で動作する関数
  const string query3 = 
     "parameters__ := {py}$"
     "TC1_ := {infinity, {{{py,geq,9},{py,leq,11}}}}$"
     "TC2_ := {(sqrt(py)*sqrt(5))/5, {{{py,geq,9},{py,leq,11}}}}$"
     "mode_ := min$"

     "symbolic redeval '(compareParamTime TC1_ TC2_ mode_);";

  BOOST_CHECK(check(query3, "(list (list (quotient (times (sqrt py) (sqrt 5)) 5) (list (list (list py leq 11) (list py geq 9)))))"));

  const string query_case = 
    "depend usrvary,t$"
    "maxTime_ := 7/5$"
    "discCause_ := {usrvary = 0}$"
    "cons_ := {usrvary = parameter_y_0_1 - 5*t**2}$"
    "initCons_ := {initusrvary_1lhs = 0,initusrvarylhs = parameter_y_0_1}$"
    "pCons_ := {parameter_y_0_1 - 9 > 0,parameter_y_0_1 - 11 < 0}$"
    "variables__ := vars_ := {df(usrvary,t,2),df(usrvary,t),usrvary}$"
    "parameters__ := {parameter_y_0_1}$"

    "symbolic redeval '(calculateNextPointPhaseTimeMain maxTime_ discCause_ cons_ initCons_ pCons_ vars_);";

  BOOST_CHECK(check(query_case, "(list (list (quotient (times (sqrt parameter_y_0_1) (sqrt 5)) 5) (list (list (list parameter_y_0_1 1 (quotient 49 5)) (list parameter_y_0_1 2 9))) 0) (list (quotient 7 5) (list (list (list parameter_y_0_1 4 (quotient 49 5)) (list parameter_y_0_1 1 11))) 1))"));
}


BOOST_AUTO_TEST_CASE(createVariableMapMain_test){
  const string query1 =
    "depend usrvary, t$"
    "cons_:={df(usrvary,t,2) = -10, df(usrvary,t) = 0, - usrvary + 9 <= 0, usrvary - 11 <= 0}$"
    "vars_:={df(usrvary,t,2), df(usrvary,t), prev(df(usrvary,t)), prev(usrvary), usrvary}$"
    "pars_:={}$"
    // exIneqSolveのため
    "variables__:= {df(usrvary,t,2), df(usrvary,t), prev(df(usrvary,t,2)), prev(df(usrvary,t)), prev(usrvarg), prev(usrvary), usrvary}$"

    "symbolic redeval '(createVariableMapMain cons_ vars_ pars_);";

  BOOST_CHECK(check(query1, "(list (list (df usrvary t 2) 0 -10) (list (df usrvary t) 0 0) (list usrvary 4 9) (list usrvary 3 11))"));
  const string query2 =
    "depend {usrvarvx, usrvarvy, usrvarx, usrvary}, t$"
    "cons_:={df(usrvarvx,t) = 0, df(usrvarvy,t) = -10, df(usrvarx,t) = usrvarvx, df(usrvary,t) = usrvarvy, usrvare = 77/100, usrvarvx = 1, usrvarvy = 0, usrvarx = 4/3, usrvary = 20}$"
    "vars_:={df(usrvarvx,t), df(usrvarvy,t), df(usrvarx,t), df(usrvary,t), prev(usrvarvx), prev(usrvarvy), prev(usrvarx), prev(usrvary), usrvare, usrvarvx, usrvarvy, usrvarx, usrvary}$"
    "pars_:={}$"
    // exIneqSolveのため
    "variables__:= {df(usrvarvx,t), df(usrvarvy,t), df(usrvarx,t), df(usrvary,t), prev(usrvarvx), prev(usrvarvy), prev(usrvarx), prev(usrvary), usrvare, usrvarvx, usrvarvy, usrvarx, usrvary}$"
    "symbolic redeval '(createVariableMapMain cons_ vars_ pars_);";

  BOOST_CHECK(check(query2, "(list (list (df usrvarvx t) 0 0) (list (df usrvarvy t) 0 -10) (list (df usrvarx t) 0 1) (list (df usrvary t) 0 0) (list usrvare 0 (quotient 77 100)) (list usrvarvx 0 1) (list usrvarvy 0 0) (list usrvarx 0 (quotient 4 3)) (list usrvary 0 20))"));

  const string query3 =
    "depend usrvary, t$"
    "cons_:={df(usrvary,t,2) = -10, usrvary = 10, - df(usrvary,t) <= 0, df(usrvary,t) - 2 <= 0}$"
    "vars_:={df(usrvary,t,2), df(usrvary,t), prev(df(usrvary,t)), prev(usrvary), usrvary}$"
    "pars_:={}$"
    // exIneqSolveのため
    "variables__:= {df(usrvary,t,2), df(usrvary,t), prev(usrvary), usrvary}$"
    "symbolic redeval '(createVariableMapMain cons_ vars_ pars_);";

  BOOST_CHECK(check(query3, "(list (list (df usrvary t 2) 0 -10) (list (df usrvary t) 4 0) (list (df usrvary t) 3 2) (list usrvary 0 10))"));

  const string query4 =
    "depend {usrvarvx, usrvarvy, usrvarx, usrvary}, t$"
    "cons_:={df(usrvarx,t) = usrvarvx, df(usrvary,t) = usrvarvy, usrvarvx = (375*sqrt(5) + 974)/(54*sqrt(5) - 937), usrvarvy = ( - 480*sqrt(5) + 1288)/(54*sqrt(5) - 937), usrvarx = (160*sqrt(5) + 75)/89, usrvary = (80*sqrt(5) - 96)/89}$"
    "vars_:={df(usrvarx,t), df(usrvary,t), prev(usrvarvx), prev(usrvarvy), prev(usrvarx), prev(usrvary), usrvarvx, usrvarvy, usrvarx, usrvary}$"
    "pars_:={}$"
    // exIneqSolveのため
    "variables__:= {df(usrvarx,t), df(usrvary,t), prev(usrvarvx), prev(usrvarvy), prev(usrvarx), prev(usrvary), usrvarvx, usrvarvy, usrvarx, usrvary}$"
    "symbolic redeval '(createVariableMapMain cons_ vars_ pars_);";

  BOOST_CHECK(check(query4, "(list (list (df usrvarx t) 0 (quotient (plus (minus (times 51 (sqrt 5))) (minus 128)) 109)) (list (df usrvary t) 0 (quotient (plus (times 48 (sqrt 5)) (minus 136)) 109)) (list usrvarvx 0 (quotient (plus (minus (times 51 (sqrt 5))) (minus 128)) 109)) (list usrvarvy 0 (quotient (plus (times 48 (sqrt 5)) (minus 136)) 109)) (list usrvarx 0 (quotient (plus (times 160 (sqrt 5)) 75) 89)) (list usrvary 0 (quotient (plus (times 80 (sqrt 5)) (minus 96)) 89)))"));
}

BOOST_AUTO_TEST_CASE(createVariableMapInterval_test){
  string query_rcs_recovery = 
    "depend usrvary,t$"
    // removeinitconsのため
    "initVariables__:={initusrvary_1lhs,initusrvarylhs}$"

    "constraint__:= {df(usrvary,t,2) = -10, prev(df(usrvary,t,2)) = -10, prev(df(usrvary,t)) = 0, prev(usrvarg) = 10, prev(usrvary) = 10}$"
    "initConstraint__:= {initusrvary_1lhs = prev(df(usrvary,t)),initusrvarylhs = prev(usrvary)}$"
    "pConstraint__:= {}$"
    "variables__:= {df(usrvary,t,2), df(usrvary,t), prev(df(usrvary,t,2)), prev(df(usrvary,t)), prev(usrvarg), prev(usrvary), usrvary}$"

    "symbolic redeval '(createVariableMapInterval);";

  BOOST_CHECK(check(query_rcs_recovery, "(list (list (df usrvary t 2) 0 -10) (list (df usrvary t) 0 (minus (times 10 t))) (list usrvary 0 (plus (minus (times 5 (expt t 2))) 10)))"));
}

BOOST_AUTO_TEST_CASE(balloon_tank_checkConsistency_test){
  const string query_balloon_tank_ccpm =
    "depend {usrvar_ex!!t, usrvar_h, usrvar_timer, usrvar_volume}, t$"
    "cons_:= { - usrvar_ex!!t + 2 < 0 and usrvar_ex!!t - 4 < 0, - usrvar_volume + 1 < 0 and usrvar_volume - 3 < 0, df(usrvar_ex!!t,t) = 0, df(usrvar_h,t) = 0, df(usrvar_timer,t) = 1, df(usrvar_volume,t) = 0, usrvar_ex!!t = prev(usrvar_ex!!t), usrvar_fuel = 1, usrvar_h = prev(usrvar_h), usrvar_h = 10, usrvar_timer = prev(usrvar_timer), usrvar_timer = 0, usrvar_volume = prev(usrvar_volume)}$"
    "pCons_:= {}$"
    "vars_:= {df(usrvar_ex!!t,t), df(usrvar_h,t), df(usrvar_timer,t), df(usrvar_volume,t), prev(usrvar_ex!!t), prev(usrvar_h), prev(usrvar_timer), prev(usrvar_volume), usrvar_ex!!t, usrvar_fuel, usrvar_h, usrvar_timer, usrvar_volume}$"

    "symbolic redeval '(checkConsistencyPointMain cons_ pCons_ vars_);";
  BOOST_CHECK(check(query_balloon_tank_ccpm, "(list true false)"));

  const string query_balloon_tank_ccim =
    "depend {usrvar_ex!!t, usrvar_h, usrvar_timer, usrvar_volume}, t$"
    // exIneqSolveのため
    "variables__:= {df(usrvar_ex!!t,t), df(usrvar_timer,t), df(usrvar_volume,t), prev(usrvar_ex!!t), prev(usrvar_timer), prev(usrvar_volume), usrvar_ex!!t, usrvar_timer, usrvar_volume}$"
    "parameters__:= {parameter_ex!!t_0_1,parameter_volume_0_1}$"
    // removeinitconsのため
    "initVariables__:= {initusrvar_ex!!tlhs, initusrvar_h_1lhs, initusrvar_h_2lhs, initusrvar_hlhs, initusrvar_timerlhs, initusrvar_volumelhs}$"

    "cons_:={df(usrvar_ex!!t,t) = 0,df(usrvar_timer,t) = 1,df(usrvar_volume,t) = 0,usrvar_timer - usrvar_volume < 0}$"
    "guardCons_:={}$"
    "initCons_:={initusrvar_ex!!tlhs = parameter_ex!!t_0_1,initusrvar_timerlhs = 0,initusrvar_volumelhs = parameter_volume_0_1}$"
    "pCons_:={parameter_ex!!t_0_1 - 2 > 0, parameter_volume_0_1 - 1 > 0, parameter_ex!!t_0_1 - 4 < 0, parameter_volume_0_1 - 3 < 0}$"
    "vars_:={df(usrvar_ex!!t,t), df(usrvar_timer,t), df(usrvar_volume,t), prev(usrvar_ex!!t), prev(usrvar_timer), prev(usrvar_volume), usrvar_ex!!t, usrvar_timer, usrvar_volume}$"

    "symbolic redeval '(checkConsistencyIntervalMain cons_ guardCons_ initCons_ pCons_ vars_);";
  BOOST_CHECK(check(query_balloon_tank_ccim, "(list (list (list parameter_ex!!t_0_1 2 2) (list parameter_ex!!t_0_1 1 4) (list parameter_volume_0_1 2 1) (list parameter_volume_0_1 1 3)) false)"));

  const string query_test_ccim =
    "depend usrvar_volume, t$"
    // exIneqSolveのため
    "variables__:= {df(usrvar_volume,t),prev(usrvar_volume),usrvar_volume}$"
    "parameters__:= {parameter_volume_0_1}$"
    // removeinitconsのため
    "initVariables__:= {initusrvar_volumelhs}$"

    "cons_:={df(usrvar_volume,t) = 0}$"
    "guardCons_:={}$"
    "initCons_:={initusrvar_volumelhs = parameter_volume_0_1}$"
    "pCons_:={parameter_volume_0_1 - 1 > 0,parameter_volume_0_1 - 3 < 0}$"
    "vars_:={df(usrvar_volume,t),prev(usrvar_volume),usrvar_volume}$"

    "symbolic redeval '(checkConsistencyIntervalMain cons_ guardCons_ initCons_ pCons_ vars_);";
  BOOST_CHECK(check(query_test_ccim, "(list (list (list parameter_volume_0_1 2 1) (list parameter_volume_0_1 1 3)) false)"));

  const string query_test_ccim2 =
    "depend usrvar_volume, t$"
    // exIneqSolveのため
    "variables__:= {df(usrvar_volume,t),prev(usrvar_volume),usrvar_volume}$"
    "parameters__:= {parameter_volume_0_1}$"
    // removeinitconsのため
    "initVariables__:= {initusrvar_volumelhs}$"

    "cons_:={df(usrvar_volume,t) = 0}$"
    "guardCons_:={usrvar_volume > 0}$"
    "initCons_:={initusrvar_volumelhs = parameter_volume_0_1}$"
    "pCons_:={parameter_volume_0_1 - 1 > 0,parameter_volume_0_1 - 3 < 0}$"
    "vars_:={df(usrvar_volume,t),prev(usrvar_volume),usrvar_volume}$"

    "symbolic redeval '(checkConsistencyIntervalMain cons_ guardCons_ initCons_ pCons_ vars_);";
  BOOST_CHECK(check(query_test_ccim2, "(list (list (list parameter_volume_0_1 2 1) (list parameter_volume_0_1 1 3)) false)"));

  const string query_test_ccim3 =
    "depend usrvar_volume, t$"
    // exIneqSolveのため
    "variables__:= {df(usrvar_volume,t),prev(usrvar_volume),usrvar_volume}$"
    "parameters__:= {parameter_volume_0_1}$"
    // removeinitconsのため
    "initVariables__:= {initusrvar_volumelhs}$"

    "cons_:={df(usrvar_volume,t) = 0}$"
    "guardCons_:={usrvar_volume <= 0}$"
    "initCons_:={initusrvar_volumelhs = parameter_volume_0_1}$"
    "pCons_:={parameter_volume_0_1 - 1 > 0,parameter_volume_0_1 - 3 < 0}$"
    "vars_:={df(usrvar_volume,t),prev(usrvar_volume),usrvar_volume}$"

    "symbolic redeval '(checkConsistencyIntervalMain cons_ guardCons_ initCons_ pCons_ vars_);";
  BOOST_CHECK(check(query_test_ccim3, "(list false (list (list parameter_volume_0_1 2 1) (list parameter_volume_0_1 1 3)))"));

  // モチベーション例題
  const string query_balloon_tank_ccim_fuel_equal_1 =
    "depend {usrvar_ex!!t, usrvar_h, usrvar_timer, usrvar_volume}, t$"
    // exIneqSolveのため
    "variables__:= {df(usrvar_ex!!t,t), df(usrvar_timer,t), df(usrvar_volume,t), prev(usrvar_ex!!t), prev(usrvar_timer), prev(usrvar_volume), usrvar_ex!!t, usrvar_timer, usrvar_volume}$"
    "parameters__:= {parameter_ex!!t_0_1,parameter_volume_0_1}$"
    // removeinitconsのため
    "initVariables__:= {initusrvar_ex!!tlhs, initusrvar_h_1lhs, initusrvar_h_2lhs, initusrvar_hlhs, initusrvar_timerlhs, initusrvar_volumelhs}$"

    "cons_:={df(usrvar_ex!!t,t) = 0,df(usrvar_h,t,3) = 0,df(usrvar_timer,t) = 1,df(usrvar_volume,t) = 0}$"
    "guardCons_:={usrvar_fuel = 1}$"
    "initCons_:={initusrvar_ex!!tlhs = parameter_ex!!t_0_1,initusrvar_h_1lhs = 0,initusrvar_h_2lhs = 1,initusrvar_hlhs = 10,initusrvar_timerlhs = 0,initusrvar_volumelhs = parameter_volume_0_1}$"
    "pCons_:={parameter_ex!!t_0_1 - 2 > 0, parameter_volume_0_1 - 1 > 0, parameter_ex!!t_0_1 - 4 < 0, parameter_volume_0_1 - 3 < 0}$"
    "vars_:={df(usrvar_ex!!t,t), df(usrvar_h,t,3), df(usrvar_h,t,2), df(usrvar_h,t), df(usrvar_timer,t), df(usrvar_volume,t), prev(df(usrvar_h,t,2)), prev(df(usrvar_h,t)), prev(usrvar_ex!!t), prev(usrvar_h), prev(usrvar_timer), prev(usrvar_volume), usrvar_ex!!t, usrvar_fuel, usrvar_h, usrvar_timer, usrvar_volume}$"

    "symbolic redeval '(checkConsistencyIntervalMain cons_ guardCons_ initCons_ pCons_ vars_);";

  BOOST_CHECK(check(query_balloon_tank_ccim_fuel_equal_1, "(list (list (list parameter_ex!!t_0_1 2 2) (list parameter_ex!!t_0_1 1 4) (list parameter_volume_0_1 2 1) (list parameter_volume_0_1 1 3)) false)"));

  // モチベーション例題 肝心の箇所
  const string query_balloon_tank_ccim4_exDSolve =
    "depend {usrvar_ex!!t, usrvar_h, usrvar_timer, usrvar_volume}, t$"
    // removeInitConsのため
    "initVariables__:= {initusrvar_ex!!tlhs, initusrvar_h_1lhs, initusrvar_h_2lhs, initusrvar_hlhs, initusrvar_timerlhs, initusrvar_volumelhs}$"

    "cons_:= {df(usrvar_ex!!t,t) = 0,df(usrvar_h,t,3) = 0,df(usrvar_timer,t) = 1,df(usrvar_volume,t) = 0}$"
    "guardCons_:= {usrvar_fuel = 1}$"
    "init_:= {initusrvar_ex!!tlhs = parameter_ex!!t_0_1,initusrvar_h_1lhs = 0,initusrvar_h_2lhs = 1,initusrvar_hlhs = 10,initusrvar_timerlhs = 0,initusrvar_volumelhs = parameter_volume_0_1}$"
    "vars_:= {df(usrvar_ex!!t,t), df(usrvar_h,t,3), df(usrvar_h,t,2), df(usrvar_h,t), df(usrvar_timer,t), df(usrvar_volume,t), prev(df(usrvar_h,t,2)), prev(df(usrvar_h,t)), prev(usrvar_ex!!t), prev(usrvar_h), prev(usrvar_timer), prev(usrvar_volume), usrvar_ex!!t, usrvar_fuel, usrvar_h, usrvar_timer, usrvar_volume}$"

    "symbolic redeval '(exDSolve cons_ guardCons_ init_ vars_);";
  BOOST_CHECK(check(query_balloon_tank_ccim4_exDSolve, "(list (list) (list (equal usrvar_ex!!t parameter_ex!!t_0_1) (equal usrvar_fuel 1) (equal usrvar_h (quotient (plus (expt t 2) 20) 2)) (equal usrvar_timer t) (equal usrvar_volume parameter_volume_0_1)))"));

  // guardを上手くexDSolveで処理しないと失敗する例
  const string query_circle_ccim2 =
    "depend {usrvar_vx, usrvar_vy, usrvar_x, usrvar_y}, t$"
    // exIneqSolveのため
    "variables__:= {df(usrvar_vx,t), df(usrvar_vy,t), df(usrvar_x,t), df(usrvar_y,t), prev(usrvar_vx), prev(usrvar_vy), prev(usrvar_x), prev(usrvar_y), usrvar_vx, usrvar_vy, usrvar_x, usrvar_y}$"
    "parameters__:= {}$"
    // removeinitconsのため
    "initVariables__:= {initusrvar_vxlhs,initusrvar_vylhs,initusrvar_xlhs,initusrvar_ylhs}$"

    "cons_:={df(usrvar_vx,t) = 0,df(usrvar_vy,t) = 0,df(usrvar_x,t) = usrvar_vx,df(usrvar_y,t) = usrvar_vy}$"
    "guardCons_:={usrvar_x**2 + usrvar_y**2 = 1}$"
    "initCons_:={initusrvar_vxlhs = 2,initusrvar_vylhs = 1,initusrvar_xlhs = 3/4,initusrvar_ylhs = 0}$"
    "pCons_:={}$"
    "vars_:={df(usrvar_vx,t), df(usrvar_vy,t), df(usrvar_x,t), df(usrvar_y,t), prev(usrvar_vx), prev(usrvar_vy), prev(usrvar_x), prev(usrvar_y), usrvar_vx, usrvar_vy, usrvar_x, usrvar_y}$"

    "symbolic redeval '(checkConsistencyIntervalMain cons_ guardCons_ initCons_ pCons_ vars_);";

  BOOST_CHECK(check(query_circle_ccim2, "(list false true)"));

  const string query_balloon_tank_ccim_timer_geq_volume =
    "depend {usrvar_ex!!t, usrvar_h, usrvar_timer, usrvar_volume}, t$"
    // exIneqSolveのため
    "variables__:= {df(usrvar_ex!!t,t), df(usrvar_timer,t), df(usrvar_volume,t), prev(usrvar_ex!!t), prev(usrvar_timer), prev(usrvar_volume), usrvar_ex!!t, usrvar_timer, usrvar_volume}$"
    "parameters__:= {parameter_ex!!t_0_1,parameter_volume_0_1}$"
    // removeinitconsのため
    "initVariables__:= {initusrvar_ex!!tlhs, initusrvar_h_1lhs, initusrvar_h_2lhs, initusrvar_hlhs, initusrvar_timerlhs, initusrvar_volumelhs}$"

    "cons_:={df(usrvar_ex!!t,t) = 0,df(usrvar_timer,t) = 1,df(usrvar_volume,t) = 0}$"
    "guardCons_:={usrvar_timer - usrvar_volume >= 0}$"
    "initCons_:={initusrvar_ex!!tlhs = parameter_ex!!t_0_1,initusrvar_timerlhs = 0,initusrvar_volumelhs = parameter_volume_0_1}$"
    "pCons_:={parameter_ex!!t_0_1 - 2 > 0, parameter_volume_0_1 - 1 > 0, parameter_ex!!t_0_1 - 4 < 0, parameter_volume_0_1 - 3 < 0}$"
    "vars_:={df(usrvar_ex!!t,t), df(usrvar_timer,t), df(usrvar_volume,t), prev(usrvar_ex!!t), prev(usrvar_timer), prev(usrvar_volume), usrvar_ex!!t, usrvar_timer, usrvar_volume}$"

    "symbolic redeval '(checkConsistencyIntervalMain cons_ guardCons_ initCons_ pCons_ vars_);";
  // someday
  //BOOST_CHECK(check(query_balloon_tank_ccim_timer_geq_volume, "(list false (list (list parameter_ex!!t_0_1 2 2) (list parameter_ex!!t_0_1 1 4) (list parameter_volume_0_1 2 1) (list parameter_volume_0_1 1 3)))"));

  const string query_balloon_tank_ccim_timer_less_volume =
    "depend {usrvar_ex!!t, usrvar_h, usrvar_timer, usrvar_volume}, t$"
    // exIneqSolveのため
    "variables__:= {df(usrvar_ex!!t,t), df(usrvar_timer,t), df(usrvar_volume,t), prev(usrvar_ex!!t), prev(usrvar_timer), prev(usrvar_volume), usrvar_ex!!t, usrvar_timer, usrvar_volume}$"
    "parameters__:= {parameter_ex!!t_0_1,parameter_volume_0_1}$"
    // removeinitconsのため
    "initVariables__:= {initusrvar_ex!!tlhs, initusrvar_h_1lhs, initusrvar_h_2lhs, initusrvar_hlhs, initusrvar_timerlhs, initusrvar_volumelhs}$"

    "cons_:={df(usrvar_ex!!t,t) = 0,df(usrvar_timer,t) = 1,df(usrvar_volume,t) = 0}$"
    "guardCons_:={usrvar_timer - usrvar_volume < 0}$"
    "initCons_:={initusrvar_ex!!tlhs = parameter_ex!!t_0_1,initusrvar_timerlhs = 0,initusrvar_volumelhs = parameter_volume_0_1}$"
    "pCons_:={parameter_ex!!t_0_1 - 2 > 0, parameter_volume_0_1 - 1 > 0, parameter_ex!!t_0_1 - 4 < 0, parameter_volume_0_1 - 3 < 0}$"
    "vars_:={df(usrvar_ex!!t,t), df(usrvar_timer,t), df(usrvar_volume,t), prev(usrvar_ex!!t), prev(usrvar_timer), prev(usrvar_volume), usrvar_ex!!t, usrvar_timer, usrvar_volume}$"

    "symbolic redeval '(checkConsistencyIntervalMain cons_ guardCons_ initCons_ pCons_ vars_);";
  // someday
  // BOOST_CHECK(check(query_balloon_tank_ccim_timer_less_volume, "(list (list (list parameter_ex!!t_0_1 2 2) (list parameter_ex!!t_0_1 1 4) (list parameter_volume_0_1 2 1) (list parameter_volume_0_1 1 3)) false)"));

  const string query_balloon_tank_ccim_fuel_isnot_1 =
    "depend {usrvar_ex!!t, usrvar_h, usrvar_timer, usrvar_volume}, t$"
    // exIneqSolveのため
    "variables__:= {df(usrvar_ex!!t,t), df(usrvar_timer,t), df(usrvar_volume,t), prev(usrvar_ex!!t), prev(usrvar_timer), prev(usrvar_volume), usrvar_ex!!t, usrvar_timer, usrvar_volume}$"
    "parameters__:= {parameter_ex!!t_0_1,parameter_volume_0_1}$"
    // removeinitconsのため
    "initVariables__:= {initusrvar_ex!!tlhs, initusrvar_h_1lhs, initusrvar_h_2lhs, initusrvar_hlhs, initusrvar_timerlhs, initusrvar_volumelhs}$"

    "cons_:={df(usrvar_ex!!t,t) = 0,df(usrvar_h,t,3) = 0,df(usrvar_timer,t) = 1,df(usrvar_volume,t) = 0}$"
    "guardCons_:={usrvar_fuel - 1 <> 0}$"
    "initCons_:={initusrvar_ex!!tlhs = parameter_ex!!t_0_1,initusrvar_h_1lhs = 0,initusrvar_h_2lhs = 1,initusrvar_hlhs = 10,initusrvar_timerlhs = 0,initusrvar_volumelhs = parameter_volume_0_1}$"
    "pCons_:={parameter_ex!!t_0_1 - 2 > 0, parameter_volume_0_1 - 1 > 0, parameter_ex!!t_0_1 - 4 < 0, parameter_volume_0_1 - 3 < 0}$"
    "vars_:={df(usrvar_ex!!t,t), df(usrvar_h,t,3), df(usrvar_h,t,2), df(usrvar_h,t), df(usrvar_timer,t), df(usrvar_volume,t), prev(df(usrvar_h,t,2)), prev(df(usrvar_h,t)), prev(usrvar_ex!!t), prev(usrvar_h), prev(usrvar_timer), prev(usrvar_volume), usrvar_ex!!t, usrvar_fuel, usrvar_h, usrvar_timer, usrvar_volume}$" 
    "symbolic redeval '(checkConsistencyIntervalMain cons_ guardCons_ initCons_ pCons_ vars_);";

  BOOST_CHECK(check(query_balloon_tank_ccim_fuel_isnot_1, "(list (list (list parameter_ex!!t_0_1 2 2) (list parameter_ex!!t_0_1 1 4) (list parameter_volume_0_1 2 1) (list parameter_volume_0_1 1 3)) false)"));
}

BOOST_AUTO_TEST_CASE(dSolveByLaplace_test){
  string query_square =
    "depend {usrvar_f}, t$"
    "expr_:= {df(usrvar_f,t) = 0}$"
    "initCons_:= {initusrvar_flhs = 0,initusrvar_tlhs = 0}$"
    "vars_:= {usrvar_f}$"

    "symbolic redeval '(dSolveByLaplace expr_ initCons_ vars_);";
  BOOST_CHECK(check(query_square, "(list (equal usrvar_f 0))"));

  string query_square_exDSolve =
    "depend {usrvar_f, usrvar_t}, t$"
    "cons_:= {df(usrvar_f,t) = 0,df(usrvar_t,t) = 1}$"
    "guardCons_:= {}$"
    "initCons_:= {initusrvar_flhs = 0,initusrvar_tlhs = 0}$"
    "vars_:= {df(usrvar_f,t), df(usrvar_t,t), prev(usrvar_f), prev(usrvar_t), usrvar_f, usrvar_t}$"

    "symbolic redeval '(exDSolve cons_ guardCons_ initCons_ vars_);";
  BOOST_CHECK(check(query_square_exDSolve, "(list (list) (list (equal usrvar_f 0) (equal usrvar_t t)))"));
}

BOOST_AUTO_TEST_CASE(broken_example_test){
  string query_bouncing_particle_rp_ccim2 =
    "depend {usrvar_e, usrvar_ht, usrvar_v}, t$"
    // exIneqSolveのため
    "variables__:= {df(usrvar_e,t), df(usrvar_ht,t), df(usrvar_v,t), prev(usrvar_e), prev(usrvar_ht), prev(usrvar_v), usrvar_e, usrvar_ht, usrvar_v}$"
    "parameters__:= {parameter_ht_0_1}$"
    // removeinitconsのため
    "initVariables__:= {initusrvar_elhs,initusrvar_htlhs,initusrvar_vlhs}$"

    "cons_:={df(usrvar_e,t) = 0,df(usrvar_ht,t) = usrvar_v,df(usrvar_v,t) = -10}$"
    "guardCons_:={usrvar_ht = 0}$"
    "initCons_:={initusrvar_elhs = 4/5,initusrvar_htlhs = parameter_ht_0_1,initusrvar_vlhs = 0}$"
    "pCons_:={parameter_ht_0_1 - 5 > 0,parameter_ht_0_1 - 15 < 0}$"
    "vars_:={df(usrvar_e,t), df(usrvar_ht,t), df(usrvar_v,t), prev(usrvar_e), prev(usrvar_ht), prev(usrvar_v), usrvar_e, usrvar_ht, usrvar_v}$"

    "symbolic redeval '(checkConsistencyIntervalMain cons_ guardCons_ initCons_ pCons_ vars_);";

  //BOOST_CHECK(check(query_bouncing_particle_rp_ccim2 , "(list false (list (list parameter_ht_0_1 2 5) (list parameter_ht_0_1 1 15)))"));
}

BOOST_AUTO_TEST_CASE(balloon_tank_calculateNextPointPhaseTimeMain_test){
  const string query_balloon_tank_cnppt =
    "depend {usrvar_ex!!t, usrvar_h, usrvar_timer, usrvar_volume}, t$"
    // convertValueToIntervalで必要
    "pConstraint__:= {parameter_ex!!t_0_1 - 2 > 0, parameter_volume_0_1 - 1 > 0, parameter_ex!!t_0_1 - 4 < 0, parameter_volume_0_1 - 3 < 0}$"

    "maxTime_ := 100$"
    "discCause_ := {usrvar_fuel - 1 <> 0, usrvar_timer - usrvar_volume >= 0, usrvar_timer - usrvar_volume >= 0, - usrvar_ex!!t + usrvar_timer - usrvar_volume >= 0, usrvar_fuel = 0, usrvar_h < 0}$"
    "cons_ := {usrvar_ex!!t = parameter_ex!!t_0_1,usrvar_fuel = 1,usrvar_h = (t**2 + 20)/2,usrvar_timer = t,usrvar_volume = parameter_volume_0_1}$"
    "initCons_ := {initusrvar_ex!!tlhs = parameter_ex!!t_0_1,initusrvar_h_1lhs = 0,initusrvar_hlhs = 10,initusrvar_timerlhs = 0,initusrvar_volumelhs = parameter_volume_0_1}$"
    "pCons_ := {parameter_ex!!t_0_1 - 2 > 0, parameter_volume_0_1 - 1 > 0, parameter_ex!!t_0_1 - 4 < 0, parameter_volume_0_1 - 3 < 0}$"
    "variables__ := vars_ := {df(usrvar_ex!!t,t), df(usrvar_h,t,2), df(usrvar_h,t), df(usrvar_timer,t), df(usrvar_volume,t), usrvar_ex!!t, usrvar_fuel, usrvar_h, usrvar_timer, usrvar_volume}$"
    "parameters__ := {parameter_ex!!t_0_1,parameter_volume_0_1}$"

    "symbolic redeval '(calculateNextPointPhaseTimeMain maxTime_ discCause_ cons_ initCons_ pCons_ vars_);";

  BOOST_CHECK(check(query_balloon_tank_cnppt, "(list (list parameter_volume_0_1 (list (list (list parameter_ex!!t_0_1 1 4) (list parameter_ex!!t_0_1 2 2) (list parameter_volume_0_1 2 1) (list parameter_volume_0_1 1 3))) 0))"));
}

BOOST_AUTO_TEST_CASE(bp_yv_test){
  const string query_bp_yv_ccim =
    "depend usrvar_y, t$"
    // exIneqSolveのため
    "variables__:= {df(usrvar_y,t,2), df(usrvar_y,t), prev(df(usrvar_y,t)), prev(usrvar_y), usrvar_y}$"
    "parameters__:= {parameter_y_0_1,parameter_y_1_1}$"
    // removeinitconsのため
    "initVariables__:= {initusrvar_y_1lhs,initusrvar_ylhs}$"

    "cons_:={df(usrvar_y,t,2) = -10}$"
    "guardCons_:={usrvar_y = 0}$"
    "initCons_:={initusrvar_y_1lhs = parameter_y_1_1,initusrvar_ylhs = parameter_y_0_1}$"
    "pCons_:={parameter_y_1_1 >= 0, parameter_y_0_1 - 9 > 0, parameter_y_1_1 - 2 <= 0, parameter_y_0_1 - 11 < 0}$"
    "vars_:={df(usrvar_y,t,2), df(usrvar_y,t), prev(df(usrvar_y,t)), prev(usrvar_y), usrvar_y}$" 
    "symbolic redeval '(checkConsistencyIntervalMain cons_ guardCons_ initCons_ pCons_ vars_);";

  // someday
  // BOOST_CHECK(check(query_bp_yv_ccim, "false", true));

  const string query_bp_yv_cnppt =
    "depend usrvar_y, t$"
    // convertValueToIntervalで必要
    "pConstraint__:= {parameter_y_1_1 >= 0, parameter_y_0_1 - 9 > 0, parameter_y_1_1 - 2 <= 0, parameter_y_0_1 - 11 < 0}$"

    "maxTime_ := 1$"
    "discCause_ := {usrvar_y = 0}$"
    "cons_ := {usrvar_y = parameter_y_0_1 + parameter_y_1_1*t - 5*t**2}$"
    "initCons_ := {initusrvar_y_1lhs = parameter_y_1_1,initusrvar_ylhs = parameter_y_0_1}$"
    "pCons_ := {parameter_y_1_1 >= 0, parameter_y_0_1 - 9 > 0, parameter_y_1_1 - 2 <= 0, parameter_y_0_1 - 11 < 0}$"
    "variables__ := vars_ := {df(usrvar_y,t,2),df(usrvar_y,t),usrvar_y}$"
    "parameters__ := {parameter_y_0_1,parameter_y_1_1}$"

    "symbolic redeval '(calculateNextPointPhaseTimeMain maxTime_ discCause_ cons_ initCons_ pCons_ vars_);";
  // TODO 無限ループの回避
  //BOOST_CHECK(check(query_bp_yv_cnppt, "{{1, Inequality[0, LessEqual, parameter[y, 1, 1], LessEqual, 2] && Inequality[9, Less, parameter[y, 0, 1], Less, 11], 1}}", true));
}

BOOST_AUTO_TEST_CASE(two_sawtooth_waves){
  const string query_1_wave_ccim =
    "depend usrvar_f1, t$"
    // exIneqSolveのため
    "variables__:= {df(usrvar_f1,t),prev(usrvar_f1),usrvar_f1}$"
    "parameters__:= {parameter_f1_0_1}$"
    // removeinitconsのため
    "initVariables__:= {initusrvar_f1lhs}$"

    "cons_:={df(usrvar_f1,t) = 3}$"
    "guardCons_:={usrvar_f1 = 10}$"
    "initCons_:={initusrvar_f1lhs = parameter_f1_0_1}$"
    "pCons_:={parameter_f1_0_1 - 1 > 0,parameter_f1_0_1 - 2 < 0}$"
    "vars_:={df(usrvar_f1,t),prev(usrvar_f1),usrvar_f1}$"
    "symbolic redeval '(checkConsistencyIntervalMain cons_ guardCons_ initCons_ pCons_ vars_);";
   BOOST_CHECK(check(query_1_wave_ccim, "(list false (list (list parameter_f1_0_1 2 1) (list parameter_f1_0_1 1 2)))"));

  const string query_two_waves_cnppt =
    "depend {usrvar_f1, usrvar_f2}, t$"
    "maxTime_:= 100$"
    "discCause_:= {usrvar_f2 = 100,usrvar_f1 = 100}$"
    "cons_:= {usrvar_f1 = parameter_f1_0_1 + 10*t,usrvar_f2 = parameter_f2_0_1 + 10*t}$"
    "initCons_:= {initusrvar_f2lhs = parameter_f2_0_1}$"
    "pConstraint__:= pCons_:= {parameter_f1_0_1 - 1 > 0, parameter_f2_0_1 - 2 > 0, parameter_f1_0_1 - 3 < 0, parameter_f2_0_1 - 4 < 0}$"

    "variables__:= vars_ := {df(usrvar_f1,t),df(usrvar_f2,t),usrvar_f1,usrvar_f2}$"
    "parameters__:= {parameter_f1_0_1,parameter_f2_0_1}$"

    "symbolic redeval '(calculateNextPointPhaseTimeMain maxTime_ discCause_ cons_ initCons_ pCons_ vars_);";
  
  // TODO
  //BOOST_CHECK(check(query_two_waves_cnppt,  "{{( - parameter_f1_0_1 + 100)/10, {{{parameter_f1_0_1,lessp,2}, {parameter_f1_0_1,greaterp,3}, {parameter_f2_0_1,greaterp,2}, {parameter_f2_0_1,lessp,parameter_f1_0_1}}} }, {( - parameter_f2_0_1 + 100)/10, {{{parameter_f1_0_1,lessp,1}, {parameter_f1_0_1,geq,2}, {parameter_f2_0_1,greaterp,2}, {parameter_f2_0_1,lessp,4}}, {{parameter_f1_0_1,lessp,2}, {parameter_f1_0_1,greaterp,3}, {parameter_f2_0_1,leq, parameter_f1_0_1}, {parameter_f2_0_1,lessp,4}}}}, 0}", true));

}


#endif // DISABLE_VCS_REDUCE_SOURCE_TEST

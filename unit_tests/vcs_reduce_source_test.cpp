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

  BOOST_CHECK(check(query, "(list (equal v (minus (times 10 t))) (equal ht (plus (minus (times 5 (expt t 2))) 10)))"));

  const string query_rcs_recovery = 
    "depend {usrvary}, t$"
    "cons_:= {df(usrvary,t,2) = -10}$"
    "guardCons_:={}$"
    "init_:= {initusrvary_1lhs = 0,initusrvarylhs = 10}$"
    "vars_:= {usrvary}$"

    "symbolic redeval '(exDSolve cons_ guardCons_ init_ vars_);";

  BOOST_CHECK(check(query_rcs_recovery, "(list (equal usrvary (plus (minus (times 5 (expt t 2))) 10)))"));
}

/**
 * Hyrose_v0.6.3 checkConsistencyInterval() 1回目
 */
BOOST_AUTO_TEST_CASE(checkConsistencyInterval_test){
  string query =
    "depend usrvary,t$"
    "constraint__:= {df(usrvary,t,2) = -10, prev(df(usrvary,t,2)) = -10, prev(df(usrvary,t)) = 0, prev(usrvarg) = 10, prev(usrvary) = 10, usrvarg = 10}$"
    "variables__:= {df(usrvary,t,2), prev(df(usrvary,t,2)), prev(df(usrvary,t)), prev(usrvarg), prev(usrvary), usrvarg}$"

    "initConstraint__:= initTmpConstraint__:={initusrvary_1lhs = prev(df(usrvary,t)),initusrvarylhs = prev(usrvary)}$"

    "tmpVariables__:={df(usrvary,t),initusrvary_1lhs,initusrvarylhs,usrvary}$"
    "guardVars__:= pConstraint__:= {}$"
    "initVariables__:={initusrvary_1lhs,initusrvarylhs}$"

    "symbolic redeval '(checkConsistencyInterval);";

  BOOST_CHECK(check(query, "(list true false)"));

  string query_rcs_recovery_cci1 = 
    "depend usrvary,t$"
    "constraint__:= {df(usrvary,t,2) = -10, prev(df(usrvary,t,2)) = -10, prev(df(usrvary,t)) = 0, prev(usrvarg) = 10, prev(usrvary) = 10}$"
    "variables__:= {df(usrvary,t,2), df(usrvary,t), prev(df(usrvary,t,2)), prev(df(usrvary,t)), prev(usrvarg), prev(usrvary), usrvary}$"
    "initConstraint__:= initTmpConstraint__:= {initusrvary_1lhs = prev(df(usrvary,t)), initusrvarylhs = prev(usrvary)}$"
    "tmpVariables__:= guardVars__:= {df(usrvary,t,2), prev(df(usrvary,t,2)), prev(df(usrvary,t)), prev(usrvarg), prev(usrvary), usrvary}$"
    "pConstraint__:= {}$"
    "initVariables__:={initusrvary_1lhs,initusrvarylhs}$"

    "symbolic redeval '(checkConsistencyInterval);";

  BOOST_CHECK(check(query_rcs_recovery_cci1, "(list true false)"));
}

/**
 * checkConsistencyIntervalMain()
 */
BOOST_AUTO_TEST_CASE(checkConsistencyIntervalMain_test){
  const string query = 
    "depend {ht,v}, t$"
    "initVariables__:={inithtlhs,initvlhs}$"
    "cons_:= variables__:= {}$"
    "guardCons_:={df(ht,t) = v, df(v,t) = -10 }$"
    "rconts_:={inithtlhs = 10, initvlhs = 0 }$"
    "pCons_:={}$"
    "vars_:={ht,v,df(ht,t),df(v,t)}$"

    "symbolic redeval '(checkConsistencyIntervalMain cons_ guardCons_ rconts_ pCons_ vars_);";

  BOOST_CHECK(check(query, "(list false true)"));

  const string query_bouncing_particle_ccim2 =
    "depend {usrvar_y}, t$"
    "variables__:={df(usrvar_y,t,2), df(usrvar_y,t), prev(df(usrvar_y,t)), prev(usrvar_y), usrvar_y}$"
    "parameters__:={}$"
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
    "constraint__:= {df(usrvary,t,2) = -10, prev(df(usrvary,t,2)) = -10, prev(df(usrvary,t)) = 0, prev(usrvarg) = 10, prev(usrvary) = 10}$"
    "initConstraint__:= {initusrvary_1lhs = prev(df(usrvary,t)),initusrvarylhs = prev(usrvary)}$"
    "pConstraint__:= {}$"
    "variables__:= {df(usrvary,t,2), df(usrvary,t), prev(df(usrvary,t,2)), prev(df(usrvary,t)), prev(usrvarg), prev(usrvary), usrvary}$"
    "initVariables__:={initusrvary_1lhs,initusrvarylhs}$"

    "symbolic redeval '(createVariableMapInterval);";

  BOOST_CHECK(check(query_rcs_recovery, "(list (list (df usrvary t 2) 0 -10) (list (df usrvary t) 0 (minus (times 10 t))) (list usrvary 0 (plus (minus (times 5 (expt t 2))) 10)))"));
}

BOOST_AUTO_TEST_CASE(balloon_tank_test){
  const string query_balloon_tank_ccpm =
    "depend {usrvar_ex!!t, usrvar_h, usrvar_timer, usrvar_volume}, t$"
    "cons_:= { - usrvar_ex!!t + 2 < 0 and usrvar_ex!!t - 4 < 0, - usrvar_volume + 1 < 0 and usrvar_volume - 3 < 0, df(usrvar_ex!!t,t) = 0, df(usrvar_h,t) = 0, df(usrvar_timer,t) = 1, df(usrvar_volume,t) = 0, usrvar_ex!!t = prev(usrvar_ex!!t), usrvar_fuel = 1, usrvar_h = prev(usrvar_h), usrvar_h = 10, usrvar_timer = prev(usrvar_timer), usrvar_timer = 0, usrvar_volume = prev(usrvar_volume)}$"
    "pCons_:= {}$"
    "vars_:= {df(usrvar_ex!!t,t), df(usrvar_h,t), df(usrvar_timer,t), df(usrvar_volume,t), prev(usrvar_ex!!t), prev(usrvar_h), prev(usrvar_timer), prev(usrvar_volume), usrvar_ex!!t, usrvar_fuel, usrvar_h, usrvar_timer, usrvar_volume}$"

    "symbolic redeval '(checkConsistencyPointMain cons_ pCons_ vars_);";
  BOOST_CHECK(check(query_balloon_tank_ccpm, "(list true false)"));

  const string query_balloon_tank_ccim =
    "depend {usrvar_ex!!t, usrvar_h, usrvar_timer, usrvar_volume}, t$"
    "initVariables__:= {initusrvar_ex!!tlhs, initusrvar_h_1lhs, initusrvar_h_2lhs, initusrvar_hlhs, initusrvar_timerlhs, initusrvar_volumelhs}$"
    // exIneqSolveのため
    "variables__:= {df(usrvar_ex!!t,t), df(usrvar_timer,t), df(usrvar_volume,t), prev(usrvar_ex!!t), prev(usrvar_timer), prev(usrvar_volume), usrvar_ex!!t, usrvar_timer, usrvar_volume}$"
    "parameters__:= {parameter_ex!!t_0_1,parameter_volume_0_1}$"

    "cons_:={df(usrvar_ex!!t,t) = 0,df(usrvar_timer,t) = 1,df(usrvar_volume,t) = 0,usrvar_timer - usrvar_volume < 0}$"
    "guardCons_:={}$"
    "initCons_:={initusrvar_ex!!tlhs = parameter_ex!!t_0_1,initusrvar_timerlhs = 0,initusrvar_volumelhs = parameter_volume_0_1}$"
    "pCons_:={parameter_ex!!t_0_1 - 2 > 0, parameter_volume_0_1 - 1 > 0, parameter_ex!!t_0_1 - 4 < 0, parameter_volume_0_1 - 3 < 0}$"
    "vars_:={df(usrvar_ex!!t,t), df(usrvar_timer,t), df(usrvar_volume,t), prev(usrvar_ex!!t), prev(usrvar_timer), prev(usrvar_volume), usrvar_ex!!t, usrvar_timer, usrvar_volume}$"

    "symbolic redeval '(checkConsistencyIntervalMain cons_ guardCons_ initCons_ pCons_ vars_);";
  BOOST_CHECK(check(query_balloon_tank_ccim, "(list (list (list parameter_ex!!t_0_1 2 2) (list parameter_ex!!t_0_1 1 4) (list parameter_volume_0_1 2 1) (list parameter_volume_0_1 1 3)) false)"));

  const string query_test_ccim =
    "depend usrvar_volume, t$"
    "initVariables__:= {initusrvar_volumelhs}$"
    // exIneqSolveのため
    "variables__:= {df(usrvar_volume,t),prev(usrvar_volume),usrvar_volume}$"
    "parameters__:= {parameter_volume_0_1}$"

    "cons_:={df(usrvar_volume,t) = 0}$"
    "guardCons_:={}$"
    "initCons_:={initusrvar_volumelhs = parameter_volume_0_1}$"
    "pCons_:={parameter_volume_0_1 - 1 > 0,parameter_volume_0_1 - 3 < 0}$"
    "vars_:={df(usrvar_volume,t),prev(usrvar_volume),usrvar_volume}$"

    "symbolic redeval '(checkConsistencyIntervalMain cons_ guardCons_ initCons_ pCons_ vars_);";
  BOOST_CHECK(check(query_test_ccim, "(list (list (list parameter_volume_0_1 2 1) (list parameter_volume_0_1 1 3)) false)"));

  const string query_test_ccim2 =
    "depend usrvar_volume, t$"
    "initVariables__:= {initusrvar_volumelhs}$"
    // exIneqSolveのため
    "variables__:= {df(usrvar_volume,t),prev(usrvar_volume),usrvar_volume}$"
    "parameters__:= {parameter_volume_0_1}$"

    "cons_:={df(usrvar_volume,t) = 0}$"
    "guardCons_:={usrvar_volume > 0}$"
    "initCons_:={initusrvar_volumelhs = parameter_volume_0_1}$"
    "pCons_:={parameter_volume_0_1 - 1 > 0,parameter_volume_0_1 - 3 < 0}$"
    "vars_:={df(usrvar_volume,t),prev(usrvar_volume),usrvar_volume}$"

    "symbolic redeval '(checkConsistencyIntervalMain cons_ guardCons_ initCons_ pCons_ vars_);";
  BOOST_CHECK(check(query_test_ccim2, "(list (list (list parameter_volume_0_1 2 1) (list parameter_volume_0_1 1 3)) false)", true));

  const string query_test_ccim3 =
    "depend usrvar_volume, t$"
    "initVariables__:= {initusrvar_volumelhs}$"
    // exIneqSolveのため
    "variables__:= {df(usrvar_volume,t),prev(usrvar_volume),usrvar_volume}$"
    "parameters__:= {parameter_volume_0_1}$"

    "cons_:={df(usrvar_volume,t) = 0}$"
    "guardCons_:={usrvar_volume <= 0}$"
    "initCons_:={initusrvar_volumelhs = parameter_volume_0_1}$"
    "pCons_:={parameter_volume_0_1 - 1 > 0,parameter_volume_0_1 - 3 < 0}$"
    "vars_:={df(usrvar_volume,t),prev(usrvar_volume),usrvar_volume}$"

    "symbolic redeval '(checkConsistencyIntervalMain cons_ guardCons_ initCons_ pCons_ vars_);";
  BOOST_CHECK(check(query_test_ccim3, "(list false (list (list parameter_volume_0_1 2 1) (list parameter_volume_0_1 1 3)))"));
}
#endif // DISABLE_VCS_REDUCE_SOURCE_TEST

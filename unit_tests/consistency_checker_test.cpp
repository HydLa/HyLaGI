/**
 * ConsistencyCheckerのテスト
 */

#include "test_common.h"
#ifndef DISABLE_CONSISTENCY_CHECKER_TEST

#include <boost/shared_ptr.hpp>

#include "math_source.h"
#include "simulator/TellCollector.h"

#include "constraint_hierarchy/ModuleSet.h"

#include "symbolic_simulator/ConsistencyChecker.h"
#include "symbolic_simulator/mathlink_helper.h"
#include "parser/HydLaAST.h"
#include "simulator/TellCollector.h"
#include "parser/NodeFactory.h"
#include "ParseTreeGenerator.h"

using namespace hydla;
using namespace hydla::parser;
using namespace hydla::parse_tree;
using namespace hydla::simulator;
using namespace hydla::simulator::symbolic;
using namespace hydla::ch;
using namespace boost;

BOOST_AUTO_TEST_CASE(ss_consistency_checker_test)
{
  MathLink ml;
  ml.init("-linkmode launch -linkname 'math -mathlink'");
  ml.MLPutFunction("ToExpression", 1);
  ml.MLPutString(math_source());  
  ml.MLEndPacket();
  ml.skip_pkt_until(RETURNPKT);
  ml.MLNewPacket();

  ParseTreeGenerator<DefaultNodeFactory> ptg;
//  boost::shared_ptr<DefaultNodeFactory> nf(new DefaultNodeFactory());  から変えてある
  HydLaAST ast;

#define SS_CONSISTEMCY_CHECKER_TEST(EXP)                          \
  ast.parse_string(EXP);                                          \
  ptg.generate(ast.get_tree_iterator());                          \
  TellCollector tc(ms, false);                                    \
  tells_t tells;                                                  \
  expanded_always_t expanded_always;                              \
  ask_set_t positive_asks;                                  \
  tc.collect_all_tells(&tells, &expanded_always, &positive_asks); \
  hydla::simulator::symbolic::ConsistencyChecker cc(ml);           \
  ConstraintStoreBuilderPoint csbp;                               \
  csbp.build_constraint_store();                                  \

//  元々はSS_CONSISTEMCY_CHECKER_TESTの2行目あたりに以下の記述があったが変えてある
//  module_set_sptr ms(                                           
//    new ModuleSet("a", ast.parse_tree().get_tree()));           

#define SS_CONSISTEMCY_CHECKER_TEST_T(EXP)              \
  {                                                     \
    SS_CONSISTEMCY_CHECKER_TEST(EXP);                   \
    BOOST_CHECK_MESSAGE(cc.is_consistent(tells, csbp.getcs()), EXP);  \
  }

#define SS_CONSISTEMCY_CHECKER_TEST_F(EXP)              \
  {                                                     \
    SS_CONSISTEMCY_CHECKER_TEST(EXP);                   \
    BOOST_CHECK_MESSAGE(!cc.is_consistent(tells, csbp.getcs()), EXP); \
  }

  SS_CONSISTEMCY_CHECKER_TEST_T("x=1.");
  SS_CONSISTEMCY_CHECKER_TEST_F("x=1 & x=2.");

}

#endif // DISABLE_CONSISTENCY_CHECKER_TEST

/**
 * ConsistencyChecker‚ÌƒeƒXƒg
 */
/*
#include <boost/test/auto_unit_test.hpp>
#include <boost/shared_ptr.hpp>

#include "math_source.h"
#include "simulator/TellCollector.h"

#include "constraint_hierarchy/ModuleSet.h"

#include "symbolic_simulator/ConsistencyChecker.h"
#include "symbolic_simulator/mathlink_helper.h"
#include "parser/HydLaParser.h"
#include "simulator/TellCollector.h"

using namespace hydla;
using namespace hydla::parser;
using namespace hydla::parse_tree;
using namespace hydla::simulator;
using namespace hydla::symbolic_simulator;
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

  boost::shared_ptr<NodeFactory> nf(new NodeFactory());
  HydLaParser hp(nf);

#define SS_CONSISTEMCY_CHECKER_TEST(EXP)                          \
  hp.parse_string(EXP);                                           \
  boost::shared_ptr<ModuleSet> ms(                                \
    new ModuleSet("a", hp.parse_tree().get_tree()));              \
  TellCollector tc(ms);                                           \
  TellCollector::tells_t tells;                                   \
  expanded_always_t expanded_always;                              \
  positive_asks_t positive_asks;                                  \
  tc.collect_all_tells(&tells, &expanded_always, &positive_asks); \
  hydla::symbolic_simulator::ConsistencyChecker cc(ml);           \


#define SS_CONSISTEMCY_CHECKER_TEST_T(EXP)              \
  {                                                     \
    SS_CONSISTEMCY_CHECKER_TEST(EXP);                   \
    BOOST_CHECK_MESSAGE(cc.is_consistent(tells), EXP);  \
  }

#define SS_CONSISTEMCY_CHECKER_TEST_F(EXP)              \
  {                                                     \
    SS_CONSISTEMCY_CHECKER_TEST(EXP);                   \
    BOOST_CHECK_MESSAGE(!cc.is_consistent(tells), EXP); \
  }

  SS_CONSISTEMCY_CHECKER_TEST_T("x=1.");
  SS_CONSISTEMCY_CHECKER_TEST_F("x=1 & x=2.");

}
*/
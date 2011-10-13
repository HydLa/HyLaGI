
#include <boost/shared_ptr.hpp>

#include "ModuleSetList.h"
#include "ProgramOptions.h"
#include "BPSimulator.h"

using namespace hydla;
using namespace hydla::ch;
using namespace hydla::bp_simulator;

/**
 * RealPaverを使用したBranch and Pruneによるシミュレーション
 */
void branch_and_prune_simulate(boost::shared_ptr<hydla::parse_tree::ParseTree> parse_tree)
{
  ProgramOptions &po = ProgramOptions::instance();
  BPSimulator::Opts bpopts;

  bpopts.mathlink      = po.get<std::string>("mathlink");
  bpopts.debug_mode    = po.count("debug") > 0;
  bpopts.max_time      = po.get<std::string>("time");
  bpopts.profile_mode  = po.count("profile") > 0;
  bpopts.parallel_mode = po.count("parallel")> 0;

  BPSimulator bps(bpopts);
  bps.initialize(parse_tree);
  bps.simulate();
}

#include <iostream>

#include "ProgramOptions.h"
#include "SymbolicLegacySimulator.h"

using namespace hydla;
using namespace hydla::parse_tree;
using namespace hydla::symbolic_legacy_simulator;

/**
 * 記号処理によるシミュレーション
 */
void setup_symbolic_legacy_simulator_opts(
  SymbolicLegacySimulator::Opts& opts)
{  
  ProgramOptions &po = ProgramOptions::instance();

  if(po.get<std::string>("output-format") == "t") {
    opts.output_format = SymbolicLegacySimulator::fmtTFunction;
  } else if(po.get<std::string>("output-format") == "n"){
    opts.output_format = SymbolicLegacySimulator::fmtNumeric; 
  } else {
    // TODO: 例外を投げるようにする
    std::cerr << "invalid option - output format" << std::endl;
    exit(-1);
  }

  opts.mathlink         = po.get<std::string>("mathlink");
  opts.debug_mode       = po.count("debug")>0;
  opts.max_time         = po.get<std::string>("time");
  opts.profile_mode     = po.count("profile")>0;
  opts.parallel_mode    = po.count("parallel")>0;
  opts.output_interval  = po.get<std::string>("output-interval");
  opts.output_precision = po.get<int>("output-precision");
  opts.approx_precision = po.get<int>("approx");
}

void symbolic_legacy_simulate(
  boost::shared_ptr<hydla::parse_tree::ParseTree> parse_tree)
{
  SymbolicLegacySimulator::Opts opts;
  setup_symbolic_legacy_simulator_opts(opts);

  SymbolicLegacySimulator ms(opts);
  ms.initialize(parse_tree);
  ms.simulate();
}


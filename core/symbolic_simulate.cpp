#include "ModuleSetList.h"

#include "ProgramOptions.h"
#include "MathSimulator.h"


using namespace hydla;
using namespace hydla::parse_tree;
using namespace hydla::ch;
using namespace hydla::symbolic_simulator;

/**
 * 記号処理によるシミュレーション
 */
void setup_symbolic_simulator_opts(MathSimulator::Opts& opts)
{  
  ProgramOptions &po = ProgramOptions::instance();

  if(po.get<std::string>("output-format") == "t") {
    opts.output_format = MathSimulator::fmtTFunction;
  } else if(po.get<std::string>("output-format") == "n"){
    opts.output_format = MathSimulator::fmtNumeric; 
  } else {
    // TODO: 例外を投げるようにする
    std::cerr << "invalid option - output format" << std::endl;
    exit(-1);
  }

  opts.mathlink      = po.get<std::string>("mathlink");
  opts.debug_mode    = po.count("debug")>0;
  opts.max_time      = po.get<std::string>("simulation-time");
  opts.profile_mode  = po.count("profile")>0;
  opts.parallel_mode = po.count("parallel")>0;
  opts.output_interval = po.get<std::string>("output-interval");
  opts.output_precision = po.get<int>("output-precision");

}

void symbolic_simulate(boost::shared_ptr<hydla::parse_tree::ParseTree> parse_tree)
{
  MathSimulator::Opts opts;
  setup_symbolic_simulator_opts(opts);

  MathSimulator ms(opts);
  ms.initialize(parse_tree);
  ms.simulate();
}


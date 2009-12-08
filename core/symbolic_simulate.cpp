#include "ModuleSetList.h"

#include "ProgramOptions.h"
#include "MathSimulator.h"


using namespace hydla;
using namespace hydla::parse_tree;
using namespace hydla::ch;
using namespace hydla::symbolic_simulator;

/**
 * �L�������ɂ��V�~�����[�V����
 */
void symbolic_simulate(boost::shared_ptr<hydla::ch::ModuleSetContainer> msc,
                       boost::shared_ptr<hydla::parse_tree::ParseTree> pt) 
{
  ProgramOptions &po = ProgramOptions::instance();

  MathSimulator::Opts msopts;

  if(po.get<std::string>("output-format") == "t") {
    msopts.output_format = MathSimulator::fmtTFunction;
  } else if(po.get<std::string>("output-format") == "n"){
    msopts.output_format = MathSimulator::fmtNumeric; 
  } else {
    std::cerr << "invalid option - output format" << std::endl;
    return;
  }

  msopts.mathlink      = po.get<std::string>("mathlink");
  msopts.debug_mode    = po.count("debug")>0;
  msopts.max_time      = po.get<std::string>("simulation-time");
  msopts.profile_mode  = po.count("profile")>0;
  msopts.parallel_mode = po.count("parallel")>0;

  MathSimulator ms;
  ms.simulate(msc, pt, msopts);
}


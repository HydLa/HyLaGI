#include <iostream>
#include <exception>
#include <string>
#include <vector>

#ifdef _MSC_VER
#include <windows.h>
#endif

#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>

// core
#include "version.h"
#include "ProgramOptions.h"

// parser
#include "HydLaParser.h"
#include "ModuleSetList.h"
#include "ModuleSetContainerCreator.h"

// symbolic_simulator
#include "SSNodeFactory.h"
#include "MathSimulator.h"
#include "mathlink_helper.h"

using namespace hydla;
using namespace hydla::parse_tree;
using namespace hydla::symbolic_simulator;
using namespace hydla::ch;
using namespace boost;

void hydla_main(int argc, char* argv[])
{
  ProgramOptions &po = ProgramOptions::instance();
  po.parse(argc, argv);

  if(po.count("help")) {
    po.help_msg(std::cout);
    return;
  }

  if(po.count("version")) {
    std::cout << Version::description() << std::endl;
    return;
  }

  bool debug_mode = po.count("debug")>0;

  // パースツリーの構築
  shared_ptr<NodeFactory> nf(new NodeFactory());
  HydLaParser hp(nf, debug_mode);
  if(po.count("input-file")) {
    hp.parse_flie(po.get<std::string>("input-file"));
  } else {
    hp.parse(std::cin);
  }

  // 解候補モジュール集合の導出
  boost::shared_ptr<ModuleSetList> msl(
    ModuleSetContainerCreator<ModuleSetList>().
    create_module_set_container(&hp.parse_tree()));

  if(debug_mode) {
    std::cout << "#*** set of module sets which might be solution ***\n"
//              << *msl << std::endl;
              << msl->get_name() << "\n\n" << msl->get_tree_dump() << std::endl;
  }

  if(po.count("module-set-list")>0) {
//    std::cout << *msl << std::endl;
    std::cout << msl->get_name() << "\n\n" << msl->get_tree_dump() << std::endl;
    return;
  }

  // シミュレーション開始
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
  msopts.debug_mode    = debug_mode;
  msopts.max_time      = po.get<std::string>("simulation-time");
  msopts.profile_mode  = po.count("profile")>0;
  msopts.parallel_mode = po.count("parallel")>0;

  MathSimulator ms;
  ms.simulate(hp, msl, msopts);
}

int main(int argc, char* argv[]) 
{
#ifdef _MSC_VER
  _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

  int ret = 0;

  try {
    hydla_main(argc, argv);
  }
  catch(std::exception &e) {
    std::cerr << "error : " << e.what() << std::endl;
    ret = -1;
  } 
#if !(defined(_MSC_VER) && defined(_DEBUG))
  catch(...) {
    std::cerr << "fatal error!!" << std::endl;
    ret = -1;
  }
#else
    system("pause");
#endif

  return ret;
}

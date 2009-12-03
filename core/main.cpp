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
#include "HydLaAST.h"
#include "NodeFactory.h"
#include "ParseTreeGenerator.h"
#include "ModuleSetList.h"
#include "ModuleSetContainerCreator.h"

// symbolic_simulator
#include "MathSimulator.h"
#include "mathlink_helper.h"

// namespace
using namespace hydla;
using namespace hydla::parser;
using namespace hydla::parse_tree;
using namespace hydla::symbolic_simulator;
using namespace hydla::ch;
using namespace boost;


// typedef
//typedef boost::shared_ptr<hydla::ch::ModuleSetContainer> module_set_container_sptr;


// prototype declaration
int main(int argc, char* argv[]);
void hydla_main(int argc, char* argv[]);
void symbolic_simulate(boost::shared_ptr<hydla::ch::ModuleSetContainer> msc);
void branch_and_prune_simulate(boost::shared_ptr<hydla::ch::ModuleSetContainer> msc);


//
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
  HydLaAST ast;
  if(po.count("input-file")) {
    ast.parse_flie(po.get<std::string>("input-file"));
  } else {
    ast.parse(std::cin);
  }
  if(debug_mode) {
    std::cout << ast << std::endl;
  }

  boost::shared_ptr<ParseTree> pt(
    ParseTreeGenerator<DefaultNodeFactory>().generate(
      ast.get_tree_iterator()));
  if(debug_mode) {
    std::cout << *pt << std::endl;
  }

  // 解候補モジュール集合の導出
/*
  boost::shared_ptr<ModuleSetList> msl(
    ModuleSetContainerCreator<ModuleSetList>().
    create_module_set_container(&hp.parse_tree()));
  if(debug_mode) {
    std::cout << "#*** set of module sets which might be solution ***\n"
              << msl->get_name() << "\n\n" << msl->get_tree_dump() << std::endl;
  }  
  if(po.count("module-set-list")>0) {
    std::cout << msl->get_name() << "\n\n" << msl->get_tree_dump() << std::endl;
    return;
  }
  
  // シミュレーション開始
  std::string method(po.get<std::string>("method"));
  if(method == "s" || method == "SymbolicSimulator") {
    symbolic_simulate(msl);
  } 
  else if(method == "b" || method == "BandPSimulator") {
    branch_and_prune_simulate(msl);
  } else {
    std::cerr << "invalid method" << std::endl;
    return;
  }
*/
}


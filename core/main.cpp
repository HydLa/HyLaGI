#include <iostream>
#include <exception>
#include <string>
#include <vector>
#include <fstream>

#ifdef _MSC_VER
#include <windows.h>
#endif

#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>

// core
#include "version.h"
#include "ProgramOptions.h"

//common
#include "Logger.h"

// parser
#include "DefaultNodeFactory.h"

// symbolic_simulator
#include "MathSimulator.h"
#include "mathlink_helper.h"

// namespace
using namespace hydla;
using namespace hydla::logger;
using namespace hydla::parser;
using namespace hydla::parse_tree;
using namespace hydla::simulator;
using namespace hydla::symbolic_simulator;
using namespace hydla::ch;
using namespace boost;


// typedef
//typedef boost::shared_ptr<hydla::ch::ModuleSetContainer> module_set_container_sptr;


// prototype declaration
int main(int argc, char* argv[]);
void hydla_main(int argc, char* argv[]);
void symbolic_simulate(boost::shared_ptr<hydla::parse_tree::ParseTree> parse_tree);
void branch_and_prune_simulate(boost::shared_ptr<hydla::parse_tree::ParseTree> parse_tree);
void setup_symbolic_simulator_opts(MathSimulator::Opts& opts);

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

  if(po.count("debug")>0) {
    Logger::instance().set_log_level(Logger::Debug);
  }
  else {
    Logger::instance().set_log_level(Logger::None);
  }

  if(po.count("help")) {
    po.help_msg(std::cout);
    return;
  }

  if(po.count("version")) {
    std::cout << Version::description() << std::endl;
    return;
  }

  // ParseTreeの構築
  boost::shared_ptr<ParseTree> pt(new ParseTree);
  if(po.count("input-file")) {
    std::string filename(po.get<std::string>("input-file"));
    std::ifstream in(filename.c_str());
    if (!in) {    
      throw std::runtime_error(std::string("cannot open \"") + filename + "\"");
    }
    pt->parse<DefaultNodeFactory>(in);
  } else {
    pt->parse<DefaultNodeFactory>(std::cin);
  }

  
  // シミュレーション開始
  std::string method(po.get<std::string>("method"));
  if(method == "s" || method == "SymbolicSimulator") {
    symbolic_simulate(pt);
  } 
  else if(method == "b" || method == "BandPSimulator") {
    branch_and_prune_simulate(pt);
  } 
  else {
    // TODO: 例外を投げるようにする
    std::cerr << "invalid method" << std::endl;
    return;
  }
}


/*
  if(po.count("module-set-list")>0) {
    ModuleSetContainerCreator<ModuleSetList> mcc;
    boost::shared_ptr<ModuleSetList> msc(mcc.create(pt));
    msc->dump_node_names(std::cout);
    return;
  }

  if(po.count("module-set-list-noinit")>0) {
    ModuleSetContainerCreator<ModuleSetList> mcc;
    boost::shared_ptr<ModuleSetList> msc(mcc.create(pt_no_init_node));
    msc->dump_node_names(std::cout);
    return;
  }

  if(po.count("module-set-graph")>0) {
    ModuleSetContainerCreator<ModuleSetGraph> mcc;
    boost::shared_ptr<ModuleSetGraph> msc(mcc.create(pt));
    msc->dump_graphviz(std::cout);
    return;
  }

  if(po.count("module-set-graph-noinit")>0) {
    ModuleSetContainerCreator<ModuleSetGraph> mcc;
    boost::shared_ptr<ModuleSetGraph> msc(mcc.create(pt_no_init_node));
    msc->dump_graphviz(std::cout);
    return;
  }
*/
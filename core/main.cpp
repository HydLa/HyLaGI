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
#include "ModuleSetGraph.h"
#include "ModuleSetContainerCreator.h"
#include "InitNodeRemover.h"

#include "AskDisjunctionFormatter.h"

// symbolic_simulator
#include "MathSimulator.h"
#include "mathlink_helper.h"

// namespace
using namespace hydla;
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
void symbolic_simulate(boost::shared_ptr<hydla::parse_tree::ParseTree> parse_tree,
                       boost::shared_ptr<hydla::ch::ModuleSetContainer> msc, 
                       boost::shared_ptr<hydla::ch::ModuleSetContainer> msc_no_init);
void branch_and_prune_simulate(boost::shared_ptr<hydla::parse_tree::ParseTree> parse_tree,
                               boost::shared_ptr<hydla::ch::ModuleSetContainer> msc, 
                               boost::shared_ptr<hydla::ch::ModuleSetContainer> msc_no_init);
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

  if(po.count("help")) {
    po.help_msg(std::cout);
    return;
  }

  if(po.count("version")) {
    std::cout << Version::description() << std::endl;
    return;
  }

  bool debug_mode = po.count("debug")>0;

  // ASTの構築
  HydLaAST ast;
  if(po.count("input-file")) {
    ast.parse_flie(po.get<std::string>("input-file"));
  } else {
    ast.parse(std::cin);
  }
  if(debug_mode) {
    std::cout << "#*** AST Tree ***\n"
              << ast << std::endl;
  }

  // ParseTreeの構築
  boost::shared_ptr<ParseTree> pt(
    ParseTreeGenerator<DefaultNodeFactory>().generate(
      ast.get_tree_iterator()));
  if(debug_mode) {
    std::cout << "#*** Analyzed Parse Tree ***\n"
              << *pt << std::endl;
  }

  /*
  AskDisjunctionFormatter<DefaultNodeFactory> adf;
  adf.format(pt);
  std::cout << "#*** Format Ask Disjunction ***\n"
            << *pt << std::endl;
  return;
  */

  // alwaysが付いていない制約を取り除いたパースツリーの構築
  boost::shared_ptr<ParseTree> pt_no_init_node(new hydla::parse_tree::ParseTree(*pt));
  hydla::simulator::InitNodeRemover init_node_remover;
  init_node_remover.apply(pt_no_init_node.get());
  if(debug_mode) {
    std::cout << "#*** No Initial Node Tree ***\n"
              << *pt_no_init_node << std::endl;
  }

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

  // 解候補モジュール集合の導出
  boost::shared_ptr<ModuleSetContainer> msc;
  boost::shared_ptr<ModuleSetContainer> msc_no_init;
  if(po.count("nd")>0) {    
    ModuleSetContainerCreator<ModuleSetGraph> mcc;
    msc         = mcc.create(pt);
    msc_no_init = mcc.create(pt_no_init_node);
  }
  else {    
    ModuleSetContainerCreator<ModuleSetList> mcc;
    msc         = mcc.create(pt);
    msc_no_init = mcc.create(pt_no_init_node);
  }
  if(debug_mode) {
    std::cout << "#*** set of module sets ***\n"
              << *msc 
              << std::endl;

    std::cout << "#*** set of no init module sets ***\n"
              << *msc_no_init 
              << std::endl;
  }
  
  // シミュレーション開始
  std::string method(po.get<std::string>("method"));
  if(method == "s" || method == "SymbolicSimulator") {
    symbolic_simulate(pt, msc, msc_no_init);
  } 
  else if(method == "b" || method == "BandPSimulator") {
    branch_and_prune_simulate(pt, msc, msc_no_init);
  } else {
    // TODO: 例外を投げるようにする
    std::cerr << "invalid method" << std::endl;
    return;
  }
}

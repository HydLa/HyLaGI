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

// common
#include "Logger.h"

// constraint hierarchy
#include "ModuleSetContainerCreator.h"
#include "ModuleSetList.h"
#include "ModuleSetGraph.h"

// parser
#include "DefaultNodeFactory.h"

// namespace
using namespace boost;
using namespace hydla;
using namespace hydla::logger;
using namespace hydla::parser;
using namespace hydla::parse_tree;
using namespace hydla::ch;

// prototype declarations
int main(int argc, char* argv[]);
void hydla_main(int argc, char* argv[]);
void symbolic_simulate(boost::shared_ptr<hydla::parse_tree::ParseTree> parse_tree);
void symbolic_legacy_simulate(boost::shared_ptr<hydla::parse_tree::ParseTree> parse_tree);
void branch_and_prune_simulate(boost::shared_ptr<hydla::parse_tree::ParseTree> parse_tree);
bool dump(boost::shared_ptr<ParseTree> pt);

/**
 * エントリポイント
 */
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
  
  std::string area_string(po.get<std::string>("debug"));
  if(area_string!=""){                 // デバッグ出力の範囲指定
    Logger::instance().set_log_level(Logger::Area);
    Logger::parsing_area_ = (area_string.find('p') != std::string::npos);
    Logger::calculate_closure_area_ = (area_string.find('c') != std::string::npos);
    Logger::module_set_area_ = (area_string.find('m') != std::string::npos);
    Logger::vcs_area_ = (area_string.find('v') != std::string::npos);
    Logger::external_area_ = (area_string.find('e') != std::string::npos);
    Logger::output_area_ = (area_string.find('o') != std::string::npos);
    Logger::rest_area_ = (area_string.find('r') != std::string::npos);
  }else {                              // 警告のみ出力
    Logger::instance().set_log_level(Logger::Warn);
  }
  
  if(po.count("help")) {     // ヘルプ表示して終了
    po.help_msg(std::cout);
    return;
  }

  if(po.count("version")) {  // バージョン表示して終了
    std::cout << Version::description() << std::endl;
    return;
  }

  // ParseTreeの構築
  // ファイルがを指定されたらファイルから
  // そうでなければ標準入力から受け取る
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
  
  // いろいろと表示
  if(dump(pt)) {
    return;
  }
  
  // シミュレーション開始
  std::string method(po.get<std::string>("method"));
  if(method == "s" || method == "SymbolicSimulator") {
    symbolic_simulate(pt);
  } 
  else if(method == "b" || method == "BandPSimulator") {
    branch_and_prune_simulate(pt);
  } 
  else if(method == "l" || method == "SymbolicLegacySimulator") {
    symbolic_legacy_simulate(pt);
  } 
  else {
    // TODO: 例外を投げるようにする
    std::cerr << "invalid method" << std::endl;
    return;
  }
}

/**
 * ProgramOptionとParseTreeを元に出力
 * 何か出力したらtrueを返す
 */
bool dump(boost::shared_ptr<ParseTree> pt)
{
  ProgramOptions &po = ProgramOptions::instance();

  if(po.count("dump-parse-tree")>0) {
    pt->to_graphviz(std::cout);
    return true;
  }    

  if(po.count("dump-module-set-list")>0) {
    ModuleSetContainerCreator<ModuleSetList> mcc;
    boost::shared_ptr<ModuleSetList> msc(mcc.create(pt));
    msc->dump_node_names(std::cout);
    return true;
  }

//   if(po.count("dump-module-set-list-noinit")>0) {
//     ModuleSetContainerCreator<ModuleSetList> mcc;
//     boost::shared_ptr<ModuleSetList> msc(mcc.create(pt_no_init_node));
//     msc->dump_node_names(std::cout);
//     return true;
//   }

  if(po.count("dump-module-set-graph")>0) {
    ModuleSetContainerCreator<ModuleSetGraph> mcc;
    boost::shared_ptr<ModuleSetGraph> msc(mcc.create(pt));
    msc->dump_graphviz(std::cout);
    return true;
  }

//   if(po.count("dump-module-set-graph-noinit")>0) {
//     ModuleSetContainerCreator<ModuleSetGraph> mcc;
//     boost::shared_ptr<ModuleSetGraph> msc(mcc.create(pt_no_init_node));
//     msc->dump_graphviz(std::cout);
//     return true;
//   }

  return false;
}

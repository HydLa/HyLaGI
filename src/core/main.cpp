#include <iostream>
#include <exception>
#include <string>
#include <fstream>

#ifdef _MSC_VER
#include <windows.h>
#endif

// core
#include "version.h"
#include "ProgramOptions.h"

// common
#include "Timer.h"

// constraint hierarchy
#include "ModuleSetContainerCreator.h"

#include "SequentialSimulator.h"
#include "SignalHandler.h"

// namespace
using namespace boost;
using namespace hydla;
using namespace hydla::logger;
using namespace hydla::timer;
using namespace hydla::parser;
using namespace hydla::symbolic_expression;
using namespace hydla::parse_tree;
using namespace hydla::hierarchy;
using namespace std;

// prototype declarations
int main(int argc, char* argv[]);
void hydla_main(int argc, char* argv[]);
void simulate(boost::shared_ptr<parse_tree::ParseTree> parse_tree);
bool dump(boost::shared_ptr<ParseTree> pt);
void output_result(simulator::SequentialSimulator& ss, Opts& opts);

extern simulator::SequentialSimulator* simulator_;
extern Opts opts;

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
  
  signal(SIGINT, signal_handler::interrupt_handler);
  signal(SIGTERM, signal_handler::term_handler);
  
  Timer main_timer;
  
  if(po.count("debug")){                 // デバッグ出力
    Logger::instance().set_log_level(Logger::Debug);
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
  // ファイルを指定されたらファイルから
  // そうでなければ標準入力から受け取る
  boost::shared_ptr<ParseTree> pt(new ParseTree);
  if(po.count("input-file")) {
    std::string filename(po.get<std::string>("input-file"));
    std::ifstream in(filename.c_str());
    if (!in) {
      throw std::runtime_error(std::string("cannot open \"") + filename + "\"");
    }
    pt->parse(in);
  } else {
    pt->parse(std::cin);
  }

  if(po.count("parse_only"))
  {
    std::cout << "successfully parsed" << std::endl;
    return;
  }
  
  // いろいろと表示
  if(dump(pt)) {
    return;
  }

  // シミュレーション開始
  simulate(pt);

  if(po.get<std::string>("tm") != "n"){
    main_timer.elapsed("Finish Time");
    std::cout << std::endl;
  }

}

/**
 * ProgramOptionとParseTreeを元に出力
 * 何か出力したらtrueを返す
 */
bool dump(boost::shared_ptr<ParseTree> pt)
{
  ProgramOptions &po = ProgramOptions::instance();

  if(po.count("dump_parse_tree")>0) {
    pt->to_graphviz(std::cout);
    return true;
  }

/*
  TODO: implement
  if(po.count("dump_module_set_graph")>0) {
    ModuleSetContainerCreator<IncrementalModuleSet> mcc;
    boost::shared_ptr<IncrementalModuleSet> msc(mcc.create(pt));
    msc->dump_graphviz(std::cout);
    return true;
  }
*/

  return false;
}

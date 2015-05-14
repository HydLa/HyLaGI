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
#include "IncrementalModuleSet.h"

#include "SequentialSimulator.h"
#include "Logger.h"
#include "SignalHandler.h"
#include "Utility.h"

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
int hydla_main(int argc, char* argv[]);
int simulate(boost::shared_ptr<parse_tree::ParseTree> parse_tree);
bool dump(boost::shared_ptr<ParseTree> pt);
void output_result(simulator::SequentialSimulator& ss, Opts& opts);
void setup_simulator_opts(Opts& opts, ProgramOptions& p, bool use_default);

extern ProgramOptions options;
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

  return hydla_main(argc, argv);
}


int hydla_main(int argc, char* argv[])
{
  options.parse(argc, argv);
  
  signal(SIGINT, signal_handler::interrupt_handler);
  signal(SIGTERM, signal_handler::term_handler);
  
  Timer main_timer;
  
  if(options.count("debug")){                 // デバッグ出力
    Logger::instance().set_log_level(Logger::Debug);
  }else {                              // 警告のみ出力
    Logger::instance().set_log_level(Logger::Warn);
  }
  
  
  if(options.count("help")) {     // ヘルプ表示して終了
    options.help_msg(cout);
    return 0;
  }

  if(options.count("version")) {  // バージョン表示して終了
    cout << Version::description() << endl;
    return 0;
  }
  // ParseTreeの構築
  // ファイルを指定されたらファイルから
  // そうでなければ標準入力から受け取る
  boost::shared_ptr<ParseTree> pt(new ParseTree);
  string input;
  if(options.count("input-file")) {
    string filename(options.get<string>("input-file"));
    ifstream in(filename.c_str());
    if (!in) {
      throw runtime_error(string("cannot open \"") + filename + "\"");
    }
    istreambuf_iterator<char> it(in), last;
    input = string(it, last);
  } else {
    istreambuf_iterator<char> it(cin), last;
    input = string(it, last);
  }
  string comment = utility::remove_comment(input);
  ProgramOptions options_in_source;
  const string option_header = "#hylagi";
  string option_string;
  string::size_type option_pos = comment.find(option_header);
  if(option_pos != string::npos)
  {
    option_pos += option_header.length();
    string::size_type pos = comment.find('\n', option_pos + 1);
    option_string = comment.substr(option_pos, pos==string::npos?string::npos:pos - option_pos - 1);
  }
  options_in_source.parse(option_string);
  setup_simulator_opts(opts, options_in_source, true);
  pt->parse_string(input);


  if(options.count("parse_only"))
  {
    cout << "successfully parsed" << endl;
    return 0;
  }
  
  if(dump(pt)) {
    return 0;
  }

  Timer simulation_timer;
  // シミュレーション開始
  int simulation_result = simulate(pt);

  if(options.get<string>("tm") != "n"){
    std::cout << "Simulation Time : " << simulation_timer.get_time_string() << std::endl;
    std::cout << "Finish Time : " << main_timer.get_time_string() << std::endl;
    cout << endl;
  }
  return simulation_result;
}

/**
 * ProgramOptionとParseTreeを元に出力
 * 何か出力したらtrueを返す
 */
bool dump(boost::shared_ptr<ParseTree> pt)
{

  if(options.count("dump_parse_tree")>0) {
    pt->to_graphviz(cout);
    return true;
  }

  if(options.count("dump_module_set_graph")>0) {
    ModuleSetContainerCreator<IncrementalModuleSet> mcc;
    boost::shared_ptr<IncrementalModuleSet> msc(mcc.create(pt));
    msc->dump_module_sets_for_graphviz(cout);
    return true;
  }

  if(options.count("dump_module_priority_graph")>0) {
    ModuleSetContainerCreator<IncrementalModuleSet> mcc;
    boost::shared_ptr<IncrementalModuleSet> msc(mcc.create(pt));
    msc->dump_priority_data_for_graphviz(cout);
    return true;
  }

  return false;
}

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
#include "AffineTreeVisitor.h"
#include "Parser.h"

#include <boost/regex.hpp>

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
bool dump(boost::shared_ptr<ParseTree> pt, ProgramOptions& p);
bool dump_in_advance(ProgramOptions& p);
void output_result(simulator::SequentialSimulator& ss, Opts& opts);
void add_vars_from_string(string var_string, set<string> &set_to_add, string warning_prefix);
bool process_opts(Opts& opts, ProgramOptions& p, bool use_default);
void add_vars_from_string(string vars_list_string, set<string> &set_to_add, string warning_prefix);

extern ProgramOptions cmdline_options;
extern simulator::SequentialSimulator* simulator_;
extern Opts opts;
extern string input_file_name;

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
  cmdline_options.parse(argc, argv);
  
  signal(SIGINT, signal_handler::interrupt_handler);
  signal(SIGTERM, signal_handler::term_handler);

  if(dump_in_advance(cmdline_options))return 0;
  
  Timer main_timer;

  // ParseTreeの構築
  // ファイルを指定されたらファイルから
  // そうでなければ標準入力から受け取る
  boost::shared_ptr<ParseTree> pt(new ParseTree);
  string input;
  if(cmdline_options.count("input-file")) {
    input_file_name = cmdline_options.get<string>("input-file");
    ifstream in(input_file_name.c_str());
    if (!in) {
      throw runtime_error(string("cannot open \"") + input_file_name + "\"");
    }
    input = string(std::istreambuf_iterator<char>(in), 
                   std::istreambuf_iterator<char>());
    input_file_name = utility::extract_file_name(input_file_name);
  } else {
    input_file_name = "unknown";
    input = string(std::istreambuf_iterator<char>(cin), 
                   std::istreambuf_iterator<char>());
  }
  input = utility::cr_to_lf(input);
  string comment = utility::remove_comment(input);
  ProgramOptions options_in_source;
  const string option_header = "#hylagi";
  string option_string;
  string::size_type option_pos = comment.find(option_header);
  if(option_pos != string::npos)
  {
    option_pos += option_header.length();
    string::size_type pos = comment.find('\n', option_pos + 1);
    option_string = comment.substr(option_pos, pos==string::npos?string::npos:pos - option_pos);
  }
  options_in_source.parse(option_string);
  if(dump_in_advance(options_in_source))return 0;
  process_opts(opts, options_in_source, true);

  // コメント中の変数省略指定を調べる
  opts.output_mode = Opts::None;

  bool isOmit = false;
  bool isOutput = false;
  const string omit_comment = "#omit ";
  const string output_comment = "#output ";
  // #omit を検索
  string::size_type output_pos = comment.find(omit_comment);
  if (output_pos != string::npos)
  {
    isOmit = true;
    opts.output_mode = Opts::Omit;
  }
  else 
  {
    // #output を検索
    output_pos = comment.find(output_comment);
    if (output_pos != string::npos)
    {
      isOutput = true;
      opts.output_mode = Opts::Output;
    }
  }
  if (isOmit || isOutput)
  {
    output_pos += (isOmit ? omit_comment.length() : output_comment.length());

    // 省略対象指定部を取り出す
    string::size_type pos = comment.find('\n', output_pos);
    string var_string = comment.substr(output_pos, pos == string::npos ? string::npos : pos - output_pos);

    add_vars_from_string(var_string, opts.output_vars, string("[") + (isOmit ? "#omit" : "#output") + "]");
  }

  // {
  //   Logger::instance().set_log_level(Logger::Debug);
  //   parser::Parser p(input);
  //   node_sptr expr = p.arithmetic();
  //   interval::parameter_idx_map_t map;
  //   simulator::variable_map_t vm;
  //   interval::AffineTreeVisitor visitor(map, vm);
  //   interval::AffineMixedValue affine = visitor.approximate(expr);
  //   cout << affine << endl;
  //   cout << affine.to_interval() << endl;
  //   cout << affine.to_affine() << endl;
  //   return 0;
  // }
  
  
  pt->parse_string(input);

  if(cmdline_options.count("parse_only") || options_in_source.count("parse_only"))
  {
    cout << "successfully parsed" << endl;
    return 0;
  }
  
  if(dump(pt, cmdline_options) || dump(pt, options_in_source)) {
    return 0;
  }

  Timer simulation_timer;
  // シミュレーション開始
  int simulation_result = simulate(pt);

  std::cout << "Simulation Time : " << simulation_timer.get_time_string() << std::endl;
  std::cout << "Finish Time : " << main_timer.get_time_string() << std::endl;
  cout << endl;
  
  return simulation_result;
}

/**
 * ProgramOptionとParseTreeを元に出力
 * 何か出力したらtrueを返す
 */
bool dump(boost::shared_ptr<ParseTree> pt, ProgramOptions& po)
{

  if(po.count("dump_parse_tree")>0) {
    pt->to_graphviz(cout);
    return true;
  }

  if(po.count("dump_module_set_graph")>0) {
    ModuleSetContainerCreator<IncrementalModuleSet> mcc;
    boost::shared_ptr<IncrementalModuleSet> msc(mcc.create(pt));
    msc->dump_module_sets_for_graphviz(cout);
    return true;
  }

  if(po.count("dump_module_priority_graph")>0) {
    ModuleSetContainerCreator<IncrementalModuleSet> mcc;
    boost::shared_ptr<IncrementalModuleSet> msc(mcc.create(pt));
    msc->dump_priority_data_for_graphviz(cout);
    return true;
  }

  return false;
}


bool dump_in_advance(ProgramOptions& po)
{
  if(po.count("help")) {     // ヘルプ表示して終了
    po.help_msg(cout);
    return true;
  }

  if(po.count("version")) {  // バージョン表示して終了
    cout << Version::description() << endl;
    return true;
  }
    

  return false;
}

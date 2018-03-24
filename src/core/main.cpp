#include <iostream>
#include <exception>
#include <string>
#include <fstream>
#include <regex>

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

// debug
#include "debug_main.h"


// namespace
using namespace boost;
using namespace hydla;
using namespace hydla::logger;
using namespace hydla::timer;
using namespace hydla::parser;
using namespace hydla::symbolic_expression;
using namespace hydla::parse_tree;
using namespace hydla::debug;
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

  //バックスラッシュ直後の改行の除去
  {
    const std::regex r("([^/]\\\\[ \t]*(\n|\r\n))");
    input = std::regex_replace(input, r, " ");
    input = std::regex_replace(input, std::regex("\n"), "\n\n");
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

  Logger::set_html_mode(opts.html);

  //マクロ処理
  {
    static const bool debugPrint = false;

    class MacroDefinition
    {
    public:

      MacroDefinition(const std::string& name, const std::string& argments, const std::string& definition)
        : m_name(name)
        , m_argments(splitArgments(argments))
        , m_definition(definition)
      {}

      void apply(std::string& input, std::string::size_type offset)const
      {
        if (debugPrint)
        {
          std::cout << "apply macro \"" << m_name << "\"\n";
          std::cout << "apply target {" << std::string(input.cbegin() + offset, input.cend()) << "}\n";
        }

        if (m_argments.empty())
        {
          for (string::size_type pos = input.find(m_name, offset); pos != string::npos; pos = input.find(m_name, pos))
          {
            {
              //識別子の前後が英数字だったら読み飛ばす
              if (0 < pos && isAlNum(input[pos - 1]))
              {
                pos += m_name.length();
                continue;
              }
              if (pos + m_name.length() < input.length() && isAlNum(input[pos + m_name.length()]))
              {
                pos += m_name.length();
                continue;
              }
            }

            input.replace(pos, m_name.length(), m_definition);
            pos += m_definition.length();

            if (debugPrint)
            {
              std::cout << "name : \"" << m_name << "\"\n";
              std::cout << "replaced :\n";
              std::cout << input;
            }
          }
        }
        else
        {
          //左括弧と右括弧が同じ数になるように対応づけなければならないので正規表現は使えない
          for (string::size_type pos = input.find(m_name, offset); pos != string::npos; pos = input.find(m_name, pos))
          {
            {
              //識別子の前後が英数字だったら読み飛ばす
              if (0 < pos && isAlNum(input[pos - 1]))
              {
                pos += m_name.length();
                continue;
              }
              if (pos + m_name.length() < input.length() && isAlNum(input[pos + m_name.length()]))
              {
                pos += m_name.length();
                continue;
              }
            }

            const string::size_type start = input.find_first_of('(', pos);
            if (start == string::npos)
            {
              std::cout << "ERROR\n";
              return;
            }

            string::size_type currentPos = start + 1;

            int nestDepth = 1;
            while (nestDepth != 0)
            {
              string::size_type next = input.find_first_of("()", currentPos);
              if (next == string::npos)
              {
                std::cout << "ERROR\n";
                return;
              }
              nestDepth += input[next] == '(' ? 1 : -1;
              currentPos = next + 1;
            }

            const auto argments = splitArgments(std::string(input.cbegin() + start, input.cbegin() + currentPos));
            if (argments.size() != m_argments.size())
            {
              pos = currentPos;
              continue;
            }

            const auto def = appliedDefinition(argments);
            input.replace(pos, currentPos - pos, def);
            pos += def.length();

            if (debugPrint)
            {
              std::cout << "argments :\n";
              for (const auto& arg : argments)
              {
                std::cout << arg << "\n";
              }

              std::cout << "replaced :\n";
              std::cout << input;
            }
          }
        }
      }

    private:

      static std::vector<std::string> splitArgments(const std::string& argments)
      {
        const auto left = argments.find_first_of('(');
        const auto right = argments.find_last_of(')');

        if (left == string::npos || right == string::npos || !(left + 1 < right))
        {
          return{};
        }

        std::vector<std::string> result;

        const std::string rawStr(argments.cbegin() + left + 1, argments.cbegin() + right);
        if (debugPrint)
        {
          std::cout << "split \"" << rawStr << "\"\n";
        }

        {
          string::size_type startPos = 0;
          string::size_type currentPos = startPos;

          int nestDepth = 0;
          while (true)
          {
            string::size_type next = rawStr.find_first_of("(),", currentPos);
            if (next == string::npos)
            {
              if (nestDepth == 0)
              {
                result.emplace_back(rawStr.cbegin() + startPos, rawStr.cend());
                if (debugPrint)
                {
                  std::cout << "    add \"" << result.back() << "\"\n";
                }
              }
              break;

            }
            if (rawStr[next] == ',' && nestDepth == 0)
            {
              result.emplace_back(rawStr.cbegin() + startPos, rawStr.cbegin() + next);
              if (debugPrint)
              {
                std::cout << "    add \"" << result.back() << "\"\n";
              }
              startPos = next + 1;
            }
            else if (rawStr[next] != ',')
            {
              nestDepth += rawStr[next] == '(' ? 1 : -1;
            }

            currentPos = next + 1;
          }
        }

        if (debugPrint)
        {
          std::cout << "    size : " << result.size() << "\n";
        }

        return result;

      }

      std::string appliedDefinition(const std::vector<std::string>& argments)const
      {
        if (m_argments.size() != argments.size())
        {
          return "";
        }

        std::string result = m_definition;

        for (size_t i = 0; i < m_argments.size(); ++i)
        {
          for (string::size_type pos = result.find(m_argments[i]); pos != string::npos; pos = result.find(m_argments[i], pos))
          {
            {
              //識別子の前後が英数字だったら読み飛ばす
              if (0 < pos && isAlNum(result[pos - 1]))
              {
                pos += m_argments[i].length();
                continue;
              }
              if (pos + m_argments[i].length() < result.length() && isAlNum(result[pos + m_argments[i].length()]))
              {
                pos += m_argments[i].length();
                continue;
              }
            }
            result.replace(pos, m_argments[i].length(), argments[i]);
            pos += argments[i].length();
          }
        }

        return result;
      }

      static bool isAlNum(char c)
      {
        return ('0' <= c && c <= '9') || ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z');
      }

      std::string m_name;
      std::vector<std::string> m_argments;
      std::string m_definition;
    };

    const string define_header = "#define";

    std::vector<MacroDefinition> macro_definitions;
    const std::regex reg("(#define\\s+)(\\w+)(\\((\\w+)(,(\\w+))*\\))?(.*)\n");

    for (string::size_type pos = comment.find(define_header); pos != string::npos; pos = comment.find(define_header, pos))
    {
      if (debugPrint)
      {
        std::cout << "=======================================\n";
      }

      std::smatch match;
      if (std::regex_search((comment.cbegin() + pos), comment.cend(), match, reg))
      {
        macro_definitions.emplace_back(match[2], match[3], match[7]);
        macro_definitions.back().apply(comment, pos + match.length());

        pos += 1;
      }
      else
      {
        if (debugPrint)
        {
          std::cout << "Macro couldn't parse : {\n";
          std::cout << std::string((comment.cbegin() + pos), comment.cend()) << "}\n";
        }
        pos += 1;
      }
    }

    for (const auto& macro : macro_definitions)
    {
      if (debugPrint)
      {
        std::cout << "===================================\n";
      }
      macro.apply(input, 0u);
    }

    input = std::regex_replace(input, std::regex("[^#]"), "$&", std::regex_constants::format_no_copy);

    if (debugPrint)
    {
      std::cout << "===================================\n";
      std::cout << "<- result\n";
      std::cout << input;
      std::cout << "result ->\n";
    }
  }
  
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
    HYDLA_LOGGER_STANDARD("successfully parsed");
    return 0;
  }

  if(cmdline_options.count("debug_constraint") || options_in_source.count("debug_constraint"))
  {
    cout << "debug constraint" << endl;
    Debug db;
    ModuleSetContainerCreator<IncrementalModuleSet> mcc;
    boost::shared_ptr<IncrementalModuleSet> msc(mcc.create(pt));
    Debug::dump_debug(&db, input, pt, msc);
    return 0;
  }
  
  if(dump(pt, cmdline_options) || dump(pt, options_in_source)) {
    return 0;
  }

  Timer simulation_timer;
  // シミュレーション開始
  int simulation_result = simulate(pt);

  HYDLA_LOGGER_STANDARD("Simulation Time : ", simulation_timer.get_time_string());
  HYDLA_LOGGER_STANDARD("Finish Time : ", main_timer.get_time_string());
  HYDLA_LOGGER_STANDARD("");

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

  if(po.count("dump_parse_tree_json")>0) {
    pt->dump_in_json(cout);
    return true;
  }

  if(po.count("debug_constraint")>0) {
    //dump_debug(cout);
    //cout << "hoge";
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

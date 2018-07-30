#include "ProgramOptions.h"
#include "PhaseSimulator.h"
#include "SequentialSimulator.h"
#include "SymbolicTrajPrinter.h"
#include "StdProfilePrinter.h"
#include "CsvProfilePrinter.h"
#include "LTLModelChecker.h"
#include "HybridAutomatonConverter.h"
#include "TreeInfixPrinter.h"
#include "MathematicaLink.h"
#include "Backend.h"
#include "JsonWriter.h"
#include "Timer.h"
#include "Utility.h"
#include "Parser.h"
#include <sys/stat.h>
#include <fstream>
#include <regex>

#include "version.h"
#include "Logger.h"
#include <boost/regex.hpp>
#include <regex>

#ifdef _MSC_VER
#include <windows.h>
#endif
// namespace
using namespace hydla;
using namespace hydla::symbolic_expression;
using namespace hydla::parser;
using namespace hydla::hierarchy;
using namespace hydla::simulator;
using namespace hydla::backend;
using namespace hydla::backend::mathematica;
using namespace hydla::io;
using namespace hydla::logger;
using namespace std;

Simulator* simulator_;
Opts opts;
backend_sptr_t backend_;
ProgramOptions cmdline_options;
string input_file_name;

static string get_file_without_ext(const string &path)
{
  size_t start_idx = path.find_last_of('/');
  if(start_idx == string::npos) start_idx = 0;
  else ++start_idx;
  size_t end_idx = path.find_last_of('.');
  if(end_idx != string::npos) end_idx = end_idx - start_idx;
  return path.substr(start_idx, end_idx);
}

void output_result(Simulator& ss, Opts& opts){
  auto detail = logger::Detail(__FUNCTION__);

  std::stringstream sstr;
  sstr << "------ Result of Simulation ------\n";
  HYDLA_LOGGER_DEBUG("%% output_result A");
  hydla::io::SymbolicTrajPrinter Printer(backend_, sstr, opts.interval);
  HYDLA_LOGGER_DEBUG("%% output_result B");
  if(opts.epsilon_mode >= 0){Printer.set_epsilon_mode(backend_,true);}

  HYDLA_LOGGER_DEBUG("%% output_result C1");
  parameter_map_t par_map = ss.get_parameter_map();
  HYDLA_LOGGER_DEBUG("%% output_result C2");
  if(!par_map.empty())
  {
    sstr << "---------parameter condition(global)---------" << endl;
	HYDLA_LOGGER_DEBUG("%% output_result D1");
    Printer.output_parameter_map(par_map);
	HYDLA_LOGGER_DEBUG("%% output_result D2");
  }
  HYDLA_LOGGER_DEBUG("%% output_result E");
  Printer.output_result_tree(ss.get_result_root());
  HYDLA_LOGGER_STANDARD(sstr.str());

  //todo : この std::cout << std::endl; を外すと、ログの出力が遅延して detail タグが先に閉じてしまう...
  std::cout << std::endl;

  std::string of_name = cmdline_options.get<string>("output_name");
  if(of_name.empty())
  {
    const std::string hydat_dir = "./hydat/";
    if(cmdline_options.count("input-file"))
    {
      std::string if_name = cmdline_options.get<string>("input-file");
      of_name = hydat_dir + get_file_without_ext(if_name) + ".hydat";
    }
    else
    {
      of_name = hydat_dir + "no_name.hydat";
    }
    struct stat st;
    int ret = stat(hydat_dir.c_str(), &st);
    if(ret == -1)
    {
      mkdir(hydat_dir.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    }
  }
  JsonWriter writer;
  writer.write(*simulator_, of_name, input_file_name);

  if(opts.epsilon_mode >= 0){
    writer.set_epsilon_mode(backend_, true);
    std::string of_name = cmdline_options.get<string>("output_name");
    if(of_name.empty())
    {
      const std::string hydat_dir = "./hydat/";
      if(cmdline_options.count("input-file"))
      {
        std::string if_name = cmdline_options.get<string>("input-file");
        of_name = hydat_dir + get_file_without_ext(if_name) + "_diff.hydat";
      }
      else
      {
        of_name = hydat_dir + "no_name_diff.hydat";
      }
      struct stat st;
      int ret = stat(hydat_dir.c_str(), &st);
      if(ret == -1)
      {
        mkdir(hydat_dir.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
      }
    }
    writer.write(*simulator_, of_name, input_file_name + "_diff");
  }

  if(cmdline_options.get<std::string>("tm") == "s") {
    hydla::io::StdProfilePrinter().print_profile(ss.get_profile());
  } else if(cmdline_options.get<std::string>("tm") == "c") {
    std::string csv_name = cmdline_options.get<std::string>("csv");
    if(csv_name == ""){
      hydla::io::CsvProfilePrinter().print_profile(ss.get_profile());
    }else{
      std::ofstream ofs;
      ofs.open(csv_name.c_str());
      hydla::io::CsvProfilePrinter(ofs).print_profile(ss.get_profile());
      ofs.close();
    }
  }
}


void trim_front_and_behind_space(std::string &buffer)
{
      // 前後の空白を取り除く
  auto left = buffer.find_first_not_of(" ");
  if (left != string::npos)
  {
    auto right = buffer.find_last_not_of(" ");
    buffer = buffer.substr(left, right - left + 1);
  }
}

void add_vars_from_string(string vars_list_string, set<string> &set_to_add, string warning_prefix)
{
  HYDLA_LOGGER_DEBUG_VAR(vars_list_string);
  std::stringstream sstr(vars_list_string);
  string buffer;
  while(std::getline(sstr, buffer, ','))
  {
    trim_front_and_behind_space(buffer);
    boost::regex re("^[[:lower:]][[:digit:][:lower:]]*'*$", std::regex_constants::extended);
    boost::smatch match;
    if (!boost::regex_search(buffer, match, re))
    {
      cout << warning_prefix << " warning : \"" << buffer << "\" is not a valid variable name." << endl;
    }
    set_to_add.insert(buffer);
  }
}


#define IF_SPECIFIED(X) if(use_default || !po.defaulted(X))

void process_opts(Opts& opts, ProgramOptions& po, bool use_default)
{
  opts.wstp      = "-linkmode launch -linkname '" + po.get<string>("math_name") + " -wstp'";
  if(po.count("debug"))
  {
    opts.debug_mode    = true;
  }
  IF_SPECIFIED("time")
  {
    parser::Parser parser(po.get<string>("time"));
    opts.max_time = parser.arithmetic();
  }
  IF_SPECIFIED("simplify_time")opts.simplify_time = po.get<string>("simplify_time");
  IF_SPECIFIED("phase")opts.max_phase      = po.get<int>("phase");
  IF_SPECIFIED("nd")opts.nd_mode       = po.count("nd") > 0 && po.get<char>("nd") == 'y';
  IF_SPECIFIED("static_generation_of_module_sets")  opts.static_generation_of_module_sets = po.count("static_generation_of_module_sets") && po.get<char>("static_generation_of_module_sets") == 'y';
  IF_SPECIFIED("dump_in_progress") opts.dump_in_progress = po.count("dump_in_progress")>0 && po.get<char>("dump_in_progress") == 'y';
  opts.dump_relation = po.count("dump_relation_graph")>0;
  IF_SPECIFIED("ignore_warnings")opts.ignore_warnings = po.count("ignore_warnings")>0 && po.get<char>("ignore_warnings") == 'y';
  IF_SPECIFIED("use_shorthand")TreeInfixPrinter::set_use_shorthand(po.count("use_shorthand") > 0 && po.get<char>("use_shorthand") == 'y');
  IF_SPECIFIED("ha")opts.ha_convert_mode = po.count("ha")>0 && po.get<char>("ha") == 'y';
  IF_SPECIFIED("hs")opts.ha_simulator_mode = po.count("hs")>0 && po.get<char>("hs") == 'y';
  IF_SPECIFIED("ltl")opts.ltl_model_check_mode = po.count("ltl")>0 && po.get<char>("ltl") == 'y';
  IF_SPECIFIED("affine")opts.affine = po.count("affine")>0 && po.get<char>("affine") == 'y';
  IF_SPECIFIED("epsilon")opts.epsilon_mode = po.get<int>("epsilon");
  IF_SPECIFIED("interval")opts.interval = po.count("interval") > 0 && po.get<char>("interval") == 'y';
  IF_SPECIFIED("numerize_without_validation")opts.numerize_mode =  po.count("numerize_without_validation") > 0 && po.get<char>("numerize_without_validation") == 'y';
  IF_SPECIFIED("eager_approximation")opts.eager_approximation =  po.count("eager_approximation") > 0 && po.get<char>("eager_approximation") == 'y';
  IF_SPECIFIED("fail_on_stop")opts.stop_at_failure = po.count("fail_on_stop") > 0 && po.get<char>("fail_on_stop") == 'y';
  IF_SPECIFIED("approximation_step")opts.approximation_step = po.get<int>("approximation_step");
  IF_SPECIFIED("extra_dummy")opts.extra_dummy_num = po.get<int>("extra_dummy_num");

  IF_SPECIFIED("vars_to_approximate")
  {
    add_vars_from_string(po.get<string>("vars_to_approximate"), opts.vars_to_approximate, "");
  }
  IF_SPECIFIED("guards_to_interval_newton")
  {
    std::stringstream sstr(po.get<string>("guards_to_interval_newton"));
    string buffer;
    while(std::getline(sstr, buffer, ','))
    {
      buffer = utility::replace(buffer, " ", "");
      opts.guards_to_interval_newton.insert(buffer);
    }
  }
  IF_SPECIFIED("step_by_step")opts.step_by_step = po.count("step_by_step") > 0 && po.get<char>("step_by_step") == 'y';
  IF_SPECIFIED("simplify")opts.simplify = po.get<int>("simplify");
  IF_SPECIFIED("solve_over_reals")opts.solve_over_reals = po.count("solve_over_reals") > 0 && po.get<char>("solve_over_reals") == 'y';
  IF_SPECIFIED("html")opts.html = po.count("html") > 0 && po.get<char>("html") == 'y';
}


int simulate(boost::shared_ptr<hydla::parse_tree::ParseTree> parse_tree)
{
  process_opts(opts, cmdline_options, false);

  // todo : コマンドラインオプションをここで読むのは遅すぎるので何とかする
  Logger::set_html_mode(opts.html);
  Logger::initialize();

  if(opts.debug_mode)    Logger::instance().set_log_level(Logger::Debug);
  else     Logger::instance().set_log_level(Logger::Warn);

  backend_.reset(new Backend(new MathematicaLink(opts.wstp, opts.ignore_warnings, opts.simplify_time, opts.simplify, opts.solve_over_reals)));
  PhaseResult::backend = backend_.get();

  if(opts.ltl_model_check_mode)
    {
      simulator_ = new LTLModelChecker(opts);
    }
  else if(opts.ha_convert_mode)
    {
      simulator_ = new HybridAutomatonConverter(opts);
    }
  else
    {
      simulator_ = new SequentialSimulator(opts);
    }

  simulator_->set_backend(backend_);
  simulator_->set_phase_simulator(new PhaseSimulator(simulator_, opts));
  simulator_->initialize(parse_tree);
  simulator_->simulate();
  HYDLA_LOGGER_DEBUG("%% output result");
  if(!opts.ha_convert_mode)
  {
    output_result(*simulator_, opts);
  }
  int simulation_status = simulator_->get_exit_status();
  delete simulator_;
  return simulation_status;
}

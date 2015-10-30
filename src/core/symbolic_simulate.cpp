#include "ProgramOptions.h"
#include "PhaseSimulator.h"
#include "SequentialSimulator.h"
#include "SymbolicTrajPrinter.h"
#include "StdProfilePrinter.h"
#include "CsvProfilePrinter.h"
#include "LTLModelChecker.h"
#include "MathematicaLink.h"
#include "Backend.h"
#include "JsonWriter.h"
#include "Timer.h"
#include <sys/stat.h>
#include <fstream>
#include <regex>

#include "version.h"
#include "Logger.h"
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
  std::stringstream sstr;

  hydla::io::SymbolicTrajPrinter Printer(backend_, sstr, opts.interval);
  if(opts.epsilon_mode >= 0){Printer.set_epsilon_mode(backend_,true);}

  parameter_map_t par_map = ss.get_parameter_map();
  if(!par_map.empty())
  {
    sstr << "---------parameter condition(global)---------" << endl;
    Printer.output_parameter_map(par_map);
  }
  Printer.output_result_tree(ss.get_result_root());
  std::cout << sstr.str();

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
  writer.write(*simulator_, of_name);

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
    writer.write(*simulator_, of_name);
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


void add_vars_from_string(string vars_list_string, set<string> &set_to_add, string warning_prefix)
{
  HYDLA_LOGGER_DEBUG_VAR(vars_list_string);
  std::stringstream sstr(vars_list_string);
  string buffer;
  while(std::getline(sstr, buffer, ','))
  {


    // 前後の空白を取り除く
    auto left = buffer.find_first_not_of(" ");
    if (left != string::npos)
    {
      auto right = buffer.find_last_not_of(" ");
      buffer = buffer.substr(left, right - left + 1);
    }
    // 変数名に使えない文字が入っていたら警告
    regex re("^[[:lower:]][[:digit:][:lower:]]*'*$", std::regex_constants::extended);
    smatch match;
    if (!regex_search(buffer, match, re))
    {
      cout << warning_prefix << " warning : \"" << buffer << "\" is not a valid variable name." << endl;
    }
      
    set_to_add.insert(buffer);
  }
}


#define IF_SPECIFIED(X) if(use_default || !po.defaulted(X))

void process_opts(Opts& opts, ProgramOptions& po, bool use_default)
{
  opts.mathlink      = "-linkmode launch -linkname '" + po.get<string>("math_name") + " -mathlink'";
  if(po.count("debug"))
  {
    opts.debug_mode    = true;
  }
  IF_SPECIFIED("time")opts.max_time      = po.get<string>("time");
  IF_SPECIFIED("phase")opts.max_phase      = po.get<int>("phase");
  IF_SPECIFIED("nd")opts.nd_mode       = po.count("nd") > 0 && po.get<char>("nd") == 'y';
  IF_SPECIFIED("static_generation_of_module_sets")  opts.static_generation_of_module_sets = po.count("static_generation_of_module_sets") && po.get<char>("static_generation_of_module_sets") == 'y';
  IF_SPECIFIED("dump_in_progress") opts.dump_in_progress = po.count("dump_in_progress")>0 && po.get<char>("dump_in_progress") == 'y';
  opts.dump_relation = po.count("dump_relation_graph")>0;
  IF_SPECIFIED("ignore_warnings")opts.ignore_warnings = po.count("ignore_warnings")>0 && po.get<char>("ignore_warnings") == 'y';
  IF_SPECIFIED("ha")opts.ha_convert_mode = po.count("ha")>0 && po.get<char>("ha") == 'y';
  IF_SPECIFIED("hs")opts.ha_simulator_mode = po.count("hs")>0 && po.get<char>("hs") == 'y';
  IF_SPECIFIED("ltl")opts.ltl_model_check_mode = po.count("ltl")>0 && po.get<char>("ltl") == 'y';
  IF_SPECIFIED("epsilon")opts.epsilon_mode = po.get<int>("epsilon");
  IF_SPECIFIED("interval")opts.interval = po.count("interval") > 0 && po.get<char>("interval") == 'y';
  IF_SPECIFIED("numerize_without_validation")opts.numerize_mode =  po.count("numerize_without_validation") > 0 && po.get<char>("numerize_without_validation") == 'y';
  IF_SPECIFIED("fail_on_stop")opts.stop_at_failure = po.count("fail_on_stop") > 0 && po.get<char>("fail_on_stop") == 'y';
  IF_SPECIFIED("approximation_step")opts.approximation_step = po.get<int>("approximation_step");
  IF_SPECIFIED("vars_to_approximate")
  {
    add_vars_from_string(po.get<string>("vars_to_approximate"), opts.vars_to_approximate, "");
  }
  IF_SPECIFIED("use_fullsimplify")opts.fullsimplify = po.count("use_fullsimplify");
}


int simulate(boost::shared_ptr<hydla::parse_tree::ParseTree> parse_tree)
{
  process_opts(opts, cmdline_options, false);
  
  if(opts.debug_mode)    Logger::instance().set_log_level(Logger::Debug);
  else     Logger::instance().set_log_level(Logger::Warn);


  backend_.reset(new Backend(new MathematicaLink(opts.mathlink, opts.ignore_warnings)));
  PhaseResult::backend = backend_.get();

  if(opts.ltl_model_check_mode)
  {
    simulator_ = new LTLModelChecker(opts);
  }
  else
  {
    simulator_ = new SequentialSimulator(opts);
  }

  simulator_->set_backend(backend_);
  simulator_->set_phase_simulator(new PhaseSimulator(simulator_, opts));
  simulator_->initialize(parse_tree);
  simulator_->simulate();
  if(!opts.ha_convert_mode)
  {
    output_result(*simulator_, opts);
  }
  int simulation_status = simulator_->get_exit_status();
  delete simulator_;
  return simulation_status;
}

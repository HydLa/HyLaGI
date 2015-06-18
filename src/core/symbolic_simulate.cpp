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

  hydla::io::SymbolicTrajPrinter Printer(sstr);
  if(opts.epsilon_mode >= 0){Printer.set_epsilon_mode(backend_, true);}

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

#define IF_SPECIFIED(X) if(use_default || !po.defaulted(X))

void setup_simulator_opts(Opts& opts, ProgramOptions& po, bool use_default)
{
  opts.mathlink      = "-linkmode launch -linkname '" + po.get<string>("math_name") + " -mathlink'";
  opts.debug_mode    = po.count("debug") > 0;
  opts.ltl_model_check_mode = po.count("ltl")>0;
  IF_SPECIFIED("time")opts.max_time      = po.get<string>("time");
  IF_SPECIFIED("phase")opts.max_phase      = po.get<int>("phase");
  IF_SPECIFIED("nd")opts.nd_mode       = po.count("nd") > 0 && po.get<char>("nd") == 'y';
  IF_SPECIFIED("static_generation_of_module_sets")  opts.static_generation_of_module_sets = po.count("static_generation_of_module_sets") && po.get<char>("static_generation_of_module_sets") == 'y';
  IF_SPECIFIED("dump_in_progress") opts.dump_in_progress = po.count("dump_in_progress")>0 && po.get<char>("dump_in_progress") == 'y';
  opts.dump_relation = po.count("dump_relation_graph")>0;
<<<<<<< HEAD
  IF_SPECIFIED("ignore_warnings")opts.ignore_warnings = po.count("ignore_warnings")>0 && po.get<char>("ignore_warnings") == 'y';
  IF_SPECIFIED("ha")opts.ha_convert_mode = po.count("ha")>0 && po.get<char>("ha") == 'y';
  IF_SPECIFIED("hs")opts.ha_simulator_mode = po.count("hs")>0 && po.get<char>("hs") == 'y';
  IF_SPECIFIED("epsilon")opts.epsilon_mode = po.get<int>("epsilon");

  IF_SPECIFIED("fail_on_stop")opts.stop_at_failure = po.count("fail_on_stop") > 0 && po.get<char>("fail_on_stop") == 'y';
=======
  opts.interactive_mode = po.count("in")>0;
  opts.use_unsat_core = po.count("use_unsat_core")>0;
  opts.ignore_warnings = po.count("ignore_warnings")>0;
  opts.ha_convert_mode = po.count("ha")>0;
  opts.ha_simulator_mode = po.count("hs")>0;
  //opts.profile_mode  = po.count("profile")>0;
  opts.parallel_mode = po.count("parallel")>0;
  opts.parallel_number   = po.get<int>("pn");
  opts.reuse = po.count("reuse")>0;
  opts.approx = po.count("approx")>0;
  opts.cheby = po.count("change")>0;
  opts.epsilon_mode = po.get<int>("epsilon");
  opts.interval_newton = po.count("interval_newton")>0;
  opts.max_ip_width = po.get<double>("max_ip_width");
  /*
  opts.output_interval = po.get<std::string>("output_interval");
  */
  opts.analysis_mode = po.get<std::string>("analysis_mode");
  opts.analysis_file = po.get<std::string>("analysis_file");
  opts.stop_at_failure = po.count("fail_stop") == 1;
  opts.solver        = po.get<std::string>("solver");
  /*opts.optimization_level = po.get<int>("optimization_level");
  if(opts.optimization_level < 0 || opts.optimization_level > 4){
    throw std::runtime_error(std::string("invalid option - optimization_level"));
  }
  */
  opts.optimization_level = 0;

  /*
  opts.timeout = po.get<int>("timeout");
  opts.timeout_case = po.get<int>("timeout_case");
  opts.timeout_phase = po.get<int>("timeout_phase");
  */
  opts.timeout_calc= po.get<int>("timeout_calc");

  
  //opts.max_loop_count= po.get<int>("mlc");

  // select search method (dfs or bfs)
  if(po.get<std::string>("search") == "d"){
    opts.search_method = DFS;
  }else if(po.get<std::string>("search") == "b"){
    opts.search_method = BFS;
  }else{
    throw std::runtime_error(std::string("invalid option - search"));
  }
>>>>>>> Hyrose_Newton
}

int simulate(boost::shared_ptr<hydla::parse_tree::ParseTree> parse_tree)
{
  setup_simulator_opts(opts, cmdline_options, false);

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

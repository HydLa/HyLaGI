#include "ModuleSetList.h"

#include "ProgramOptions.h"
#include "SymbolicPhaseSimulator.h"
#include "SequentialSimulator.h"
#include "InteractiveSimulator.h"
#include "SymbolicTrajPrinter.h"
#include "StdProfilePrinter.h"
#include "CsvProfilePrinter.h"
#include "HAConverter.h"
#include "HASimulator.h"
#include "ParallelSimulator.h"
#include "MathematicaLink.h"
#include "REDUCELinkFactory.h"
#include "Backend.h"
#include "JsonWriter.h"

#ifdef _MSC_VER
#include <windows.h>
#endif
// parser
#include "DefaultNodeFactory.h"

#include <boost/lexical_cast.hpp>

// namespace
using namespace hydla;
using namespace hydla::parse_tree;
using namespace hydla::parser;
using namespace hydla::ch;
using namespace hydla::simulator::symbolic;
using namespace hydla::simulator;
using namespace hydla::backend;
using namespace hydla::backend::mathematica;
using namespace hydla::backend::reduce;
using namespace hydla::output;
using namespace std;


Simulator* simulator_;
Opts opts;

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
  ProgramOptions &po = ProgramOptions::instance();
  std::stringstream sstr;
  hydla::output::SymbolicTrajPrinter Printer(opts.output_variables, sstr);
  Printer.output_parameter_map(ss.get_parameter_map());
  Printer.output_result_tree(ss.get_result_root());
  std::cout << sstr.str(); 

  // TODO: use boost (for compatibility)
  std::string of_name = po.get<string>("output_name"); 
  if(of_name.empty()) 
  {
    const std::string hydat_dir = "./hydat/";
    if(po.count("input-file"))
    {
      std::string if_name = po.get<string>("input-file");
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

  if(po.get<std::string>("tm") == "s") {
    hydla::output::StdProfilePrinter().print_profile(ss.get_profile());
  } else if(po.get<std::string>("tm") == "c") {
    std::string csv_name = po.get<std::string>("csv");
    if(csv_name == ""){
      hydla::output::CsvProfilePrinter().print_profile(ss.get_profile());
    }else{
      std::ofstream ofs;
      ofs.open(csv_name.c_str());
      hydla::output::CsvProfilePrinter(ofs).print_profile(ss.get_profile());
      ofs.close();
    }
  }
}

void setup_symbolic_simulator_opts(Opts& opts)
{  
  ProgramOptions &po = ProgramOptions::instance();
  
  opts.mathlink      = "-linkmode launch -linkname '" + po.get<std::string>("math_name") + " -mathlink'";
  opts.debug_mode    = po.get<std::string>("debug")!="";
  opts.max_time      = po.get<std::string>("time");
  opts.max_phase      = po.get<int>("phase");
  opts.nd_mode       = po.count("nd") > 0;
  opts.dump_in_progress = po.count("dump_in_progress")>0;
  opts.dump_relation = po.count("dump_module_relation_graph")>0;
  opts.interactive_mode = po.count("in")>0;
  opts.find_unsat_core_mode = po.count("find_unsat_core")>0;
  opts.use_unsat_core = po.count("use_unsat_core")>0;
  opts.ignore_warnings = po.count("ignore_warnings")>0;
  opts.ha_convert_mode = po.count("ha")>0;
  opts.ha_simulator_mode = po.count("hs")>0;
  //opts.profile_mode  = po.count("profile")>0;
  opts.parallel_mode = po.count("parallel")>0;
  opts.parallel_number   = po.get<int>("pn");
  opts.reuse = po.count("reuse")>0;
  opts.approx = po.count("approx")>0;
  /*
  opts.output_interval = po.get<std::string>("output_interval");
  opts.output_precision = po.get<int>("output_precision");
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
  
  
  /*
  opts.max_loop_count= po.get<int>("mlc");
  opts.max_phase_expanded = po.get<int>("phase_expanded");
  */

  // select search method (dfs or bfs)
  if(po.get<std::string>("search") == "d"){
    opts.search_method = simulator::DFS;
  }else if(po.get<std::string>("search") == "b"){
    opts.search_method = simulator::BFS;
  }else{
    throw std::runtime_error(std::string("invalid option - search"));
  } 
}

void symbolic_simulate(boost::shared_ptr<hydla::parse_tree::ParseTree> parse_tree)
{
  Opts opts;
  setup_symbolic_simulator_opts(opts);
  
  boost::shared_ptr<Backend> backend;
  
  if(opts.solver == "m" || opts.solver == "Mathematica") {
    backend.reset(new Backend(new MathematicaLink(opts)));
  }else{
    REDUCELinkFactory rlf;
    REDUCELink *reduce_link  = rlf.createInstance(opts);
    backend.reset(new Backend(reduce_link));
  }

  if(opts.interactive_mode)
  {
    simulator_ = new InteractiveSimulator(opts);
  }
  else if(opts.parallel_mode)
  {
    simulator_ = new ParallelSimulator(opts);
  }
  else if(opts.ha_convert_mode)
  {
    simulator_ = new HAConverter(opts);
  }
  else if(opts.ha_simulator_mode)
  {
    opts.nd_mode = true;
  	
  	timer::Timer hac_timer;

  	HAConverter ha_converter(opts);
    ha_converter.set_backend(backend);
    ha_converter.set_phase_simulator(new SymbolicPhaseSimulator(&ha_converter, opts));
    ha_converter.initialize(parse_tree);

  	ha_converter.simulate();
  	hac_timer.elapsed("HAConverter Time");
    
    HASimulator* ha_simulator = new HASimulator(opts);
    ha_simulator->set_ha_results(ha_converter.get_results());
    simulator_ = ha_simulator;
  }
  else
  {
    simulator_ = new SequentialSimulator(opts);
  }

  simulator_->set_backend(backend);
  simulator_->set_phase_simulator(new SymbolicPhaseSimulator(simulator_, opts));  
  simulator_->initialize(parse_tree);
  simulator_->simulate();
  if(!opts.ha_convert_mode)
  {
    output_result(*simulator_, opts);
  }

  delete simulator_;
}

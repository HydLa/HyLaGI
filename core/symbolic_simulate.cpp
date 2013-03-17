#include "ModuleSetList.h"

#include "ProgramOptions.h"
#include "SymbolicSimulator.h"
#include "SequentialSimulator.h"
#include "InteractiveSimulator.h"
#include "SymbolicTrajPrinter.h"
#include "StdProfilePrinter.h"
#include "CsvProfilePrinter.h"
//#include "HAConverter.h"
//#include "ParallelSimulator.h"


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
using namespace hydla::symbolic_simulator;
using namespace hydla::simulator;

SequentialSimulator* simulator_;
Opts opts;


void output_result(SequentialSimulator& ss, Opts& opts){
  ProgramOptions &po = ProgramOptions::instance();
  hydla::output::SymbolicTrajPrinter Printer(opts.output_variables);
  Printer.output_parameter_set(ss.get_parameter_set());
  Printer.output_result_tree(ss.get_result_root());
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

/*
void output_result(ParallelSimulator& ss, Opts& opts){
  ProgramOptions &po = ProgramOptions::instance();
  hydla::output::SymbolicTrajPrinter Printer(opts.output_variables);
  Printer.output_result_tree(ss.get_result_root());
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
*/


/**
 * 記号処理によるシミュレーション
 */
void setup_symbolic_simulator_opts(Opts& opts)
{  
  ProgramOptions &po = ProgramOptions::instance();
  
  opts.mathlink      = "-linkmode launch -linkname '" + po.get<std::string>("math-name") + " -mathlink'";
  opts.debug_mode    = po.get<std::string>("debug")!="";
  opts.max_time      = po.get<std::string>("time");
  opts.max_phase      = po.get<int>("phase");
  opts.nd_mode       = po.count("nd") > 0;
  opts.dump_in_progress = po.count("dump-in-progress")>0;
  opts.dump_relation = po.count("dump-module-relation-graph")>0;
  opts.interactive_mode = po.count("in")>0;
  //opts.ha_convert_mode = po.count("ha")>0;
  //opts.profile_mode  = po.count("profile")>0;
  //opts.parallel_mode = po.count("parallel")>0;
  //opts.parallel_number   = po.get<int>("pn");
  /*
  opts.output_interval = po.get<std::string>("output-interval");
  opts.output_precision = po.get<int>("output-precision");
  */
  opts.stop_at_failure = po.count("fail-stop") == 1;
  //opts.solver        = po.get<std::string>("solver");
  /*opts.optimization_level = po.get<int>("optimization-level");
  if(opts.optimization_level < 0 || opts.optimization_level > 4){
    throw std::runtime_error(std::string("invalid option - optimization_level"));
  }
  */
  
  /*
  opts.timeout = po.get<int>("timeout");
  opts.timeout_case = po.get<int>("timeout_case");
  opts.timeout_phase = po.get<int>("timeout_phase");
  */
  opts.timeout_calc= po.get<int>("timeout_calc");
  
  
  /*opts.max_loop_count= po.get<int>("mlc");
  opts.max_phase_expanded = po.get<int>("phase_expanded");
  */
  if(po.get<std::string>("search") == "d"){
    opts.search_method = simulator::DFS;
  }else if(po.get<std::string>("search") == "b"){
    opts.search_method = simulator::BFS;
  }else{
    throw std::runtime_error(std::string("invalid option - search"));
  }
  
  
  
  /* std::string output_variables = po.get<std::string>("output-variables");
  std::string::size_type prev = 0, now;
  while( (now = output_variables.find('_', prev)) != std::string::npos){
    std::string name = output_variables.substr(prev, now-prev);
    if(now == output_variables.length()-1)break;
    now++;
    prev = output_variables.find('_', now);
    if(prev == std::string::npos) prev = output_variables.length();
    int d_count;
    try{ 
      d_count = boost::lexical_cast<int>(output_variables.substr(now, prev-now));
    }catch(boost::bad_lexical_cast e){
      std::cerr << "invalid option format - output variables" << std::endl;
      exit(-1);
    }
    name.append(d_count, '\'');
    opts.output_variables.insert(name);
    prev++;
  }
  */
}

void symbolic_simulate(boost::shared_ptr<hydla::parse_tree::ParseTree> parse_tree)
{
  setup_symbolic_simulator_opts(opts);
  simulator_ = new SequentialSimulator(opts);
  ProgramOptions &po = ProgramOptions::instance();

  if(opts.interactive_mode){
    InteractiveSimulator is(opts);
    is.set_phase_simulator(new SymbolicSimulator(opts));
    is.initialize(parse_tree);
    is.simulate();
    /*
    hydla::output::SymbolicTrajPrinter Printer(opts.output_variables);
    Printer.output_parameter_set(is.get_parameter_set());
    Printer.output_result_tree(is.get_result_root());
  }else if(opts.parallel_mode){
    ParallelSimulator ps(opts);
    ps.set_phase_simulator(new SymbolicSimulator(opts));
    ps.initialize(parse_tree);
    ps.simulate();
    output_result(ps, opts);
  }else if(opts.ha_convert_mode){
  	opts.nd_mode = true;
  	HAConverter ha_converter(opts);
    ha_converter.set_phase_simulator(new SymbolicSimulator(opts));
    ha_converter.initialize(parse_tree);
    ha_converter.simulate();
    */
  }
  else{
    SequentialSimulator& ss = *simulator_;
    ss.set_phase_simulator(new SymbolicSimulator(opts));
    ss.initialize(parse_tree);
    ss.simulate();
    output_result(ss, opts);
  }
}

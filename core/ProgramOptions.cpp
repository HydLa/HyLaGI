#include "ProgramOptions.h"

#define LINE_LENGTH 30

namespace hydla {

using namespace boost::program_options;

ProgramOptions::ProgramOptions() : visible_desc_(LINE_LENGTH)
{
  init_descriptions();
}

ProgramOptions::~ProgramOptions() 
{
}

// TODO:コメントアウトしてあるオプションについて考える
void ProgramOptions::init_descriptions()
{
  options_description generic_desc("Usage: hydla [options] [file]\n\nAllowed options:", 
                                   LINE_LENGTH);
  generic_desc.add_options()
    ("help, h", "produce help message")
    ("version", "display version")
    ("debug,d", value<std::string>()->default_value(""), "enable debug mode\n")
   // ("profile", "enable profile mode")

  
    ("dump-parse-tree", 
     "output parse tree")
    ("dump-module-set-list", 
     "output set of module sets\n"
     "  which might be solution\n"
     "  by list representation")
    /*("dump-module-set-list-noinit", 
     "output set of non initial module sets\n"
     "  which might be solution\n"
     "  by list representation")*/
    ("dump-module-set-graph", 
     "output set of module sets\n"
     "  which might be solution\n"
     "  by graph representation")
    /*("dump-module-set-graph-noinit", 
     "output set of non initial module sets\n"
     "  which might be solution\n"
     "  by graph representation")
     */
    ("dump-module-relation-graph", 
     "output relation of module and variables\n"
     "  by graph representation")
    ("dump-in-progress", 
     "output each phase in progress")
     
     /*
    ("output-variables,v", 
     value<std::string>()->default_value(""), 
     "variables to output")
     */
     
    ("search",
     value<std::string>()->default_value("d"), 
     "search method:\n"
     "  d: Depth First Search\n"
     "  b: Breadth First Search")

     
     /*
     ("solver,s",
     value<std::string>()->default_value("m"), 
     "solver:\n"
     "  m or Mathematica\n"
     "  r or Reduce")
     */

    ("nd", "nondeterministic mode")
    
    ("in", "interactive mode")

	  /*("ha", "convert to HA")

    ("parallel", "parallel mode")

    ("pn", 
     value<int>()->default_value(2),
     "parallel number")
     */

    ("tm", 
     value<std::string>()->default_value("n"),
     "time measurement:\n"
     "  n - not measured\n"
     "  s - output standard format\n"
     "  c - output csv format\n")
     
    ("csv", 
     value<std::string>()->default_value(""),
     "csv file name for \"--tm c\":\n"
     " empty - standard out\n")
/*
    ("optimization-level,O",
     value<int>()->default_value(0),
     "optimization level:\n")
     */
    /*
    ("output-format,f", 
     value<std::string>()->default_value("t"), 
     "output format:\n"
     "  t - time function\n"
     "  n - numeric\n"
     "  i - numeric interval\n"
     "  m - for Mathematica Plot")
     */
    
    /*
    ("output-interval", 
     value<std::string>()->default_value("1/10"), 
     "max time interval of output message")
  
    ("output-precision", 
     value<int>()->default_value(5), 
     "precision of output message")
     */

    ("time,t", 
     value<std::string>()->default_value(""), 
     "simulation time for the model\n"
     "  empty: infinity")
     
    ("phase,p", 
     value<int>()->default_value(-1), 
     "simulation limit for number of phases in model\n"
     "  positive value: number of phases\n"
     "  negative value: infinity")

     /*
    ("phase_expanded", 
     value<int>()->default_value(-1), 
     "simulation limit for number of phases expanded in simulation\n"
     " (equivalent to \"phase\" if \"nd\" is invalid)"
     "  positive value: number of phases\n"
     "  negative value: infinity")
     */
     
     /*
    ("timeout", 
     value<int>()->default_value(-1),
     "timeout (not implemented)"
     " negative or zero - infinity")
     
    ("timeout_phase", 
     value<int>()->default_value(-1),
     "timeout for each phase(not implemented)"
     " negative or zero - infinity")
     
    ("timeout_case", 
     value<int>()->default_value(-1),
     "timeout for each case(not implemented)"
     " negative or zero - infinity")
     */
     
    ("timeout_calc", 
     value<int>()->default_value(-1),
     "timeout for each calculation in backend(second)"
     " negative or zero - infinity")
     
    ("fail-stop", 
     "stop all simulation cases when assertion fails")

    /*
		("mlc", 
     value<int>()->default_value(1),
     "HAConverter: Max Loop Count")
     */

    ("math-name", 
     value<std::string>()->default_value("math"), 
     "name of mathematica command")
    ;

  options_description hidden_desc("Hidden options");
  hidden_desc.add_options()
    ("input-file", value<std::string>(), "input file")
    ;

  visible_desc_.add(generic_desc);
  cmdline_desc_.add(generic_desc).add(hidden_desc);
}


void ProgramOptions::parse(int argc, char* argv[]) 
{
  positional_options_description positional_opt;
  positional_opt.add("input-file", -1);

  store(command_line_parser(argc, argv).
            options(cmdline_desc_).
            positional(positional_opt).run(), vm_);
  notify(vm_);
}

ProgramOptions& ProgramOptions::instance() {
  static ProgramOptions inst;
  return inst;
}

} //namespace hydla

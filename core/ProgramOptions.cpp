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

void ProgramOptions::init_descriptions()
{
  options_description generic_desc("Usage: hydla [options] [file]\n\nAllowed options:", 
				   LINE_LENGTH);
  generic_desc.add_options()
    ("help,h", "produce help message")
    ("version", "version")
    ("debug,d", value<std::string>()->default_value(""), "enable debug mode\n")
    ("profile", "enable profile mode")
    ("parallel,p", "enable parallel execution")

    ("dump-parse-tree", 
     "output parse tree")
    ("dump-module-set-list", 
     "output set of module sets\n"
     "  which might be solution\n"
     "  by list representation")
    ("dump-module-set-list-noinit", 
     "output set of non initial module sets\n"
     "  which might be solution\n"
     "  by list representation")
    ("dump-module-set-graph", 
     "output set of module sets\n"
     "  which might be solution\n"
     "  by graph representation")
    ("dump-module-set-graph-noinit", 
     "output set of non initial module sets\n"
     "  which might be solution\n"
     "  by graph representation")
    ("dump-in-progress", 
     "output each phase in progress")
     
    ("fail-stop", 
     "stop all when any assertion fails")
    ("output-variables,v", 
     value<std::string>()->default_value(""), 
     "variables to output")
     
    ("method,m", 
     value<std::string>()->default_value("s"), 
     "simulation method:\n"
     "  b or BandPSimulator\n"
     "  s or SymbolicSimulator\n"
     "  l or SymbolicLegacySimulator")
     
     ("solver,s",
     value<std::string>()->default_value("m"), 
     "solver:\n"
     "  m or Mathematica\n"
     "  r or Reduce")

    ("nd", "nondeterministic mode")
    
    ("in", "interactive mode")

    ("tm", 
     value<std::string>()->default_value("n"),
     "time measurement:\n"
     "  n - not measured\n"
     "  s - output standard format\n"
     "  c - output csv format\n")
    
    ("output-format,f", 
     value<std::string>()->default_value("t"), 
     "output format:\n"
     "  t - time function\n"
     "  n - numeric\n  i - numeric interval\n"
     "  g - for GUI\n"
     "  m - for Mathematica Plot")
    
    ("output-interval", 
     value<std::string>()->default_value("1/10"), 
     "max time interval of output message")
  
    ("output-precision", 
     value<int>()->default_value(5), 
     "precision of output message")
     
    ("continuity", 
     value<int>()->default_value(2), 
     "default continuity level")

    ("time,t", 
     value<std::string>()->default_value("1"), 
     "simulation time")
     
    ("step", 
     value<int>()->default_value(-1), 
     "simulation steps\n"
     "  positive value: steps\n"
     "  negative value: infinity")
    
    ("interlanguage,i", 
     "show intermediate language")

    ("approx,a", 
     value<int>()->default_value(-1), 
     "approximate mode:\n  negative value: no approx\n  positive value: presicion")

    ("mathlink", 
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

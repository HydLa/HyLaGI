#include "ProgramOptions.h"

#define LINE_LENGTH 1000

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
    ("debug,d", "enable debug mode")
    ("profile", "enable profile mode")
    ("parallel,p", "enable parallel execution")
    ("module-set-list", "output set of module sets which might be solution")
    
    ("output-format,f", 
     value<std::string>()->default_value("n"), 
     "output format:\n  t - time function\n  n - numeric")
    
    ("simulation-time,s", 
     value<std::string>()->default_value("1"), 
     "simulation time")
    
    ("interlanguage,i", 
     "show intermediate language")
    
    ("mathlink", 
     value<std::string>()->default_value("-linkmode launch -linkname 'math -mathlink'"), 
     "mathlink option")
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

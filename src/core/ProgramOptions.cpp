#include "ProgramOptions.h"

#define LINE_LENGTH 30

namespace hydla {

using namespace boost::program_options;
using namespace std;

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
  options_description generic_desc("Usage: hylagi [options] [file]\n\nAllowed options:",
                                   LINE_LENGTH);
  generic_desc.add_options()
    ("help,h", "produce help message (this option)")
    ("version", "display version")
    ("debug,d", "display debug trace\n")
    ("parse_only", "only parse hydla program")
    ("dump_parse_tree", 
     "output parse tree")

    ("dump_module_set_graph", 
     "output candidate sets of module sets\n"
     "  by graph representation")
    ("dump_module_priority_graph",
     "output priorities of modules\n"
     "  by graphviz format")
    ("dump_relation_graph", 
     "output relation of constraints and variables\n"
     "  by graphviz format")
    ("dump_in_progress",
     "output each phase in progress")

    ("search",
     value<std::string>()->default_value("d"),
     "search method:\n"
     "  d: Depth First Search\n"
     "  b: Breadth First Search")
     
    ("static_generation_of_module_sets", "simulation with static generation of module sets")

    ("nd", "nondeterministic mode")

    ("in", "interactive mode")

    ("ha", "convert to HA")

    ("hs", "simulation using HA")

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

    ("output_name,o",
     value<std::string>()->default_value(""),
     "file name for hydat output (if empty ./hydat/<program_name>.hydat)")

    ("time,t",
     value<std::string>()->default_value(""),
     "simulation time for the model\n"
     "  empty: infinity")

    ("ignore_warnings", "ignore warnings created by Backend solvers. \n"
     "(current canidates: DSolve::bvnul, Reduce::ztest1)"
      )

    ("phase,p",
     value<int>()->default_value(-1),
     "simulation limit for number of phases in model\n"
     "  positive value: number of phases\n"
     "  negative value: infinity")

    // TODO: fix message
    ("epsilon,e",
     value<int>()->default_value(-1),
     "epsilon mode")

    ("fail_on_stop",
     "stop all simulation cases when assertion fails")

    ("math_name",
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

void ProgramOptions::parse(string src_str)
{
  char dst_str[src_str.length()];
  strcpy(dst_str, src_str.c_str());
  // TODO: reserve possibly smallest array
  char* argv[src_str.length() + 1];
  // Set the first element(program name) to dummy
  char dummy_hydla[1]{'\0'};
  argv[0] = dummy_hydla;
  int argc = 1;
  if(src_str.length() > 0)
  {
    argv[argc] = strtok(dst_str, " ");
    while(argv[argc] != nullptr){
      ++argc;
      argv[argc] = strtok(nullptr, " ");
    }
  }

  parse(argc, argv);
}

} //namespace hydla

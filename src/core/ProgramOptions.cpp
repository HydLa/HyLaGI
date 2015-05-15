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

pair<string, string> reg_toggle(const string& s)
{
  if (s.find("--f") == 0) {
    if (s.substr(3, 3) == "no-")
      return make_pair(s.substr(6), string("n"));
    else
      return make_pair(s.substr(3), string("y"));
  } else {
    return make_pair(string(), string());
  }
}

// TODO:コメントアウトしてあるオプションについて考える
void ProgramOptions::init_descriptions()
{
  options_description generic_desc("Usage: hylagi [options] [file]\n\nAllowed options",
                                   LINE_LENGTH);
  generic_desc.add_options()
    ("help,h", "produce help message (this option)")
    ("version", "display version")

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

    ("search",
     value<std::string>()->default_value("d"),
     "search method:\n"
     "  d: Depth First Search\n"
     "  b: Breadth First Search")



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
    ("debug,d", "display debug trace\n")

    ("phase,p",
     value<int>()->default_value(-1),
     "simulation limit for number of phases in model\n"
     "  positive value: number of phases\n"
     "  negative value: infinity")

    // TODO: fix message
    ("epsilon,e",
     value<int>()->default_value(-1),
     "epsilon mode")

    ("math_name",
     value<std::string>()->default_value("math"),
     "name of mathematica command")
    ;


  options_description hidden_desc("Hidden options");
  hidden_desc.add_options()
    ("input-file", value<std::string>(), "input file")
    ;

  options_description toggle_desc("Flag options\n"
                                  "(can be specified \"--f[name]\" or \"--[name] y\""
                                  " and can be invalidated \"--fno-[name]\" or \"--[name] n\"");
  toggle_desc.add_options()  
    ("nd", value<std::string>()->default_value("n"), "nondeterministic mode")

    ("in", value<std::string>()->default_value("n"), "interactive mode")

    ("ha", value<std::string>()->default_value("n"), "convert to HA")

    ("hs", value<std::string>()->default_value("n"), "simulation using HA")

    ("fail_on_stop",value<std::string>()->default_value("n"),
     "stop all simulation cases when assertion fails")

    ("static_generation_of_module_sets", value<std::string>()->default_value("n"),"simulation with static generation of module sets")

    ("ignore_warnings", value<std::string>()->default_value("n"), "ignore warnings created by Backend solvers. \n"
     "(current canidates: DSolve::bvnul, Reduce::ztest1)"
      )


    ("dump_in_progress", value<std::string>()->default_value("n"),
     "goutput each phase in progress")
    ;

  visible_desc_.add(generic_desc).add(toggle_desc);
  cmdline_desc_.add(generic_desc).add(toggle_desc).add(hidden_desc);
}


void ProgramOptions::parse(int argc, char* argv[])
{
  positional_options_description positional_opt;
  positional_opt.add("input-file", -1);

  store(command_line_parser(argc, argv).
            options(cmdline_desc_).
        positional(positional_opt).extra_parser(reg_toggle).run(), vm_);
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

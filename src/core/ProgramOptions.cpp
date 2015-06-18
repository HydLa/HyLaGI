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

void ProgramOptions::init_descriptions()
{
  options_description generic_desc("Usage: hylagi [options] [file]\n\nAllowed options",
                                   LINE_LENGTH);
  generic_desc.add_options()
    ("help,h", "display help message (this option)")
    ("version", "display version")

    ("parse_only", "only try to parse given program")
    ("dump_parse_tree", "only output parse tree")

    ("dump_module_set_graph", "only output candidate sets of module sets\n"
     "  in graph representation")
    ("dump_module_priority_graph",

     "only output priorities of modules\n"
     "  in graphviz format")
    ("dump_relation_graph", 
     "only output relation of constraints and variables\n"
     "  in graphviz format")

    ("tm",
     value<std::string>()->default_value("n"),
     "time measurement:\n"
     "  n - not measured\n"
     "  s - output in standard format\n"
     "  c - output in csv format\n")

    ("csv",
     value<std::string>()->default_value(""),
     "name of csv file for \"--tm c\":\n"
     " empty - standard out\n")

    ("output_name,o",
     value<std::string>()->default_value(""),
     "file name for hydat output (if empty \"./hydat/<program_name>.hydat)\"")

    ("debug,d", "display debug trace\n")

    ("math_name",
     value<std::string>()->default_value("math"),
     "name of mathematica command");


  options_description config_desc("Following options also can be specified in comments in the form of \"#hylagi <options>\"");

  config_desc.add_options()
    ("time,t",
     value<std::string>()->default_value(""),
     "time limit of the model\n"
     "  empty: infinity")

    ("phase,p",
     value<int>()->default_value(-1),
     "simulation limit for number of phases in model\n"
     "  positive value: number of phases\n"
     "  negative value: infinity")

    ("epsilon,e",
     value<int>()->default_value(-1),
     "perform 2 additional processed below\n"
     "1. prune branches where the value of epsilon is not in neighborhood of 0\n"
     "2. approximate expressions by cutting off higher order terms about epsilon"
     "  non-negative value: order of approximation\n"
     "  negative value: invalidate this option\n");

  options_description toggle_desc("Flag options\n"
                                  "(can be specified \"--f[name]\" or \"--[name] y\""
                                  " and can be invalidated \"--fno-[name]\" or \"--[name] n\")");
  toggle_desc.add_options()  
    ("nd", value<char>()->default_value('n'), "nondeterministic mode")

    ("ha", value<char>()->default_value('n'), "convert to HA")

    ("hs", value<char>()->default_value('n'), "simulate using HA")

    ("fail_on_stop",value<char>()->default_value('n'),
     "stop all simulation cases when assertion fails")

    ("static_generation_of_module_sets", value<char>()->default_value('n'),"simulate with static generation of module sets")

    ("ignore_warnings", value<char>()->default_value('n'), "ignore warnings created by backend solvers. \n"
     "(current canidates: DSolve::bvnul, Reduce::ztest1)"
      )

    ("interval,i", value<char>()->default_value('n'), "use interval method")


    ("dump_in_progress", value<char>()->default_value('n'),
     "output each phase in progress")
    ;


  
  options_description hidden_desc("Hidden options");
  hidden_desc.add_options()
    ("input-file", value<std::string>(), "input file")
    ;


  visible_desc_.add(generic_desc).add(config_desc).add(toggle_desc);
  cmdline_desc_.add(generic_desc).add(config_desc).add(toggle_desc).add(hidden_desc);
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
  char* argv[(src_str.length() + 1)/2 + 1];
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

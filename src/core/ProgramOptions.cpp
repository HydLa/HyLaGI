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
  options_description generic_desc("Usage: hylagi [options] [file]\n\nAllowed options:",
                                   LINE_LENGTH);
  generic_desc.add_options()
    ("help,h", "produce help message")
    ("version", "display version")
    ("debug,d", "enable debug mode\n")
    ("parse_only", "only parse hydla program")
    ("dump_parse_tree",
     "output parse tree")
    ("dump_module_set_list",
     "output set of module sets\n"
     "  which might be solution\n"
     "  by list representation")

    ("dump_module_set_graph",
     "output set of module sets\n"
     "  which might be solution\n"
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

    ("solver,s",
     value<std::string>()->default_value("m"),
     "solver:\n"
     "  m or Mathematica\n"
     "  r or Reduce")

    ("static_generation_of_module_sets", "simulation with static generation of module sets")

    ("nd", "nondeterministic mode")

    ("in", "interactive mode")

    ("ha", "convert to HA")

    ("hs", "simulation using HA")

    ("parallel", "parallel mode")

    ("pn",
     value<int>()->default_value(2),
     "parallel number")

    ("reuse", "reusing results")

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
    ("optimization-level,O",
     value<int>()->default_value(0),
     "optimization level:\n")

    ("analysis_mode",
     value<std::string>()->default_value(""),
     "analysis mode\n"
     "  empty - don't analyze constraint\n"
     "  use - use analysis file\n"
     "  output - analyze constraint and output to file\n"
     "  simulate - analyze constraint and simulate using analysis result\n")

    ("analysis_file",
     value<std::string>()->default_value(""),
     "analysis file name\n"
     "  empty - standard out or standard in\n")

    ("precision",
     value<int>()->default_value(10),
     "precision of approximation\n"
     "0 or negative number means best-effort for precision\n"
     "(invalid if \"without_validation\" isn't specified ")

    ("time_delta",
     value<int>()->default_value(10),
     "exponent of minimum time step, e.g. 5 means 1.0E5, 10 means 1.0E10 \n"
     "0 or negative number means that time step equals to 0\n"
     "(invalid if \"without_validation\" isn't specified)")

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

    ("approx,a",
     "simulate with approximation")

    ("change,c",
      "change next PP time")

    ("epsilon,e",
     value<int>()->default_value(-1),
     "epsilon mode")

    ("ltl,l", "ltl model check mode")

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
     "timeout for each calculation in backend(second)\n"
     " negative or zero - infinity")

    ("fail_stop",
     "stop all simulation cases when assertion fails")

    /*
		("mlc",
     value<int>()->default_value(1),
     "HAConverter: Max Loop Count")
     */

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

ProgramOptions& ProgramOptions::instance() {
  static ProgramOptions inst;
  return inst;
}

} //namespace hydla

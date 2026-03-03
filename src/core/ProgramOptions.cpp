#include <fstream>
#include <iostream>
#include <random>

#include "ProgramOptions.h"

#define LINE_LENGTH 30

namespace hydla {

using namespace std;

// for options_description, positional_options_description, value,
// command_line_parser
using namespace boost::program_options;

ProgramOptions::ProgramOptions() : visible_desc_(LINE_LENGTH) {
  init_descriptions();
}

ProgramOptions::~ProgramOptions() {}

pair<string, string> reg_toggle(const string &s) {
  if (s.find("--f") == 0) {
    if (s.substr(3, 3) == "no-")
      return make_pair(s.substr(6), string("n"));
    else
      return make_pair(s.substr(3), string("y"));
  } else {
    return make_pair(string(), string());
  }
}

void ProgramOptions::init_descriptions() {
  options_description generic_desc(
      "Usage: hylagi [options] [file]\n\nAllowed options\n(default value is "
      "written in \"()\")\n('n' means \"no\" not a natural number)",
      LINE_LENGTH);
  auto gop = generic_desc.add_options();
  gop("help,h", "display help message (this option)");
  gop("version,v", "display version");

  gop("parse_only", "only try to parse given program");

  if (not is_master()) {
    gop("dump_parse_tree", "only output parse tree");
    gop("dump_parse_tree_json", "only output parse tree in JSON format");
    gop("debug_constraint", "debugging program");
    gop("dump_module_set_graph", "only output candidate sets of module sets\n"
                                 "  in graph representation");
    gop("dump_module_priority_graph", "only output priorities of modules\n"
                                      "  in graphviz format");
    gop("dump_relation_graph",
        "only output relation of constraints and variables\n"
        "  in graphviz format");
    gop("simplify,s", value<int>()->default_value(1),
        "the level of simplification at each phase\n"
        "  0 - no simplification\n"
        "  1 - use \"Simplify\"\n"
        "  2 or others - use \"FullSimplify\"");
    gop("simplify_time", value<std::string>()->default_value("1"),
        "time limit of simplifying expressions in the backend");
    gop("dsolve", value<int>()->default_value(0),
        "the method of differential equation in exDSolve\n"
        "  0 - Try to get initial value problem directly\n"
        "  1 - Try to get initial value problem via constant\n");
    gop("tm",
        value<std::vector<std::string>>()->multitoken()->default_value(
            std::vector<string>{"n"}, ""),
        "time measurement:\n"
        "  n - not measured\n"
        "  s - output in standard format\n"
        "  c - output in csv format\n");
  }

  gop("output_name,o", value<std::string>()->default_value(""),
      "file name for hydat output (if empty \"./hydat/<program_name>.hydat)\"");

  if (not is_master()) {
    gop("debug,d", "display debug trace\n");
    gop("math_name", value<std::string>()->default_value(""),
        "name of the command to execute mathematica");
  }

  gop("time,t", value<std::string>()->default_value("Infinity"),
      "time limit of the model");
  gop("phase,p", value<int>()->default_value(-1),
      "simulation limit for number of phases in model\n"
      "  positive value: number of phases\n"
      "  negative value: infinity");
  gop("epsilon,e", value<int>()->default_value(-1),
      "perform 2 additional processes below\n"
      "1. prune branches where the value of epsilon is not in neighborhood of "
      "0\n"
      "2. approximate expressions by cutting off higher order terms about "
      "epsilon\n"
      "  non-negative value: order of approximation\n"
      "  negative value: invalidate this option\n");

  options_description toggle_desc(
      "Flag options\n"
      "(can be specified \"--f[name]\" or \"--[name] y\""
      " and can be invalidated \"--fno-[name]\" or \"--[name] n\")");
  auto top = toggle_desc.add_options();
  top("nd", value<char>()->default_value('n'), "nondeterministic mode");
  top("ha", value<char>()->default_value('n'), "convert to HA");

  if (not is_master()) {
    // top("hs", value<char>()->default_value('n'), "simulate using HA")
    top("ltl,l", value<char>()->default_value('n'), "ltl model checking mode");
  }
  /*top("affine", value<char>()->default_value('n'),
    "use affine arithmetic to approximate expressions")*/

  // top("fail_on_stop",
  // value<char>()->default_value('n'),
  //                           "stop all simulation cases when assertion
  //                           fails");

  if (not is_master()) {
    // top("static_generation_of_module_sets",
    // value<char>()->default_value('n'),
    //  "simulate with static generation of module sets")

    //     ("ignore_warnings", value<char>()->default_value('n'),
    //      "ignore warnings created by backend solvers. \n"
    //      "current canidates: Solve::incnst, Solve::ifun, DSolve::bvnul, "
    //      "Reduce::ztest1, Minimize::ztest1, Reduce::ztest, "
    //      "Minimize::ztest\n"
    //      "Note: Warnings from HyLaGI itself are always activated")
    top("interval,i", value<char>()->default_value('n'), "use interval method");
    top("step_by_step", value<char>()->default_value('n'),
        "use find_min_time_step_by_step");
    top("guards_to_interval_newton", value<std::string>()->default_value(""),
        "guards to be solved by interval newton method(delimited by \",\")");
    top("affine", value<char>()->default_value('n'),
        "use affine arithmetic to approximate expressions");
    top("numerize_without_validation", value<char>()->default_value('n'),
        "numerize values of variables at the end of each PointPhase");
    top("eager_approximation", value<char>()->default_value('n'),
        "approximate values in advance of each Point Phase");
    top("approximation_step", value<int>()->default_value(1),
        "the interval of step to approximate value of variable");
    top("extra_dummy_num", value<int>()->default_value(0),
        "the number of extra dummy parameters in affine arithmetic.");
    top("static_generation_of_module_sets", value<char>()->default_value('n'),
        "simulate with static generation of module sets");
    top("ignore_warnings", value<char>()->default_value('n'),
        "ignore warnings created by backend solvers. \n"
        "current canidates: Solve::incnst, Solve::ifun, DSolve::bvnul, "
        "Reduce::ztest1, Minimize::ztest1, Reduce::ztest, Minimize::ztest\n"
        "Note: Warnings from HyLaGI itself are always activated");
  }

  top("fail_on_stop", value<char>()->default_value('n'),
      "stop all simulation cases when assertion fails");
  top("dump_in_progress", value<char>()->default_value('n'),
      "output each phase in progress");

  if (not is_master()) {
    top("use_shorthand", value<char>()->default_value('n'),
        "use shorthands for arithmetic expressions (only for parameters)\n");
    top("vars_to_approximate", value<std::string>()->default_value(""),
        "variables to approximate (delimited by \",\")");

    // ("guards_to_interval_newton",
    //  value<std::string>()->default_value(""),
    //  "guards to be solved by interval newton method(delimited by "
    //  "\",\")")

    // ("step_by_step", value<char>()->default_value('n'),
    //  "use find_min_time_step_by_step")

    top("solve_over_reals", value<char>()->default_value('n'),
        "solve constrants over the reals");
  }

  top("html", value<char>()->default_value('n'), "output log with HTML format");

  options_description hidden_desc("Hidden options");
  auto hop = hidden_desc.add_options();
  hop("input-file", value<std::string>(), "input file");

  if (is_master()) {
    hop("dump_parse_tree", "");
    hop("dump_parse_tree_json", "");
    hop("debug_constraint", "");
    hop("dump_module_set_graph", "");
    hop("dump_module_priority_graph", "");
    hop("dump_relation_graph", "");
    hop("simplify,s", value<int>()->default_value(1), "");
    hop("dsolve", value<int>()->default_value(0), "");
    hop("tm",
        value<std::vector<std::string>>()->multitoken()->default_value(
            std::vector<string>{"n"}, ""),
        "");
    hop("csv", value<std::string>()->default_value(""), "");
    hop("debug,d", "");
    hop("simplify_time", value<std::string>()->default_value("1"), "");
    hop("math_name", value<std::string>()->default_value(""), "");
    hop("hs", value<char>()->default_value('n'), "");
    hop("ltl,l", value<char>()->default_value('n'), "");
    hop("affine", value<char>()->default_value('n'), "");

    hop("static_generation_of_module_sets", value<char>()->default_value('n'),
        "");
    hop("ignore_warnings", value<char>()->default_value('n'), "");
    hop("interval,i", value<char>()->default_value('n'), "");
    hop("numerize_without_validation", value<char>()->default_value('n'), "");
    hop("eager_approximation", value<char>()->default_value('n'), "");
    hop("approximation_step", value<int>()->default_value(1), "");
    hop("extra_dummy_num", value<int>()->default_value(0), "");
    hop("use_shorthand", value<char>()->default_value('n'), "");
    hop("vars_to_approximate", value<std::string>()->default_value(""), "");
    hop("guards_to_interval_newton", value<std::string>()->default_value(""),
        "");
    hop("step_by_step", value<char>()->default_value('n'), "");
    hop("solve_over_reals", value<char>()->default_value('n'), "");
  }

  visible_desc_.add(generic_desc).add(toggle_desc);
  cmdline_desc_.add(generic_desc).add(toggle_desc).add(hidden_desc);
}

void ProgramOptions::parse(int argc, char *argv[]) {
  positional_options_description positional_opt;
  positional_opt.add("input-file", -1);

  store(command_line_parser(argc, argv)
            .options(cmdline_desc_)
            .positional(positional_opt)
            .extra_parser(reg_toggle)
            .run(),
        vm_);
  notify(vm_);
}

void ProgramOptions::parse(std::string src_str) {
  char dst_str[src_str.length() + 1];
  strncpy(dst_str, src_str.c_str(), sizeof(dst_str));
  char *argv[(src_str.length() + 1) / 2 + 1];
  // Set the first element(program name) to dummy
  char dummy_hydla[1]{'\0'};
  argv[0] = dummy_hydla;
  int argc = 1;
  if (src_str.length() > 0) {
    argv[argc] = strtok(dst_str, " ");
    while (argv[argc] != nullptr) {
      ++argc;
      argv[argc] = strtok(nullptr, " ");
    }
  }

  parse(argc, argv);
}

bool is_master() { return BRANCH_NAME == (std::string) "master"; }

} // namespace hydla

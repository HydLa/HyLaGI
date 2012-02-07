#include "ModuleSetList.h"

#include "ProgramOptions.h"
#include "SymbolicSimulator.h"

// parser
#include "DefaultNodeFactory.h"

#include <boost/lexical_cast.hpp>

// namespace
using namespace hydla;
using namespace hydla::parse_tree;
using namespace hydla::parser;
using namespace hydla::ch;
using namespace hydla::symbolic_simulator;

/**
 * 記号処理によるシミュレーション
 */
void setup_symbolic_simulator_opts(Opts& opts)
{  
  ProgramOptions &po = ProgramOptions::instance();

  if(po.get<std::string>("output-format") == "t") {
    opts.output_format = fmtTFunction;
  } else if(po.get<std::string>("output-format") == "n"){
    opts.output_format = fmtNumeric; 
  } else if(po.get<std::string>("output-format") == "m"){
    opts.output_format = fmtMathematica; 
  } else if(po.get<std::string>("output-format") == "g"){
    opts.output_format = fmtGUI; 
  } else if(po.get<std::string>("output-format") == "i"){
    opts.output_format = fmtNInterval;
  } else {
    // TODO: 例外を投げるようにする
    std::cerr << "invalid option - output format" << std::endl;
    exit(-1);
  }

  opts.mathlink      = "-linkmode launch -linkname '" + po.get<std::string>("mathlink") + " -mathlink'";
  opts.debug_mode    = po.get<std::string>("debug")!="";
  opts.max_time      = po.get<std::string>("time");
  opts.max_step      = po.get<int>("step");
  opts.nd_mode       = po.count("nd")>0;
  opts.dump_in_progress = po.count("dump-in-progress")>0;
  opts.interactive_mode = po.count("in")>0;
  opts.profile_mode  = po.count("profile")>0;
  opts.parallel_mode = po.count("parallel")>0;
  opts.output_interval = po.get<std::string>("output-interval");
  opts.output_precision = po.get<int>("output-precision");
  opts.approx_precision = po.get<int>("approx");
  opts.exclude_error = po.count("fail-stop") == 0;
  opts.solver        = po.get<std::string>("solver");
  int level = po.get<int>("continuity");
  if(level <= 0){
    opts.default_continuity = CONT_NONE;
  }else if(level >= CONT_NUM){
    opts.default_continuity = CONT_STRONG;
  }else{
    opts.default_continuity = (DefaultContinuity)level;
  }
  
  std::string output_variables = po.get<std::string>("output-variables");
  std::string::size_type prev = 0, now;
  while( (now = output_variables.find('_', prev)) != std::string::npos){
    std::string name = output_variables.substr(prev, now-prev);
    if(now == output_variables.length()-1)break;
    now++;
    prev = output_variables.find('_', now);
    //std::cout << output_variables.substr(now, prev-now) << std::endl;
    if(prev == std::string::npos) prev = output_variables.length();
    int d_count;
    try{ d_count = boost::lexical_cast<int>(output_variables.substr(now, prev-now));
    }catch(boost::bad_lexical_cast e){
      std::cerr << "invalid option format - output variables" << std::endl;
      exit(-1);
    }
    name.append(d_count, '\'');
    opts.output_variables.insert(name);
    prev++;
    //std::cout << prev << "," << now << std::endl;
  }
  /*
  for(std::set<std::string>::iterator it = opts.output_variables.begin(); it != opts.output_variables.end(); it++){
    std::cout << *it << std::endl;
  }*/
}

void symbolic_simulate(boost::shared_ptr<hydla::parse_tree::ParseTree> parse_tree)
{
  Opts opts;
  setup_symbolic_simulator_opts(opts);

  SymbolicSimulator ms(opts);
  ms.initialize(parse_tree);
  ms.simulate();
}


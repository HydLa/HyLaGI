#include <iostream>
#include <exception>
#include <string>
#include <vector>

#include <boost/bind.hpp>

#include "version.h"
#include "HydLaParser.h"
#include "MathSimulator.h"
#include "mathlink_helper.h"

#include "ModuleSetGraph.h"
#include "ProgramOptions.h"

using namespace hydla;

void hydla_main()
{
  ProgramOptions &po = ProgramOptions::instance();

  if(po.count("version")) {
    std::cout << Version::description() << std::endl;
    return;
  }

  HydLaParser hp;
  bool suc;
  if(po.count("input-file")) {
    suc = hp.parse_flie(po.get<std::string>("input-file").c_str());
  } else {
    suc = hp.parse(std::cin);
  }

  if(suc) {
    hp.dump();

    std::string interlanguage = 
      hp.create_interlanguage(po.get<std::string>("simulation-time").c_str());
 
    if(po.count("interlanguage")) {
      std::cout <<  interlanguage  << std::endl;
    } else {

      MathSimulator::OutputFormat output_format;
      if(po.get<std::string>("output-format") == "t") {
	output_format = MathSimulator::fmtTFunction;
      } else if(po.get<std::string>("output-format") == "n"){
	output_format = MathSimulator::fmtNumeric; 
      } else {
	std::cerr << "invalid option - output format" << std::endl;
	return;
      }

      MathSimulator ms;
      ms.simulate(po.get<std::string>("mathlink").c_str(), 
		  interlanguage.c_str(),
		  (bool)po.count("debug")>0,
		  (bool)po.count("profile")>0,
		  (bool)po.count("parallel")>0,
		  output_format);
    }
  } else {
    std::cerr << "parse error -- line: " << hp.get_line() << std::endl;
  }
}

int main(int argc, char* argv[]) 
{
#ifdef _MSC_VER
  _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

  try {
    //ModuleSetGraph msg;
    //msg.dump();
    ProgramOptions &po = ProgramOptions::instance();
    po.parse(argc, argv);
    if(po.count("help")) {
      po.help_msg(std::cout);
    } else {
      hydla_main();
    }
  } 
  catch(std::exception &e) {
    std::cerr << "err:" << e.what() << std::endl;
    return -1;
  } 
  catch(...) {
    std::cerr << "fatal error!!" << std::endl;
    return -1;
  }

  return 0;
}

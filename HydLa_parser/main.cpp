#include <iostream>
#include <exception>
#include <string>
#include <vector>

#include <boost/program_options.hpp>
#include <boost/thread.hpp>
#include <boost/bind.hpp>

#include "HydLaParser.h"
#include "MathSimulator.h"
#include "mathlink_helper.h"

using namespace hydla;
namespace po = boost::program_options;


void hydla_main(int argc, char* argv[])
{
  po::variables_map vm;

  po::options_description generic_desc("Usage: hydla [options] file\nOptions");
  generic_desc.add_options()
    ("help,h", "produce help message")
    ("version", "version")
    ("debug,d", "show debug message")
    ("simulation-time,s", po::value<std::string>()->default_value("1"), "simulation time")
    ("interlanguage,i", "show intermediate language")
    ("mathlink", 
     po::value<std::string>()->default_value("-linkmode launch -linkname 'math -mathlink'"), 
     "mathlink option")
    ;

  po::options_description hidden_desc("Hidden options");
  hidden_desc.add_options()
    ("input-file", po::value<std::string>(), "input file")
    ;

  po::options_description cmdline_desc;
  cmdline_desc.add(generic_desc).add(hidden_desc);

  po::options_description visible_desc("Allowed options");
  visible_desc.add(generic_desc);

  po::positional_options_description positional_opt;
  positional_opt.add("input-file", -1);

  po::store(po::command_line_parser(argc, argv).
	    options(cmdline_desc).
	    positional(positional_opt).run(), vm);
  po::notify(vm);


  if (vm.count("help") || !vm.count("input-file")) {
    std::cout << generic_desc << "\n";
    return;
  }

  HydLaParser hp;
  if(hp.parse_flie(vm["input-file"].as<std::string>().c_str())) {
    bool debug = vm.count("debug");
    std::string interlanguage = 
      hp.create_interlanguage(vm["simulation-time"].as<std::string>().c_str(), 
			      debug);
 
    if(vm.count("interlanguage")) {
      std::cout <<  interlanguage  << std::endl;
    } else {
      MathSimulator ms;
      //boost::thread th(boost::bind(&MathSimulator::simulate, &ms, 
      //			   vm["mathlink"].as<std::string>().c_str(), interlanguage.c_str()));
      ms.simulate(vm["mathlink"].as<std::string>().c_str(), interlanguage.c_str());
      
      //      boost::system_time timeout = boost::posix_time::milliseconds(500); 
      //while(!th.timed_join(boost::posix_time::milliseconds(100))) {
      //	std::cout << "wait..." << std::endl;
      //}
      //std::cout << "exit!!!" << std::endl;
      //      std::cout << "end" << std::endl;
    }
  } else {
    std::cout << "false" << std::endl;
  }
}

int main(int argc, char* argv[]) 
{
#ifdef _MSC_VER
  _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

  try {
    hydla_main(argc, argv);

  } catch(std::exception &e) {
      std::cerr << e.what() << std::endl;

  } catch(...) {
    std::cerr << "fatal error!!" << std::endl;
  }

  //getchar();

  return 0;
}

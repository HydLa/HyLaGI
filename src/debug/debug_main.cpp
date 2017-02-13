#include "debug_main.h"
#include "ParseTree.h"

// constraint hierarchy
#include "ModuleSetContainerCreator.h"
#include "IncrementalModuleSet.h"

#include <iostream>

using namespace hydla::debug;
using namespace std;
using namespace hydla::parse_tree;
using namespace hydla::hierarchy;

namespace hydla{
  namespace debug{
    Debug::Debug(){}

    void Debug::dump_debug(Debug *db, std::string string, boost::shared_ptr<ParseTree> pt, boost::shared_ptr<IncrementalModuleSet> msc)
    {
      //pt->dump_in_json(cout);
      cout << *pt;
      //cout << string;// << std::endl;
      db -> debug_call();
      //cout << *msc;

      for(auto m : weaker_modules_){
        for(auto wm : m.second){
          s << "  \"" << m.first.first << "\" -> \"" << wm.first << "\";" << std::endl;
        }
      }
    }

    void Debug::debug_call()
    {
      cout << "debug." << std::endl;
      //return cout;
    }
  }
}

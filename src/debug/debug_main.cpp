#include "debug_main.h"
#include "ParseTree.h"

#include <iostream>

using namespace hydla::debug;
using namespace std;
using namespace hydla::parse_tree;

namespace hydla{
  namespace debug{
    Debug::Debug(){}

    void Debug::dump_debug(Debug *db, std::string string, boost::shared_ptr<ParseTree> pt)
    {
      pt->dump_in_json(cout);
      cout << string << std::endl;
      db -> debug_call();
    }

    void Debug::debug_call()
    {
      cout << "debug." << std::endl;
      //return cout;
    }
  }
}

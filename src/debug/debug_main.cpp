#include "debug_main.h"

#include <iostream>

using namespace hydla::debug;
using namespace std;

namespace hydla{
  namespace debug{
    Debug::Debug(){}

    void Debug::dump_debug(Debug *db, std::string string)
    {
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

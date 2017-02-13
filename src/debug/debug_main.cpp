#include "debug_main.h"

#include <iostream>

using namespace hydla::debug;
using namespace std;

namespace hydla{
  namespace debug{
    Debug::Debug(){}

void Debug::dump_debug(Debug *db)
{
  db -> debug_call();
}

void Debug::debug_call()
{
  cout << "debug." << std::endl;
  //return cout;
}
  }
}

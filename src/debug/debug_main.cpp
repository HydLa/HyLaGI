#include "debug_main.h"


using namespace hydla::debug;

std::ostream& Debug::dump_debug(std::ostream& s) const
{
  s << "ParseTree is empty." << std::endl;
  return s;
}


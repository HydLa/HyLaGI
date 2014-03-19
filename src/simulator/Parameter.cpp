#include "Parameter.h"

namespace hydla{
namespace simulator{

std::ostream& operator<<(std::ostream& s, 
                               const Parameter& p)
{
  return p.dump(s);
}

} //simulator
} //hydla

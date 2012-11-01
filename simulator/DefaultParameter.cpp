#include "DefaultParameter.h"

namespace hydla{
namespace simulator{

std::ostream& operator<<(std::ostream& s, 
                               const DefaultParameter& p)
{
  return p.dump(s);
}

} //simulator
} //hydla
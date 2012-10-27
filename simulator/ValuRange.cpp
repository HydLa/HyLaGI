#include "ValueRange.h"

namespace hydla {
namespace simulator {

std::ostream& operator<<(std::ostream& s, const ValueRange & val)
{
  return val.dump(s);
}

} // namespace simulator
} // namespace hydla
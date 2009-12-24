#include "ModuleSetContainer.h"

namespace hydla {
namespace ch {

std::ostream& operator<<(std::ostream& s, const ModuleSetContainer& m)
{
  return m.dump(s);
}

} // namespace ch
} // namespace hydla

#include "MathematicaVCS.h"

#include "Logger.h"

using namespace hydla::vcs;

namespace hydla {
namespace vcs {
namespace mathematica {

MathematicaVCS::MathematicaVCS(Mode m)
{}

MathematicaVCS::~MathematicaVCS()
{}

bool MathematicaVCS::reset()
{
  return true;
}

bool MathematicaVCS::reset(const variable_map_t& vm)
{
  return true;
}

bool MathematicaVCS::create_variable_map(variable_map_t& vm)
{
  return true;
}

Trivalent MathematicaVCS::add_constraint(const tells_t& collected_tells)
{
  return Tri_TRUE;
}
  
Trivalent MathematicaVCS::check_entailment(const ask_node_sptr& negative_ask)
{
  return Tri_TRUE;
}

bool MathematicaVCS::integrate(
  integrate_result_t& integrate_result,
  const positive_asks_t& positive_asks,
  const negative_asks_t& negative_asks,
  const time_t& current_time,
  const time_t& max_time)
{
  return true;
}


} // namespace mathematica
} // namespace simulator
} // namespace hydla 

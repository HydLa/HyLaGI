#include "MathematicaVCS.h"

#include "Logger.h"

namespace hydla {
namespace vcs {
namespace mathematica {

MathematicaVCS::MathematicaVCS(Mode m)
{}

MathematicaVCS::~MathematicaVCS()
{}

bool MathematicaVCS::reset()
{}

bool MathematicaVCS::reset(const variable_map_t& vm)
{}

bool MathematicaVCS::create_variable_map(variable_map_t& vm)
{}

Trivalent MathematicaVCS::add_constraint(const tells_t& collected_tells)
{}
  
Trivalent MathematicaVCS::check_entailment(const boost::shared_ptr<Ask>& negative_ask)
{}

bool MathematicaVCS::integrate(
  integrate_result_t& integrate_result,
  const positive_asks_t& positive_asks,
  const negative_asks_t& negative_asks,
  const time_t& current_time,
  const time_t& max_time)
{}


} // namespace mathematica
} // namespace simulator
} // namespace hydla 

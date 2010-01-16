#include "MathematicaVCSPoint.h"

#include <cassert>

#include "mathlink_helper.h"
#include "Logger.h"

using namespace hydla::vcs;

namespace hydla {
namespace vcs {
namespace mathematica {

MathematicaVCSPoint::MathematicaVCSPoint(MathLink* ml) :
  ml_(ml)
{
}

MathematicaVCSPoint::~MathematicaVCSPoint()
{}

bool MathematicaVCSPoint::reset()
{
  return cons_store_.reset();
}

bool MathematicaVCSPoint::reset(const variable_map_t& vm)
{
  return cons_store_.reset(vm);
}

bool MathematicaVCSPoint::create_variable_map(variable_map_t& vm)
{
  return true;
}

Trivalent MathematicaVCSPoint::add_constraint(const tells_t& collected_tells)
{
  return Tri_TRUE;
}
  
Trivalent MathematicaVCSPoint::check_entailment(const ask_node_sptr& negative_ask)
{
  return Tri_TRUE;
}

bool MathematicaVCSPoint::integrate(
  integrate_result_t& integrate_result,
  const positive_asks_t& positive_asks,
  const negative_asks_t& negative_asks,
  const time_t& current_time,
  const time_t& max_time)
{
  // PointÇ≈ÇÕintegrateä÷êîñ≥å¯
  assert(0);
  return false;
}


} // namespace mathematica
} // namespace simulator
} // namespace hydla 


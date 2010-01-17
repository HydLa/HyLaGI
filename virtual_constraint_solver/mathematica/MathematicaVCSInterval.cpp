#include "MathematicaVCSInterval.h"

#include <cassert>

#include "mathlink_helper.h"
#include "Logger.h"
#include "PacketSender.h"

using namespace hydla::vcs;

namespace hydla {
namespace vcs {
namespace mathematica {

MathematicaVCSInterval::MathematicaVCSInterval(MathLink* ml) :
  ml_(ml)
{
}

MathematicaVCSInterval::~MathematicaVCSInterval()
{}

bool MathematicaVCSInterval::reset()
{

  return true;
}

bool MathematicaVCSInterval::reset(const variable_map_t& variable_map)
{

  return true;
}

bool MathematicaVCSInterval::create_variable_map(variable_map_t& variable_map)
{
  // prev[]ÇèúÇ≠èàóùÇÕóvÇÁÇ»Ç≥ÇªÇ§ÅH
  return true;
}

 VCSResult MathematicaVCSInterval::add_constraint(const tells_t& collected_tells)
{

  return VCSR_TRUE;
}
  
VCSResult MathematicaVCSInterval::check_entailment(const ask_node_sptr& negative_ask)
{
}

bool MathematicaVCSInterval::integrate(
  integrate_result_t& integrate_result,
  const positive_asks_t& positive_asks,
  const negative_asks_t& negative_asks,
  const time_t& current_time,
  const time_t& max_time)
{
  return false;
}

void MathematicaVCSInterval::send_cs() const
{
}

void MathematicaVCSInterval::send_cs_vars() const
{
}

} // namespace mathematica
} // namespace simulator
} // namespace hydla 


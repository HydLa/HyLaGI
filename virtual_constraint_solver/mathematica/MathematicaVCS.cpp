#include "MathematicaVCS.h"

#include <cassert>

#include "MathematicaVCSPoint.h"
#include "MathematicaVCSInterval.h"

using namespace hydla::vcs;

namespace hydla {
namespace vcs {
namespace mathematica {

MathematicaVCS::MathematicaVCS(Mode m, MathLink* ml, int approx_precision)
{
  mode_ = m;
  switch(m) {
    case DiscreteMode:
      vcs_.reset(new MathematicaVCSPoint(ml));
      break;

    case ContinuousMode:
      vcs_.reset(new MathematicaVCSInterval(ml, approx_precision));
      break;

    default:
      assert(0);
  }
}

MathematicaVCS::~MathematicaVCS()
{}

bool MathematicaVCS::reset()
{
  return vcs_->reset();
}

bool MathematicaVCS::reset(const variable_map_t& vm)
{
  return vcs_->reset(vm);
}

bool MathematicaVCS::create_variable_map(variable_map_t& vm)
{
  return vcs_->create_variable_map(vm);
}

VCSResult MathematicaVCS::add_constraint(const tells_t& collected_tells)
{
  return vcs_->add_constraint(collected_tells);
}
  
VCSResult MathematicaVCS::check_entailment(const ask_node_sptr& negative_ask)
{
  return vcs_->check_entailment(negative_ask);
}

VCSResult MathematicaVCS::integrate(
  integrate_result_t& integrate_result,
  const positive_asks_t& positive_asks,
  const negative_asks_t& negative_asks,
  const time_t& current_time,
  const time_t& max_time)
{
  return vcs_->integrate(integrate_result, 
                         positive_asks, 
                         negative_asks, 
                         current_time, 
                         max_time);
}

// void MathematicaVCS::change_mode(Mode m)
// {
//   if(mode_ == m) {
//     vcs_->reset();
//   }
//   else {
//     mode_ = m;
//     switch(m) {
//       case DiscreteMode:
//         vcs_.reset(new MathematicaVCSPoint(ml_));
//         break;

//       case ContinuousMode:
//         vcs_.reset(new MathematicaVCSInterval(ml_));
//         break;

//       default:
//         assert(0);
//     }
//   }
// }


void MathematicaVCS::set_output_func(const time_t& max_interval, 
                                     const output_function_t& func)
{
  vcs_->set_output_func(max_interval, func);
}

void MathematicaVCS::reset_output_func()
{
  vcs_->reset_output_func();
}


} // namespace mathematica
} // namespace simulator
} // namespace hydla 

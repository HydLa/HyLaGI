#include "REDUCEVCS.h"

#include <cassert>

#include "REDUCEVCSPoint.h"
#include "REDUCEVCSInterval.h"
//#include "REDUCEStringSender.h"

using namespace hydla::vcs;

namespace hydla {
namespace vcs {
namespace reduce {

void REDUCEVCS::change_mode(hydla::symbolic_simulator::Mode m, int approx_precision)
{
  mode_ = m;
  switch(m) {
    case hydla::symbolic_simulator::DiscreteMode:
      vcs_.reset(new REDUCEVCSPoint(&cl_));
      break;

    case hydla::symbolic_simulator::ContinuousMode:
      vcs_.reset(new REDUCEVCSInterval(&cl_, approx_precision));
      break;

    default:
      assert(0);//assert”­Œ©
  }
}



REDUCEVCS::REDUCEVCS(const hydla::symbolic_simulator::Opts &opts)
{
 

}

REDUCEVCS::~REDUCEVCS()
{}

bool REDUCEVCS::reset()
{
  return vcs_->reset();
}

bool REDUCEVCS::reset(const variable_map_t& vm)
{
  return vcs_->reset(vm);
}

bool REDUCEVCS::reset(const variable_map_t& vm, const parameter_map_t& pm)
{
  return vcs_->reset(vm, pm);
}

bool REDUCEVCS::create_maps(create_result_t &create_result)
{
  return vcs_->create_maps(create_result);
}


VCSResult REDUCEVCS::add_constraint(const tells_t& collected_tells, const appended_asks_t &appended_asks)
{
  return vcs_->add_constraint(collected_tells, appended_asks);
}
  
VCSResult REDUCEVCS::check_entailment(const ask_node_sptr& negative_ask, const appended_asks_t& appended_asks)
{
  return vcs_->check_entailment(negative_ask, appended_asks);
}

VCSResult REDUCEVCS::integrate(
  integrate_result_t& integrate_result,
  const positive_asks_t& positive_asks,
  const negative_asks_t& negative_asks,
  const time_t& current_time,
  const time_t& max_time,
  const not_adopted_tells_list_t& not_adopted_tells_list,
  const appended_asks_t& appended_asks)
{
  return vcs_->integrate(integrate_result, 
                         positive_asks, 
                         negative_asks, 
                         current_time, 
                         max_time,
                         not_adopted_tells_list,
                         appended_asks);
}

// void REDUCEVCS::change_mode(Mode m)
// {
//   if(mode_ == m) {
//     vcs_->reset();
//   }
//   else {
//     mode_ = m;
//     switch(m) {
//       case DiscreteMode:
//         vcs_.reset(new REDUCEVCSPoint(ml_));
//         break;

//       case ContinuousMode:
//         vcs_.reset(new REDUCEVCSInterval(ml_));
//         break;

//       default:
//         assert(0);
//     }
//   }
// }


void REDUCEVCS::apply_time_to_vm(const variable_map_t& in_vm, variable_map_t& out_vm, const time_t& time){
  vcs_->apply_time_to_vm(in_vm, out_vm, time);
}


  //value_t‚ğw’è‚³‚ê‚½¸“x‚Å”’l‚É•ÏŠ·‚·‚é
std::string REDUCEVCS::get_real_val(const value_t &val, int precision){
  std::string ret;

  assert(0);

  return ret;
}


bool REDUCEVCS::less_than(const time_t &lhs, const time_t &rhs)
{

  assert(0);

  return  false;
}


void REDUCEVCS::simplify(time_t &time) 
{
  HYDLA_LOGGER_DEBUG("SymbolicTime::send_time : ", time);

  assert(0);

}



  //SymbolicValue‚ÌŠÔ‚ğ‚¸‚ç‚·
hydla::vcs::SymbolicVirtualConstraintSolver::value_t REDUCEVCS::shift_expr_time(const value_t& val, const time_t& time){
  value_t tmp_val;

  assert(0);

  return  tmp_val;
}


} // namespace reduce
} // namespace vcs
} // namespace hydla 

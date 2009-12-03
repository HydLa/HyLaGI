#ifndef _INCLUDED_HYDLA_BP_SIMULATOR_RP_CONSTRAINT_WRAP_H_
#define _INCLUDED_HYDLA_BP_SIMULATOR_RP_CONSTRAINT_WRAP_H_

#include "rp_constraint.h"

namespace hydla {
namespace bp_simulator {

  class rp_constraint_w
  {
  public:
    rp_constraint_w(rp_ctr_num cnum)
    {
      rp_constraint_create_num(&c, cnum);
    }

    ~rp_constraint_w()
    {
      if(c) rp_constraint_destroy(&c);
    }

    operator rp_constraint(){ return c; }

  private:
    rp_constraint c;
  };

} // namespace bp_simulator
} // namespace hydla

#endif //_INCLUDED_HYDLA_BP_SIMULATOR_RP_CONSTRAINT_WRAP_H_
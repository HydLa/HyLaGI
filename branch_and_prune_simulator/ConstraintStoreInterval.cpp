#include "ConstraintStoreInterval.h"
#include "rp_constraint_ext.h"

namespace hydla {
namespace bp_simulator {

  ConstraintStoreInterval::ConstraintStoreInterval()
  {
  }
  
  ConstraintStoreInterval::~ConstraintStoreInterval()
  {
  }
  
  void ConstraintStoreInterval::build(const variable_map_t& variable_map)
  {
  }

  void ConstraintStoreInterval::add_constraint(rp_constraint c, const var_name_map_t& vars)
  {
  }

  void ConstraintStoreInterval::add_constraint(std::set<rp_constraint>::iterator start, std::set<rp_constraint>::iterator end, const var_name_map_t& vars)
  {
  }

  std::ostream& ConstraintStoreInterval::dump_cs(std::ostream& s) const
  {
    rp_vector_variable vec = this->to_rp_vector();
    std::set<rp_constraint>::const_iterator ctr_it = this->exprs_.begin();
    while(ctr_it != this->exprs_.end()){
      rp::dump_constraint(s, *ctr_it, vec); // digits, mode);
      s << "\n";
      ctr_it++;
    }
    s << "\n";
    rp_vector_destroy(&vec);
    return s;
  }

  rp_vector_variable ConstraintStoreInterval::to_rp_vector() const
  {
    rp_vector_variable vec;
    rp_vector_variable_create(&vec);
    var_name_map_t::right_const_iterator it;
    for(it=this->vars_.right.begin(); it!=this->vars_.right.end(); it++){
      rp_variable v;
      rp_variable_create(&v, ((it->second).c_str()));
      rp_variable_set_decision(v);
      rp_interval interval;
      rp_interval_set(interval,(-1)*RP_INFINITY,RP_INFINITY);
      rp_union_insert(rp_variable_domain(v), interval);
      rp_vector_insert(vec, v);
    }
    return vec;
  }

} // namespace bp_simulator
} // namespace hydla

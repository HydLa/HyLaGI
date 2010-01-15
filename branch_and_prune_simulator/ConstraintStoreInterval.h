#ifndef _INCLUDED_HYDLA_BP_SIMULATOR_CONSTRAINT_STORE_INTERVAL_H_
#define _INCLUDED_HYDLA_BP_SIMULATOR_CONSTRAINT_STORE_INTERVAL_H_

#include <set>
#include <iostream>
#include "realpaverbasic.h"

#include "BPTime.h"
#include "BPTypes.h"

namespace hydla {
namespace bp_simulator {

class ConstraintStoreInterval {
public:
  ConstraintStoreInterval();
  virtual ~ConstraintStoreInterval();
  void build(const variable_map_t& variable_map);
  void add_constraint(rp_constraint c, const var_name_map_t& vars);
  void add_constraint(std::set<rp_constraint>::iterator start, std::set<rp_constraint>::iterator end, const var_name_map_t& vars);
  const var_name_map_t& get_store_vars() const
  {
    return this->vars_;
  }
  std::ostream& dump_cs(std::ostream& s) const;
  friend std::ostream& operator<<(std::ostream& s, const ConstraintStoreInterval& cs)
  {
    return cs.dump_cs(s);
  }

private:
  rp_vector_variable to_rp_vector() const;
  void add_vm_constraint(std::string var, rp_interval val, const int op);
  std::set<rp_constraint> exprs_;
  var_name_map_t vars_;
};

} // namespace bp_simulator
} // namespace hydla

#endif //_INCLUDED_HYDLA_BP_SIMULATOR_CONSTRAINT_STORE_INTERVAL_H_
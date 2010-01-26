#ifndef _INCLUDED_HYDLA_VCS_REALPAVER_RP_CONSTRAINT_STORE_H_
#define _INCLUDED_HYDLA_VCS_REALPAVER_RP_CONSTRAINT_STORE_H_

#include "RPVCSType.h"

#include <set>
#include <iostream>
#include "rp_constraint.h"

namespace hydla {
namespace vcs {
namespace realpaver {

typedef std::set<rp_constraint> ctr_set_t;

/**
 * §–ñƒXƒgƒA
 */
class ConstraintStore
{
public:
  ConstraintStore(const double prec=0.5);

  ConstraintStore(const ConstraintStore& src);

  ~ConstraintStore();

  ConstraintStore& operator=(const ConstraintStore& src);

  void build(const virtual_constraint_solver_t::variable_map_t& variable_map);

  void build_variable_map(virtual_constraint_solver_t::variable_map_t& variable_map) const;

  ctr_set_t get_store_exprs_copy() const;

  void add_constraint(rp_constraint c, const var_name_map_t& vars);

  void add_constraint(ctr_set_t::iterator start, ctr_set_t::iterator end, const var_name_map_t& vars);

  const var_name_map_t& get_store_vars() const
  {
    return this->vars_;
  }

  std::ostream& dump_cs(std::ostream& s) const;

  friend std::ostream& operator<<(std::ostream& s, const ConstraintStore& cs)
  {
    return cs.dump_cs(s);
  }

  void set_precision(const double p);

private:

  ctr_set_t exprs_;
  var_name_map_t vars_;
  double prec_;
};

} // namespace realpaver
} // namespace vcs
} // namespace hydla 

#endif // _INCLUDED_HYDLA_VCS_REALPAVER_RP_CONSTRAINT_STORE_H_
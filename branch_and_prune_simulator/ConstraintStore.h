#ifndef _INCLUDED_HYDLA_BP_SIMULATOR_CONSTRAINT_STORE_H_
#define _INCLUDED_HYDLA_BP_SIMULATOR_CONSTRAINT_STORE_H_

#include <set>
#include <boost/bimap/bimap.hpp>
#include "realpaverbasic.h"

#include "BPTime.h"
#include "BPTypes.h"

namespace hydla {
namespace bp_simulator {

  typedef boost::bimaps::bimap<std::string, int> var_name_map_t;

/**
 * êßñÒÉXÉgÉA
 */
class ConstraintStore
{
public:
  ConstraintStore(bool debug_mode = false);
  ~ConstraintStore();
  void build(const variable_map_t& variable_map);
  const std::set<rp_constraint>& get_store_exprs() const
  {
    return this->exprs_;
  }
  std::set<rp_constraint> get_store_exprs_copy() const;
  void add_constraint(rp_constraint c, var_name_map_t vars);
  void add_constraint(std::set<rp_constraint>::iterator start, std::set<rp_constraint>::iterator end, var_name_map_t vars);
  const var_name_map_t& get_store_vars() const
  {
    return this->vars_;
  };

private:
  std::set<rp_constraint> exprs_;
  var_name_map_t vars_;
  bool debug_mode_;
};

} // namespace bp_simulator
} // namespace hydla 

#endif // _INCLUDED_HYDLA_BP_SIMULATOR_CONSTRAINT_STORE_H_
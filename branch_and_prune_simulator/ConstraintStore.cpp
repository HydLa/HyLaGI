#include "ConstraintStore.h"

namespace hydla {
namespace bp_simulator {

  ConstraintStore::ConstraintStore(bool debug_mode) :
  debug_mode_(debug_mode)
  {}

  ConstraintStore::~ConstraintStore()
  {}

  // TODO: ÇªÇÃÇ§ÇøèëÇ≠
  void ConstraintStore::build(const variable_map_t& variable_map)
  {
  }


  std::set<rp_constraint> ConstraintStore::get_store_exprs_copy() const
  {
    std::set<rp_constraint> ans;
    std::set<rp_constraint>::const_iterator it = this->exprs_.begin();
    while(it != this->exprs_.end()) {
      rp_constraint c;
      // TODO: ä‘à·Ç¡ÇƒÇ¢ÇÈâ¬î\ê´
      rp_constraint_clone(&c, (*it));
      it++;
    }
    return ans;
  }

  void ConstraintStore::add_constraint(rp_constraint c, var_name_map_t vars)
  {
    this->exprs_.insert(c);
    this->vars_.insert(vars.begin(), vars.end());
  }

  void ConstraintStore::add_constraint(std::set<rp_constraint>::iterator start,
                                       std::set<rp_constraint>::iterator end,
                                       var_name_map_t vars)
  {
    this->exprs_.insert(start, end);
    this->vars_.insert(vars.begin(), vars.end());
  }

} // namespace bp_simulator
} // namespace hydla

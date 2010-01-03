#include "ConstraintStore.h"

namespace hydla {
namespace bp_simulator {

  ConstraintStore::ConstraintStore()
  {}

  ConstraintStore::~ConstraintStore()
  {}

  std::set<rp_constraint>& ConstraintStore::get_store_exprs()
  {
    return this->exprs_;
  }

  std::set<rp_constraint> ConstraintStore::get_store_exprs_copy()
  {
    std::set<rp_constraint> ans;
    std::set<rp_constraint>::iterator it = this->exprs_.begin();
    while(it != this->exprs_.end()) {
      rp_constraint c;
      // TODO: ŠÔˆá‚Á‚Ä‚¢‚é‰Â”\«
      rp_constraint_clone(&c, (*it));
      it++;
    }
    return ans;
  }

  boost::bimaps::bimap<std::string, int>& ConstraintStore::get_store_vars()
  {
    return this->vars_;
  }

} // namespace bp_simulator
} // namespace hydla

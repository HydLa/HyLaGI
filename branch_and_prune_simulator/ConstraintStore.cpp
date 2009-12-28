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

  boost::bimaps::bimap<std::string, int>& ConstraintStore::get_store_vars()
  {
    return this->vars_;
  }

} // namespace bp_simulator
} // namespace hydla

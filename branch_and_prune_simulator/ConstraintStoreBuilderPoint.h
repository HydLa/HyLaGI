#ifndef _INCLUDED_HYDLA_BP_SIMULATOR_CONSTRAINT_STORE_BUILDER_POINT_H_
#define _INCLUDED_HYDLA_BP_SIMULATOR_CONSTRAINT_STORE_BUILDER_POINT_H_

#include "BPTime.h"
#include "BPTypes.h"
#include "ConstraintStore.h"

namespace hydla {
namespace bp_simulator {

class ConstraintStoreBuilderPoint
{
public:
  ConstraintStoreBuilderPoint(bool debug_mode = false);

  virtual ~ConstraintStoreBuilderPoint();

  void build_constraint_store(const variable_map_t& variable_map);

  //void build_variable_map(variable_map_t variable_map);

  ConstraintStore& getcs();

private:
  ConstraintStore constraint_store_;
  bool debug_mode_;

};


} // namespace bp_simulator
} // namespace hydla 

#endif // _INCLUDED_HYDLA_BP_SIMULATOR_CONSTRAINT_STORE_BUILDER_POINT_H_
#ifndef _INCLUDED_HYDLA_CONSTRAINT_STORE_BUILDER_POINT_H_
#define _INCLUDED_HYDLA_CONSTRAINT_STORE_BUILDER_POINT_H_
#include "VariableMap.h"
#include "SymbolicVariable.h"
#include "SymbolicValue.h"
#include "MathSimulator.h"

namespace hydla {
namespace symbolic_simulator {

/**
 * êßñÒÉXÉgÉA
 */

typedef variable_map_t ConstraintStore;
//typedef variable_map_t::variable_list_t ConstraintStore;

// class ConstraintStoreBuilderPoint : public VariableMap<SymbolicVariable, SymbolicValue>
class ConstraintStoreBuilderPoint
{
public:
  ConstraintStoreBuilderPoint();

  virtual ~ConstraintStoreBuilderPoint();

  void build_constraint_store( /*variable_map_t variable_map*/ );

  void build_variable_map(variable_map_t variable_map);

  ConstraintStore getcs();

private:
  ConstraintStore constraint_store_;

};


} // namespace symbolic_simulator
} // namespace hydla 

#endif // _INCLUDED_HYDLA_CONSTRAINT_STORE_BUILDER_POINT_H_


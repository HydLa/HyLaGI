#ifndef _INCLUDED_HYDLA_CONSTRAINT_STORE_BUILDER_POINT_H_
#define _INCLUDED_HYDLA_CONSTRAINT_STORE_BUILDER_POINT_H_

#include "VariableMap.h"
#include "SymbolicVariable.h"
#include "SymbolicValue.h"
#include "MathSimulator.h"

namespace hydla {
namespace symbolic_simulator {

/**
 * 制約ストア
 */

// その時点での制約ストア内に出現する変数の一覧も持つ
typedef std::pair<SymbolicValue, SymbolicValue> ConstraintStore;
//typedef variable_map_t ConstraintStore;

class ConstraintStoreBuilderPoint
{
public:
  ConstraintStoreBuilderPoint(MathLink& ml, bool debug_mode = true);

  virtual ~ConstraintStoreBuilderPoint();

  void build_constraint_store(variable_map_t& variable_map);

  void build_variable_map(variable_map_t& variable_map);

  ConstraintStore& get_constraint_store()
  {
    return constraint_store_;
  };

private:
  ConstraintStore constraint_store_;
  MathLink& ml_;
  /// デバッグ出力をするかどうか
  bool               debug_mode_;

};


} // namespace symbolic_simulator
} // namespace hydla 

#endif // _INCLUDED_HYDLA_CONSTRAINT_STORE_BUILDER_POINT_H_


#ifndef _INCLUDED_HYDLA_CONSTRAINT_STORE_BUILDER_INTERVAL_H_
#define _INCLUDED_HYDLA_CONSTRAINT_STORE_BUILDER_INTERVAL_H_

#include "VariableMap.h"
#include "SymbolicVariable.h"
#include "SymbolicValue.h"
#include "MathSimulator.h"

namespace hydla {
namespace symbolic_simulator {

/**
 * ����X�g�A
 */

// ���̎��_�ł̐���X�g�A���ɏo������ϐ��̈ꗗ������
typedef std::pair<std::set<std::set<SymbolicValue> >, std::set<SymbolicVariable> > ConstraintStoreInterval;
//typedef std::pair<SymbolicValue, SymbolicValue> ConstraintStoreInterval;

class ConstraintStoreBuilderInterval
{
public:
  ConstraintStoreBuilderInterval(bool debug_mode = true);

  virtual ~ConstraintStoreBuilderInterval();

  void build_constraint_store(const variable_map_t& variable_map);

  void build_variable_map(variable_map_t& variable_map);

  ConstraintStoreInterval& get_constraint_store()
  {
    return constraint_store_;
  }

private:
  ConstraintStoreInterval constraint_store_;
  /// �f�o�b�O�o�͂����邩�ǂ���
  bool               debug_mode_;

};


} // namespace symbolic_simulator
} // namespace hydla 

#endif // _INCLUDED_HYDLA_CONSTRAINT_STORE_BUILDER_INTERVAL_H_


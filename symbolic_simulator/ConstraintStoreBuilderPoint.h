#ifndef _INCLUDED_HYDLA_CONSTRAINT_STORE_BUILDER_POINT_H_
#define _INCLUDED_HYDLA_CONSTRAINT_STORE_BUILDER_POINT_H_

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
typedef std::pair<SymbolicValue, SymbolicValue> ConstraintStore;
//typedef variable_map_t ConstraintStore;

class ConstraintStoreBuilderPoint
{
public:
  ConstraintStoreBuilderPoint(MathLink& ml, bool debug_mode);

  virtual ~ConstraintStoreBuilderPoint();

  void build_constraint_store( /*variable_map_t variable_map*/ );

  variable_map_t build_variable_map();

  ConstraintStore& getcs();

private:
  ConstraintStore constraint_store_;
  MathLink& ml_;
  /// �f�o�b�O�o�͂����邩�ǂ���
  bool               debug_mode_;

};


} // namespace symbolic_simulator
} // namespace hydla 

#endif // _INCLUDED_HYDLA_CONSTRAINT_STORE_BUILDER_POINT_H_


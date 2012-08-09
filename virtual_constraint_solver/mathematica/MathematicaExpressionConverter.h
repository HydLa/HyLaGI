#ifndef _INCLUDED_HYDLA_VCS_MATHEMATICA_EXPRESSION_CONVERTER_H_
#define _INCLUDED_HYDLA_VCS_MATHEMATICA_EXPRESSION_CONVERTER_H_


//Mathematica�������SymbolicValue�Ƃ����C���̕ϊ���S������N���X�DSymbolicValueRange�������D


#include "../SymbolicVirtualConstraintSolver.h"
#include "PacketSender.h"
#include <map>
#include "TreeVisitor.h"
#include "ParseTree.h"

namespace hydla {
namespace vcs {
namespace mathematica {


class MathematicaExpressionConverter
{
  private:
  typedef hydla::vcs::SymbolicVirtualConstraintSolver::value_t value_t;
  typedef hydla::vcs::SymbolicVirtualConstraintSolver::value_range_t value_range_t;
  typedef hydla::vcs::SymbolicVirtualConstraintSolver::variable_t variable_t;
  typedef hydla::vcs::SymbolicVirtualConstraintSolver::parameter_t parameter_t;
  
  public:
  
  MathematicaExpressionConverter(){}
  virtual ~MathematicaExpressionConverter(){}

  typedef hydla::parse_tree::node_sptr node_sptr;

  //������
  static void initialize();

  /**
   * ��M����value�ɕϊ�����
   */
  static node_sptr receive_and_make_symbolic_value(MathLink &ml);

  //val�Ɗ֌W���Z�q�����ɁArange��ݒ肷��
  static void set_range(const value_t &val, value_range_t &range, const int& relop);
};

} // namespace mathematica
} // namespace vcs
} // namespace hydla 

#endif //_INCLUDED_HYDLA_VCS_MATHEMATICA_EXPRESSION_CONVERTER_H_

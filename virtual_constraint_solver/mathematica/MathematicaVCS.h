#ifndef _INCLUDED_HYDLA_VCS_MATHEMATICA_VCS_H_
#define _INCLUDED_HYDLA_VCS_MATHEMATICA_VCS_H_

#include <boost/scoped_ptr.hpp>

#include "VirtualConstraintSolver.h"

#inlucde "MathVariable.h"
#inlucde "MathValue.h"
#inlucde "MathTime.h"

namespace hydla {
namespace vcs {
namespace mathematica {

typedef hydla::vcs::VirtualConstraintSolver<
  MathVariable, MathValue, MathTime> virtual_constraint_solver_t;

class MathematicaVCSPoint;
class MathematicaVCSInterval;

class MathematicaVCS : 
    public virtual_constraint_solver_t
{
public:
  enum Mode {
    ContinuousMode,
    DiscreteMode,
  }

  MathematicaVCS(Mode m);

  virtual ~MathematicaVCS();

  /**
   * ���󥹥ȥ��ν�����򤪤��ʤ�
   */
  bool reset();

  /**
   * Ϳ����줿�ѿ�ɽ�򸵤ˡ����󥹥ȥ��ν�����򤪤��ʤ�
   */
  bool reset(const variable_map_t& vm);

  /**
   * ���ߤ����󥹥ȥ������ѿ�ɽ���������
   */
  bool create_variable_map(variable_map_t& vm);

  /**
   * ������ɲä���
   */
  Trivalent add_constraint(const tells_t& collected_tells);
  
  /**
   * ���ߤ����󥹥ȥ�����Ϳ����ask��Ƴ�в�ǽ���ɤ���
   */
  Trivalent check_entailment(const boost::shared_ptr<Ask>& negative_ask);

  /**
   * ask��Ƴ�о��֤��Ѳ�����ޤ���ʬ�򤪤��ʤ�
   */
  bool integrate(
    integrate_result_t& integrate_result,
    const positive_asks_t& positive_asks,
    const negative_asks_t& negative_asks,
    const time_t& current_time,
    const time_t& max_time);

private:
  Mode mode_;

  boost::scoped_ptr<virtual_constraint_solver_t> vcs_;
};

} // namespace mathematica
} // namespace simulator
} // namespace hydla 

#endif // _INCLUDED_HYDLA_VCS_MATHEMATICA_VCS_H_

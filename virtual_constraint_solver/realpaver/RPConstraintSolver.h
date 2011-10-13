#ifndef _INCLUDED_HYDLA_VCS_REALPAVER_RP_CONSTRAINT_SOLVER_H_
#define _INCLUDED_HYDLA_VCS_REALPAVER_RP_CONSTRAINT_SOLVER_H_

#include "RPVCSType.h"
#include <set>
#include <map>
#include "rp_interval.h"
#include "rp_constraint.h"
#include "rp_box.h"
#include "rp_variable.h"

namespace hydla {
namespace vcs {
namespace realpaver {

class ConstraintSolver
{
public:
  /**
   * �ȑf�Ȏ�@��ctrs�̉������ׂĊ܂�box�����߂�
   * @param result    ���߂�box��������D
   *                  ���ʂ�false�̏ꍇ���͖���`(free����K�v�͂Ȃ�)
   * @param vars      �ϐ����
   * @param ctrs      ����
   * @param precision box�̐��x(�g�p����Ȃ�)
   * @return �������݂��邩�ǂ���
   */
  static bool solve_hull(rp_box* result,
                        const var_name_map_t& vars,
                        const std::set<rp_constraint>& ctrs,
                        const double precision=1.0e-8);

  /**
   * BranchAndPrune�𗘗p���Ė�������������Hull�����߂�
   */
  static bool solve_exact_hull(rp_box* result,
                               const var_name_map_t& vars,
                               const std::set<rp_constraint>& ctrs,
                               const double precision=1.0e-8);

  /**
   * vars����rp_vector�����
   */
  static rp_vector_variable create_rp_vector(const var_name_map_t& vars,
                                             const double precision=1.0e-8);


  static rp_vector_variable create_rp_vector(const var_name_map_t& vars,
    const std::map<std::string, std::pair<double, double> >& domain,
    const double precision=1.0e-8);

  /**
   * �S�Ă̗v�f��(-oo,+oo)�ł���vars�Ɠ����T�C�Y��box�����
   */
  static void create_initial_box(rp_box* b, const var_name_map_t& vars);

};

} // namespace realapver
} // namespace vcs
} // namespace hydla 

#endif // _INCLUDED_HYDLA_VCS_REALPAVER_RP_CONSTRAINT_SOLVER_H_
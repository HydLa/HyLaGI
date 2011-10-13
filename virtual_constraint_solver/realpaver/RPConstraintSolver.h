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
   * 簡素な手法でctrsの解をすべて含むboxを求める
   * @param result    求めたboxを代入する．
   *                  結果がfalseの場合中は未定義(freeする必要はない)
   * @param vars      変数情報
   * @param ctrs      制約
   * @param precision boxの精度(使用されない)
   * @return 解が存在するかどうか
   */
  static bool solve_hull(rp_box* result,
                        const var_name_map_t& vars,
                        const std::set<rp_constraint>& ctrs,
                        const double precision=1.0e-8);

  /**
   * BranchAndPruneを利用して問題を解き答えのHullを求める
   */
  static bool solve_exact_hull(rp_box* result,
                               const var_name_map_t& vars,
                               const std::set<rp_constraint>& ctrs,
                               const double precision=1.0e-8);

  /**
   * varsからrp_vectorを作る
   */
  static rp_vector_variable create_rp_vector(const var_name_map_t& vars,
                                             const double precision=1.0e-8);


  static rp_vector_variable create_rp_vector(const var_name_map_t& vars,
    const std::map<std::string, std::pair<double, double> >& domain,
    const double precision=1.0e-8);

  /**
   * 全ての要素が(-oo,+oo)であるvarsと同じサイズのboxを作る
   */
  static void create_initial_box(rp_box* b, const var_name_map_t& vars);

};

} // namespace realapver
} // namespace vcs
} // namespace hydla 

#endif // _INCLUDED_HYDLA_VCS_REALPAVER_RP_CONSTRAINT_SOLVER_H_
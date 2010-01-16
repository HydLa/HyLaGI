#ifndef _INCLUDED_HYDLA_VCS_MATHEMATICA_VCS_H_
#define _INCLUDED_HYDLA_VCS_MATHEMATICA_VCS_H_

#include <boost/scoped_ptr.hpp>

#include "VirtualConstraintSolver.h"

#include "MathVariable.h"
#include "MathValue.h"
#include "MathTime.h"

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
  };

  MathematicaVCS(Mode m);

  virtual ~MathematicaVCS();

  /**
   * 制約ストアの初期化をおこなう
   */
  virtual bool reset();

  /**
   * 与えられた変数表を元に，制約ストアの初期化をおこなう
   */
  virtual bool reset(const variable_map_t& vm);

  /**
   * 現在の制約ストアから変数表を作成する
   */
  virtual bool create_variable_map(variable_map_t& vm);

  /**
   * 制約を追加する
   */
  virtual Trivalent add_constraint(const tells_t& collected_tells);
  
  /**
   * 現在の制約ストアから与えたaskが導出可能かどうか
   */
  virtual Trivalent check_entailment(const ask_node_sptr& negative_ask);

  /**
   * askの導出状態が変化するまで積分をおこなう
   */
  virtual bool integrate(
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

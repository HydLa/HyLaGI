#ifndef _INCLUDED_HYDLA_VCS_MATHEMATICA_VCS_H_
#define _INCLUDED_HYDLA_VCS_MATHEMATICA_VCS_H_

/**
 * プログラム間の依存性の問題から，
 * このヘッダーおよびこのヘッダーからインクルードされるヘッダーにおいて
 * ソルバー依存のヘッダー(mathematicaやrealpaver等の固有のヘッダー)を
 * インクルードしてはならない
 */

#include <boost/scoped_ptr.hpp>

#include "MathVCSType.h"

class MathLink;

namespace hydla {
namespace vcs {
namespace mathematica {

class MathematicaVCS : 
    public virtual_constraint_solver_t
{
public:
  enum Mode {
    ContinuousMode,
    DiscreteMode,
  };

  MathematicaVCS(Mode m, MathLink* ml, int approx_precision);

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
  virtual VCSResult add_constraint(const tells_t& collected_tells);
  
  /**
   * 現在の制約ストアから与えたaskが導出可能かどうか
   */
  virtual VCSResult check_entailment(const ask_node_sptr& negative_ask);

  /**
   * askの導出状態が変化するまで積分をおこなう
   */
  virtual VCSResult integrate(
    integrate_result_t& integrate_result,
    const positive_asks_t& positive_asks,
    const negative_asks_t& negative_asks,
    const time_t& current_time,
    const time_t& max_time);

  /**
   * 現在のモードを返す
   */
  Mode get_mode() const {
    return mode_;
  }

  /**
   * モードを変更する
   * 制約ストア等のすべての内部状態はすべて初期化される
   */
//  void change_mode(Mode m);

  /**
   * 結果の出力関数を設定する
   */
  virtual void set_output_func(const time_t& max_interval, 
                               const output_function_t& func);

  /**
   * 結果の出力関数の設定をリセットし，初期状態に戻す
   */
  virtual void reset_output_func();

private:
  Mode      mode_;
//  MathLink* ml_;

  boost::scoped_ptr<virtual_constraint_solver_t> vcs_;
};

} // namespace mathematica
} // namespace simulator
} // namespace hydla 

#endif // _INCLUDED_HYDLA_VCS_MATHEMATICA_VCS_H_

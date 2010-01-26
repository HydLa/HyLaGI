#ifndef _INCLUDED_HYDLA_VCS_REALPAVER_REALPAVER_VCS_H_
#define _INCLUDED_HYDLA_VCS_REALPAVER_REALPAVER_VCS_H_

/**
 * プログラム間の依存性の問題から，
 * このヘッダーおよびこのヘッダーからインクルードされるヘッダーにおいて
 * ソルバー依存のヘッダー(mathematicaやrealpaver等の固有のヘッダー)を
 * インクルードしてはならない
 */

#include <boost/scoped_ptr.hpp>

//#include "RPVCSType.h"
#include "RealPaverBaseVCS.h"

class MathLink;

namespace hydla {
namespace vcs {
namespace realpaver {

class RealPaverVCS : 
    public RealPaverBaseVCS
{
public:
  enum Mode {
    ContinuousMode,
    DiscreteMode,
  };

  RealPaverVCS(Mode m);
  RealPaverVCS(Mode m, MathLink* ml);
  RealPaverVCS(const RealPaverVCS& src);

  virtual ~RealPaverVCS();

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
   * モードを返す
   */
  Mode get_mode() const {
    return mode_;
  }

/******************** realpaver only ********************/

  virtual void add_single_constraint(const node_sptr& constraint_node,
    const bool neg_expression=false);

  virtual void set_precision(const double p);

/******************** realpaver only ********************/

private:
  virtual RealPaverBaseVCS* clone(); //コピー禁止

  Mode mode_;

  boost::scoped_ptr<RealPaverBaseVCS> vcs_;
};

} // namespace realpaver
} // namespace vcs
} // namespace hydla 

#endif // _INCLUDED_HYDLA_VCS_REALPAVER_REALPAVER_VCS_H_

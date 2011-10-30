#ifndef _INCLUDED_HYDLA_VCS_MATHEMATICA_VCS_INTERVAL_H_
#define _INCLUDED_HYDLA_VCS_MATHEMATICA_VCS_INTERVAL_H_

#include <map>
#include <set>
#include <vector>

#include "mathlink_helper.h"

#include "MathVCSType.h"
#include "PacketSender.h"

namespace hydla {
namespace vcs {
namespace mathematica {

class MathematicaVCSInterval : 
    public virtual_constraint_solver_t
{
public:
  typedef hydla::simulator::constraints_t        constraints_t;
  /**
   * @param approx_precision 近似する精度 値が負の場合は近似を行わない
   */
  MathematicaVCSInterval(MathLink* ml, int approx_precision);

  virtual ~MathematicaVCSInterval();

  
  /**
   * 制約ストアが無矛盾かを判定する．
   * 引数で制約を渡された場合は一時的に制約ストアに追加する．
   */
  virtual VCSResult check_consistency();
  virtual VCSResult check_consistency(const constraints_t& constraints);


  /**
   * askの導出状態が変化するまで積分をおこなう
   */
  virtual VCSResult integrate(
    integrate_result_t& integrate_result,
    const constraints_t &constraints,
    const time_t& current_time,
    const time_t& max_time);

  /**
   * 内部状態の出力をおこなう
   */
  std::ostream& dump(std::ostream& s) const;
  
  
  /**
   * 与えられたmapを元に，各変数の連続性を設定する．
   */
  virtual void set_continuity(const continuity_map_t& continuity_map);

private:
  // check_consistencyの共通部分
  VCSResult check_consistency_sub(const constraints_t &);
  

  /**
   * 初期値制約をMathematicaに渡す
   * 
   */
  void send_init_cons(PacketSender& ps, const continuity_map_t& continuity_map);

  /*
   * 時刻を送信する
   */
  void send_time(const time_t& time);
  
  //記号定数のリストを送る
  void send_pars() const;

  mutable MathLink* ml_;
  continuity_map_t continuity_map_;
  int approx_precision_;
};

} // namespace mathematica
} // namespace simulator
} // namespace hydla 

#endif // _INCLUDED_HYDLA_VCS_MATHEMATICA_VCS_INTERVAL_H_

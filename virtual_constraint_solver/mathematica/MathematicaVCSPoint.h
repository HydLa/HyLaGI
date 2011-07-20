#ifndef _INCLUDED_HYDLA_VCS_MATHEMATICA_VCS_POINT_H_
#define _INCLUDED_HYDLA_VCS_MATHEMATICA_VCS_POINT_H_

#include <ostream>

#include "mathlink_helper.h"
#include "MathVCSType.h"
#include "PacketSender.h"
#include "MathematicaExpressionConverter.h"

namespace hydla {
namespace vcs {
namespace mathematica {

class MathematicaVCSPoint : 
    public virtual_constraint_solver_t
{
public:
  typedef std::set<PacketSender::var_info_t> constraint_store_vars_t;
  
  typedef std::pair<std::set<std::set<MathValue> >, 
                    constraint_store_vars_t> constraint_store_t;

  MathematicaVCSPoint(MathLink* ml);

  virtual ~MathematicaVCSPoint();

  /**
   * 制約ストアの初期化をおこなう
   */
  virtual bool reset();

  /**
   * 与えられた変数表を元に，制約ストアの初期化をおこなう
   */
  virtual bool reset(const variable_map_t& variable_map);

  /**
   * 与えられた変数表と定数表を元に，制約ストアの初期化をおこなう
   */
  virtual bool reset(const variable_map_t& vm, const parameter_map_t& pm);

  /**
   * 現在の制約ストアから変数表を作成する
   */
  virtual bool create_maps(create_result_t & create_result);

  /**
   * 制約を追加する．ついでに制約ストアが無矛盾かを判定する．
   */
  virtual void add_constraint(const constraints_t& constraints);
  
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

private:
  typedef std::map<std::string, int> max_diff_map_t;

  void reset_sub(const variable_map_t& vm, std::set<MathValue>& and_cons_set,
    MathematicaExpressionConverter& mec, const bool& is_current);
  void send_cs() const;
  void send_ps() const;
  void send_store(const constraint_store_t& store) const;
  void send_cs_vars() const;
  void send_pars() const;
  void receive_constraint_store(constraint_store_t& store);
  /**
   * check_consistency の共通部分
   */
  VCSResult check_consistency_sub();

  /**
   * 左連続性に関する制約を加える
   */
  void add_left_continuity_constraint(PacketSender& ps, max_diff_map_t& max_diff_map);

  /**
   * 制約ストアがtrueであるかどうか
   */
  bool cs_is_true()
  {
    return constraint_store_.first.size()==1 && 
      (*constraint_store_.first.begin()).size() == 1 &&
      (*(*constraint_store_.first.begin()).begin()).get_string()=="True";
  }

  mutable MathLink* ml_;
  max_diff_map_t max_diff_map_;          //現在の制約ストアに出現する中で最大の微分回数を記録しておく表．
  constraint_store_t constraint_store_;
  constraint_store_t parameter_store_;
  constraints_t tmp_constraints_;  //一時的に制約を追加する対象
  std::set<std::string> par_names_; //一時しのぎ
};

std::ostream& operator<<(std::ostream& s, const MathematicaVCSPoint& m);

} // namespace mathematica
} // namespace simulator
} // namespace hydla 

#endif // _INCLUDED_HYDLA_VCS_MATHEMATICA_VCS_POINT_H_

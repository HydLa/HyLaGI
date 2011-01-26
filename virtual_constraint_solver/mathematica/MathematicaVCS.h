#ifndef _INCLUDED_HYDLA_VCS_MATHEMATICA_VCS_H_
#define _INCLUDED_HYDLA_VCS_MATHEMATICA_VCS_H_


#include <boost/scoped_ptr.hpp>

#include "MathVCSType.h"
#include "mathlink_helper.h"
#include "vcs_math_source.h"

class MathLink;

namespace hydla {
namespace vcs {
namespace mathematica {

class MathematicaVCS : 
    public virtual_constraint_solver_t
{
public:


  //MathematicaVCS(Mode m, MathLink* ml, int approx_precision);
  MathematicaVCS(const hydla::symbolic_simulator::Opts &opts);

  virtual ~MathematicaVCS();
  
  //ソルバのモードを変更する
  virtual void change_mode(hydla::symbolic_simulator::Mode m, int approx_precision);

  /**
   * 制約ストアの初期化をおこなう
   */
  virtual bool reset();

  /**
   * 与えられた変数表を元に，制約ストアの初期化をおこなう
   */
  virtual bool reset(const variable_map_t& vm);
 
  /**
   * 与えられた変数表と定数表を元に，制約ストアの初期化をおこなう
   */
  virtual bool reset(const variable_map_t& vm, const parameter_map_t& pm);
  

  /**
   * 現在の制約ストアから変数表を作成する
   */
  virtual bool create_variable_map(variable_map_t& vm);
  
  /**
   * 制約を追加する
   */
  virtual VCSResult add_constraint(const tells_t& collected_tells, const appended_asks_t& appended_asks);
  
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
    const time_t& max_time,
    const not_adopted_tells_list_t& not_adopted_tells_list);



  virtual void apply_time_to_vm(const variable_map_t& in_vm, variable_map_t& out_vm, const time_t& time);

  //SymbolicValueを指定された精度で数値に変換する
  virtual std::string get_real_val(const value_t &val, int precision);
  //SymbolicTimeを指定された精度で数値に変換する
  virtual std::string get_real_val(const time_t &val, int precision);
  //element_valueを指定された精度で数値に変換する
  virtual std::string get_real_val(const element_value_t &val, int precision);
  //SymbolicTimeを簡約する
  virtual void simplify(time_t &time);
  //SymbolicTimeを比較する
  virtual bool less_than(const time_t &lhs, const time_t &rhs);

private:
  hydla::symbolic_simulator::Mode      mode_;

  MathLink ml_;
  boost::scoped_ptr<virtual_constraint_solver_t> vcs_;
};

} // namespace mathematica
} // namespace simulator
} // namespace hydla 

#endif // _INCLUDED_HYDLA_VCS_MATHEMATICA_VCS_H_

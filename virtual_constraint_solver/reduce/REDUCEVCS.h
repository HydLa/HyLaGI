#ifndef _INCLUDED_HYDLA_VCS_REDUCE_VCS_H_
#define _INCLUDED_HYDLA_VCS_REDUCE_VCS_H_


#include <boost/scoped_ptr.hpp>

#include "REDUCEVCSType.h"
#include "REDUCELink.h"
#include "vcs_reduce_source.h"



namespace hydla {
namespace vcs {
namespace reduce {

class REDUCEVCS : 
    public virtual_constraint_solver_t
{
public:


  //REDUCEVCS(Mode m, REDUCELink* cl, int approx_precision);
  REDUCEVCS(const hydla::symbolic_simulator::Opts &opts, variable_map_t &vm);

  virtual ~REDUCEVCS();
  
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
  virtual bool create_maps(create_result_t &create_result);
  
  /**
   * 制約を追加する
   */
  virtual void add_constraint(const constraints_t& constraints);

  /**
   * 現在の制約ストアから与えたaskが導出可能かどうか
   */
  virtual VCSResult check_entailment(const ask_node_sptr& negative_ask, const appended_asks_t& appended_asks);

  /**
   * 制約ストアが無矛盾かを判定する．
   * 引数で制約を渡された場合は一時的に制約ストアに追加する．
   */
  virtual VCSResult check_consistency(const constraints_t& constraints);
  virtual VCSResult check_consistency();

  /**
   * askの導出状態が変化するまで積分をおこなう
   */
  virtual VCSResult integrate(
    integrate_result_t& integrate_result,
    const constraints_t &constraints,
    const time_t& current_time,
    const time_t& max_time);



  virtual void apply_time_to_vm(const variable_map_t& in_vm, variable_map_t& out_vm, const time_t& time);

  //SymbolicValueを指定された精度で数値に変換する
  virtual std::string get_real_val(const value_t &val, int precision);
  //SymbolicTimeを簡約する
  virtual void simplify(time_t &time);
  //SymbolicTimeを比較する
  virtual bool less_than(const time_t &lhs, const time_t &rhs);
  //SymbolicValueの時間をずらす
  virtual value_t shift_expr_time(const value_t &val, const time_t &time);

private:
  hydla::symbolic_simulator::Mode      mode_;

  REDUCELink cl_;
  boost::scoped_ptr<virtual_constraint_solver_t> vcs_;
};

} // namespace reduce
} // namespace simulator
} // namespace hydla 

#endif // _INCLUDED_HYDLA_VCS_REDUCE_VCS_H_

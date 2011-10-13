#ifndef _INCLUDED_HYDLA_VCS_REDUCE_VCS_POINT_H_
#define _INCLUDED_HYDLA_VCS_REDUCE_VCS_POINT_H_

#include <ostream>

#include "REDUCELink.h"
#include "REDUCEVCSType.h"
#include "REDUCEStringSender.h"

#include "../../parser/SExpParser.h"

using namespace hydla::parser;

namespace hydla {
namespace vcs {
namespace reduce {

class REDUCEVCSPoint :
public virtual_constraint_solver_t
{
public:
  typedef SExpParser::const_tree_iter_t const_tree_iter_t;

  typedef std::set<REDUCEStringSender::var_info_t> constraint_store_vars_t;

  typedef std::pair<std::set<std::set<REDUCEValue> >, constraint_store_vars_t> constraint_store_t;
//  typedef std::pair<std::set<const_tree_iter_t>, constraint_store_vars_t> constraint_store_t;

  typedef std::pair<std::set<std::set<REDUCEValue> >,
      constraint_store_vars_t> parameter_store_t;

  //  MathematicaVCSPoint(MathLink* ml);
  REDUCEVCSPoint(REDUCELink* cl);

  virtual ~REDUCEVCSPoint();

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
   * 制約を追加する．
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
   * 
   */
  virtual void set_continuity(const continuity_map_t& continuity_map);

private:
  typedef REDUCEStringSender::max_diff_map_t max_diff_map_t;

  /**
   * 左連続性に関する制約を加える
   */
  void add_left_continuity_constraint(const continuity_map_t& continuity_map, REDUCEStringSender &rss);

  /**
   * 制約を出現する変数や左連続制約とともに送信する
   */
  void send_constraint(const constraints_t& constraints);

  /**
   * check_consistency の受信部分
   */
  VCSResult check_consistency_receive();

  /**
   * 制約ストアがtrueであるかどうか
   */
//  bool cs_is_true()
//  {
//    return constraint_store_.first.size()==1 &&
//        (*constraint_store_.first.begin()).size()==1 &&
//        (*(*constraint_store_.first.begin()).begin()).get_string()=="True";
//  }


  mutable REDUCELink* cl_;
  continuity_map_t continuity_map_;
  max_diff_map_t max_diff_map_;          //現在の制約ストアに出現する中で最大の微分回数を記録しておく表．
  constraints_t tmp_constraints_;  //一時的に制約を追加する対象
  std::set<std::string> par_names_; //一時しのぎ
  SExpParser sp_;
};

} // namespace reduce
} // namespace simulator
} // namespace hydla

#endif // _INCLUDED_HYDLA_VCS_REDUCE_VCS_POINT_H_

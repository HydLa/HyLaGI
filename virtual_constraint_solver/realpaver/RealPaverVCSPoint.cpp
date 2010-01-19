#include "RealPaverVCSPoint.h"

#include <sstream>

#include "Logger.h"
#include "rp_constraint.h"
#include "rp_constraint_ext.h"
#include "rp_container.h"
#include "rp_container_ext.h"

namespace hydla {
namespace vcs {
namespace realpaver {

RealPaverVCSPoint::RealPaverVCSPoint()
{}

RealPaverVCSPoint::~RealPaverVCSPoint()
{}

/**
 * 制約ストアの初期化をおこなう
 */
bool RealPaverVCSPoint::reset()
{
  this->constraint_store_ = ConstraintStore();
  return true;
}

/**
 * 与えられた変数表を元に，制約ストアの初期化をおこなう
 */
bool RealPaverVCSPoint::reset(const variable_map_t& variable_map)
{
  this->constraint_store_.build(variable_map);
  return true;
}

/**
 * 現在の制約ストアから変数表を作成する
 */
bool RealPaverVCSPoint::create_variable_map(variable_map_t& variable_map)
{
  this->constraint_store_.build_variable_map(variable_map);
  return true;
}

/**
 * 制約を追加する
 */
VCSResult RealPaverVCSPoint::add_constraint(const tells_t& collected_tells)
{
  typedef std::set<rp_constraint> ctr_set_t;
  var_name_map_t vars = this->constraint_store_.get_store_vars();
  ctr_set_t ctrs, ctrs_copy;
  // tell制約をrp_constraintへ
  for(tells_t::const_iterator t_it = collected_tells.begin();
      t_it!=collected_tells.end(); t_it++) {
        ctrs.insert(this->builder_.build_constraint_from_tell(*t_it));
  }
  // tell制約のみをコピーしておく
  for(ctr_set_t::iterator it=ctrs.begin(); it!=ctrs.end(); it++) {
    rp_constraint c;
    rp_constraint_clone(&c, *it);
    ctrs_copy.insert(c);
  }
  // ストアの制約を追加
  ctr_set_t store_copy = this->constraint_store_.get_store_exprs_copy();
  ctrs.insert(store_copy.begin(), store_copy.end());
  // 確認
  rp_vector_variable vec = this->builder_.to_rp_vector();
  HYDLA_LOGGER_DEBUG("#**** vcs:add_constraint: constraints expression ****\n");
  std::stringstream ss;
  for(ctr_set_t::iterator it=ctrs.begin(); it!=ctrs.end(); it++) {
    rp::dump_constraint(ss, *it, vec, 10);
    ss << "\n";
  }
  HYDLA_LOGGER_DEBUG(ss.str());
  return VCSR_TRUE;
}

/**
 * 現在の制約ストアから与えたaskが導出可能かどうか
 */
VCSResult RealPaverVCSPoint::check_entailment(const ask_node_sptr& negative_ask)
{
  return VCSR_TRUE;
}

/**
 * askの導出状態が変化するまで積分をおこなう
 */
bool RealPaverVCSPoint::integrate(integrate_result_t& integrate_result,
                                  const positive_asks_t& positive_asks,
                                  const negative_asks_t& negative_asks,
                                  const time_t& current_time,
                                  const time_t& max_time)
{
  return true;
}

/**
 * 内部状態の出力をおこなう
 */
std::ostream& RealPaverVCSPoint::dump(std::ostream& s) const
{
  return this->constraint_store_.dump_cs(s);
}

void RealPaverVCSPoint::add_single_constraint(const node_sptr &constraint_node,
                                              const bool neg_expression)
{
}

std::ostream& operator<<(std::ostream& s, const RealPaverVCSPoint& vcs)
{
  return vcs.dump(s);
}


} // namespace realapver
} // namespace vcs
} // namespace hydla 

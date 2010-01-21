#include "RealPaverVCSInterval.h"

#include <sstream>

#include "RPConstraintSolver.h"
#include "Logger.h"
#include "rp_constraint.h"
#include "rp_constraint_ext.h"
#include "rp_container.h"
#include "rp_container_ext.h"

#include "../mathematica/PacketSender.h"

namespace hydla {
namespace vcs {
namespace realpaver {

RealPaverVCSInterval::RealPaverVCSInterval(MathLink* ml) :
constraint_store_(),
  ml_(ml)
{}


RealPaverVCSInterval::~RealPaverVCSInterval()
{}

RealPaverBaseVCS* RealPaverVCSInterval::clone()
{
  RealPaverVCSInterval* vcs_ptr = new RealPaverVCSInterval(this->ml_);
  vcs_ptr->constraint_store_ = this->constraint_store_;
  return vcs_ptr;
}

/**
 * 制約ストアの初期化をおこなう
 */
bool RealPaverVCSInterval::reset()
{
  this->constraint_store_ = ConstraintStoreInterval(); // これあってんのか？
  return true;
}

/**
 * 与えられた変数表を元に，制約ストアの初期化をおこなう
 */
bool RealPaverVCSInterval::reset(const variable_map_t& variable_map)
{
  this->constraint_store_.build(variable_map);
  HYDLA_LOGGER_DEBUG("vcs:reset: new_constraint_store\n",
    this->constraint_store_);
  return true;
}

/**
 * 現在の制約ストアから変数表を作成する
 */
bool RealPaverVCSInterval::create_variable_map(variable_map_t& variable_map)
{
  this->constraint_store_.build_variable_map(variable_map);
  return true;
}

/**
 * 制約を追加する
 */
VCSResult RealPaverVCSInterval::add_constraint(const tells_t& collected_tells)
{
  return VCSR_TRUE;
}

/**
 * 現在の制約ストアから与えたaskが導出可能かどうか
 */
VCSResult RealPaverVCSInterval::check_entailment(const ask_node_sptr& negative_ask)
{
  // ストアをコピー
  // ガード条件と否定を作る(変数はすべて初期値変数へ)
  // solve(S & g) == empty -> FALSE
  // solve(S&ng0)==empty /\ solve(S&ng1)==empty /\ ... -> TRUE
  // ngが存在しない(gが等式)場合，TRUEではない
  // else -> UNKNOWN
/*
  // 制約ストアをコピー
  ctr_set_t ctrs = this->constraint_store_.get_store_exprs_copy();
  var_name_map_t vars = this->constraint_store_.get_store_vars();
  // ガード条件とその否定を作る
  ctr_set_t g, ng;
  var_name_map_t prevs_in_g;
  GuardConstraintBuilder builder;
  builder.set_vars(vars);
  builder.create_guard_expr(negative_ask, g, ng, vars, prevs_in_g);
  // 確認
  {
    rp_vector_variable vec = ConstraintSolver::create_rp_vector(vars);
    HYDLA_LOGGER_DEBUG("#**** vcs:check_entailment: guards ****");
    std::stringstream ss;
    ctr_set_t::iterator it = g.begin();
    while(it != g.end()){
      rp::dump_constraint(ss, *it, vec, 10);
      ss << "\n";
      it++;
    }
    HYDLA_LOGGER_DEBUG(ss.str());
    ss.str("");
    HYDLA_LOGGER_DEBUG("#**** vcs:check_entailment: not_guards ****");
    it = ng.begin();
    while(it != ng.end()){
      if(*it != NULL) rp::dump_constraint(ss, *it, vec, 10);
      ss << "\n";
      it++;
    }
    HYDLA_LOGGER_DEBUG(ss.str());
    ss.str("");
    HYDLA_LOGGER_DEBUG("#**** vcs:check_entailment: prevs_in_guards ****");
    var_name_map_t::iterator it2 = prevs_in_g.begin();
    while(it2 != prevs_in_g.end()){
      HYDLA_LOGGER_DEBUG((*it2).left);
      it2++;
    }
    rp_vector_destroy(&vec);
  }

  // ask条件がprev変数に関する式であり，かつそのprev変数の値が(-oo,+oo)だった場合にはFALSE
  if(this->is_guard_about_undefined_prev(vars, ctrs, prevs_in_g)) {
    HYDLA_LOGGER_DEBUG("#*** vcs:check_entailment ==> FALSE(guard_about_undefined_prev) ***");
    RealPaverVCSInterval::clear_ctr_set(ctrs);
    RealPaverVCSInterval::clear_ctr_set(g);
    RealPaverVCSInterval::clear_ctr_set(ng);
    //this->finalize();
    return VCSR_FALSE;
  }

  // solve(S & g) == empty -> FALSE
  rp_box box;
  ctr_set_t ctr_and_g = ctrs;
  ctr_and_g.insert(g.begin(), g.end());
  if(!(ConstraintSolver::solve_hull(&box, vars, ctr_and_g))) {
    HYDLA_LOGGER_DEBUG("#*** vcs:chack_entailment: ==> FALSE ***");
    RealPaverVCSInterval::clear_ctr_set(ctrs);
    RealPaverVCSInterval::clear_ctr_set(g);
    RealPaverVCSInterval::clear_ctr_set(ng);
    //this->finalize();
    return VCSR_FALSE;
  }
  rp_box_destroy(&box);

  // solve(S&ng0)==empty /\ solve(S&ng1)==empty /\ ... -> TRUE
  // ngが存在しない(gが等式)場合，TRUEではない
  bool is_TRUE = true;
  if(ng.size() == 0) is_TRUE = false;
  for(ctr_set_t::iterator ctr_it=ng.begin();
    ctr_it!=ng.end(); ctr_it++) {
    ctr_set_t ctr_and_ng = ctrs;
    ctr_and_ng.insert(*ctr_it);
    if(ConstraintSolver::solve_hull(&box, vars, ctr_and_ng)) {
      is_TRUE = false;
      rp_box_destroy(&box);
    }
  }
  if(is_TRUE) {
    HYDLA_LOGGER_DEBUG("#*** vcs:check_entailment: ==> TRUE ***");
    RealPaverVCSInterval::clear_ctr_set(ctrs);
    RealPaverVCSInterval::clear_ctr_set(g);
    RealPaverVCSInterval::clear_ctr_set(ng);
    //this->finalize();
    return VCSR_TRUE;
  }

  // else -> UNKNOWN
  HYDLA_LOGGER_DEBUG("#*** vcs:check_entailment: ==> UNKNOWN ***");
  RealPaverVCSInterval::clear_ctr_set(ctrs);
  RealPaverVCSInterval::clear_ctr_set(g);
  RealPaverVCSInterval::clear_ctr_set(ng);
  //this->finalize();
  return VCSR_UNKNOWN;
*/
return VCSR_TRUE;
}

void RealPaverVCSInterval::clear_ctr_set(ctr_set_t& ctrs)
{
  ctrs.erase(static_cast<rp_constraint>(NULL));
  for(ctr_set_t::iterator it=ctrs.begin();
    it!=ctrs.end();it++) {
      rp_constraint c = *it;
      rp_constraint_destroy(&c);
  }
  ctrs.clear();
}
/*
bool RealPaverVCSInterval::is_guard_about_undefined_prev(const var_name_map_t& vars,
                                                      const std::set<rp_constraint>& ctrs,
                                                      const var_name_map_t& p_in_g)
{
  bool res = false;
  rp_box box;
  bool is_consistent_store_only = ConstraintSolver::solve_hull(&box, vars, ctrs);
  assert(is_consistent_store_only);
  for(var_name_map_t::right_const_iterator it=p_in_g.right.begin();
    it!=p_in_g.right.end(); it++) {
    int index = it->first;
    assert(index >= 0);
    if(rp_binf(rp_box_elem(box, index))==-RP_INFINITY
      && rp_bsup(rp_box_elem(box, index))==RP_INFINITY) res = true;
  }
  rp_box_destroy(&box);
  return res;
}
*/
/**
 * askの導出状態が変化するまで積分をおこなう
 */
VCSResult RealPaverVCSInterval::integrate(integrate_result_t& integrate_result,
                                  const positive_asks_t& positive_asks,
                                  const negative_asks_t& negative_asks,
                                  const time_t& current_time,
                                  const time_t& max_time)
{
  // nodes_をrp_constraintになおす
  // 時刻を変数として問題に組み込む(rp_variable)
  // 解く
  // 答えがない→VCSR_FALSE(向こうで積まないだけでもいいんだけど)
  // 答えがproofされているか？
  //// proofチェック１：答えのhullが初期値変数のドメインをすべて含んでいるか？
  //// proofチェック２：(ソルバによるproofが存在するか？)
  //// proofチェック２改：答え一つ一つについて時刻以外の変数を定数に変えてproofが得られるか？
  // されている→答えを時刻順にソート，hullが初期値変数ドメインをすべて含んだ時点で残りを捨てる
  //// 「時点で」だと少し捨てすぎてしまう可能性が…？ -> 同じ時刻のはゆるせばいい
  // されてない→全部積む，今のaskを抜いて再度integrateするためにVCSR_UNKNOWNを返す
  return VCSR_TRUE;
}

/**
 * 内部状態の出力をおこなう
 */
std::ostream& RealPaverVCSInterval::dump(std::ostream& s) const
{
  return s;
  //return this->constraint_store_.dump_cs(s);
}

void RealPaverVCSInterval::add_single_constraint(const node_sptr &constraint_node,
                                              const bool neg_expression)
{
  ConstraintBuilder builder;
  rp_constraint c;
  c = builder.build_constraint(constraint_node, neg_expression);
  var_name_map_t vars;
  vars.insert(builder.vars_begin(), builder.vars_end());
  //if(c) this->constraint_store_.add_constraint(c, vars);
}

std::ostream& operator<<(std::ostream& s, const RealPaverVCSInterval& vcs)
{
  return vcs.dump(s);
}


} // namespace realapver
} // namespace vcs
} // namespace hydla 

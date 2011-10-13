#include "RealPaverVCSPoint.h"

#include <sstream>

#include "RPConstraintSolver.h"
#include "Logger.h"
#include "rp_constraint.h"
#include "rp_constraint_ext.h"
#include "rp_container.h"
#include "rp_container_ext.h"

namespace hydla {
namespace vcs {
namespace realpaver {

RealPaverVCSPoint::RealPaverVCSPoint() :
  prec_(0.5)
{}


RealPaverVCSPoint::~RealPaverVCSPoint()
{}

RealPaverBaseVCS* RealPaverVCSPoint::clone()
{
  RealPaverVCSPoint* vcs_ptr = new RealPaverVCSPoint();
  vcs_ptr->constraint_store_ = this->constraint_store_;
  return vcs_ptr;
}

/**
 * 制約ストアの初期化をおこなう
 */
bool RealPaverVCSPoint::reset()
{
  this->constraint_store_ = ConstraintStore(); // これあってんのか？
  return true;
}

/**
 * 与えられた変数表を元に，制約ストアの初期化をおこなう
 */
bool RealPaverVCSPoint::reset(const variable_map_t& variable_map)
{
  this->constraint_store_.build(variable_map);
  HYDLA_LOGGER_DEBUG("vcs:reset: new_constraint_store\n",
    this->constraint_store_);
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
  ConstraintBuilder builder;
  builder.set_vars(vars);
  ctr_set_t ctrs, ctrs_copy;
  // tell制約をrp_constraintへ
  for(tells_t::const_iterator t_it = collected_tells.begin();
      t_it!=collected_tells.end(); t_it++) {
        ctrs.insert(builder.build_constraint_from_tell(*t_it));
  }
  vars.insert(builder.vars_begin(), builder.vars_end());
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
  rp_vector_variable vec = ConstraintSolver::create_rp_vector(vars, prec_);
  HYDLA_LOGGER_DEBUG("#**** vcs:add_constraint: constraints expression ****");
  std::stringstream ss;
  for(ctr_set_t::iterator it=ctrs.begin(); it!=ctrs.end(); it++) {
    rp::dump_constraint(ss, *it, vec, 10);
    ss << "\n";
  }
  rp_vector_destroy(&vec);
  HYDLA_LOGGER_DEBUG(ss.str());
  // 制約の解が存在するかどうか？
  rp_box b;
  bool res = ConstraintSolver::solve_hull(&b, vars, ctrs, prec_);
  if(res) {
    // consistentなら，ストアに制約を追加
    this->constraint_store_.add_constraint(ctrs_copy.begin(), ctrs_copy.end(), vars);
    HYDLA_LOGGER_DEBUG("#*** vcs:add_constraint ==> Consistent ***\n",
      "#**** vcs:add_constraint: new constraint_store ***\n",
      this->constraint_store_);
    rp_box_destroy(&b);
    for(ctr_set_t::iterator it=ctrs.begin(); it!=ctrs.end(); it++) {
      rp_constraint c = *it;
      rp_constraint_destroy(&c);
    }
    return VCSR_TRUE;
  } else {
    // in-consistentなら，何もしない
    for(ctr_set_t::iterator it=ctrs_copy.begin(); it!=ctrs_copy.end(); it++) {
      rp_constraint c = *it;
      rp_constraint_destroy(&c);
    }
    HYDLA_LOGGER_DEBUG("#*** vcs:add_constraint ==> Inconsistent ***\n");
    for(ctr_set_t::iterator it=ctrs.begin(); it!=ctrs.end(); it++) {
      rp_constraint c = *it;
      rp_constraint_destroy(&c);
    }
    return VCSR_FALSE;
  }
}

/**
 * 現在の制約ストアから与えたaskが導出可能かどうか
 */
VCSResult RealPaverVCSPoint::check_entailment(const ask_node_sptr& negative_ask)
{
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
    rp_vector_variable vec = ConstraintSolver::create_rp_vector(vars, prec_);
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
    RealPaverVCSPoint::clear_ctr_set(ctrs);
    RealPaverVCSPoint::clear_ctr_set(g);
    RealPaverVCSPoint::clear_ctr_set(ng);
    //this->finalize();
    return VCSR_FALSE;
  }

  // solve(S & g) == empty -> FALSE
  rp_box box;
  ctr_set_t ctr_and_g = ctrs;
  ctr_and_g.insert(g.begin(), g.end());
  if(!(ConstraintSolver::solve_hull(&box, vars, ctr_and_g, prec_))) {
    HYDLA_LOGGER_DEBUG("#*** vcs:chack_entailment: ==> FALSE ***");
    RealPaverVCSPoint::clear_ctr_set(ctrs);
    RealPaverVCSPoint::clear_ctr_set(g);
    RealPaverVCSPoint::clear_ctr_set(ng);
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
    if(ConstraintSolver::solve_hull(&box, vars, ctr_and_ng, prec_)) {
      is_TRUE = false;
      rp_box_destroy(&box);
    }
  }
  if(is_TRUE) {
    HYDLA_LOGGER_DEBUG("#*** vcs:check_entailment: ==> TRUE ***");
    RealPaverVCSPoint::clear_ctr_set(ctrs);
    RealPaverVCSPoint::clear_ctr_set(g);
    RealPaverVCSPoint::clear_ctr_set(ng);
    //this->finalize();
    return VCSR_TRUE;
  }

  // else -> UNKNOWN
  HYDLA_LOGGER_DEBUG("#*** vcs:check_entailment: ==> UNKNOWN ***");
  RealPaverVCSPoint::clear_ctr_set(ctrs);
  RealPaverVCSPoint::clear_ctr_set(g);
  RealPaverVCSPoint::clear_ctr_set(ng);
  //this->finalize();
  return VCSR_UNKNOWN;
}

void RealPaverVCSPoint::clear_ctr_set(ctr_set_t& ctrs)
{
  ctrs.erase(static_cast<rp_constraint>(NULL));
  for(ctr_set_t::iterator it=ctrs.begin();
    it!=ctrs.end();it++) {
      rp_constraint c = *it;
      rp_constraint_destroy(&c);
  }
  ctrs.clear();
}

bool RealPaverVCSPoint::is_guard_about_undefined_prev(const var_name_map_t& vars,
                                                      const std::set<rp_constraint>& ctrs,
                                                      const var_name_map_t& p_in_g)
{
  bool res = false;
  rp_box box;
  bool is_consistent_store_only = ConstraintSolver::solve_hull(&box, vars, ctrs, prec_);
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

/**
 * askの導出状態が変化するまで積分をおこなう
 */
VCSResult RealPaverVCSPoint::integrate(integrate_result_t& integrate_result,
                                       const positive_asks_t& positive_asks,
                                       const negative_asks_t& negative_asks,
                                       const time_t& current_time,
                                       const time_t& max_time,
                                       const not_adopted_tells_list_t& not_adopted_tells_list)
{
  assert(false);
  return VCSR_FALSE;
}

/**
 * 内部状態の出力をおこなう
 */
std::ostream& RealPaverVCSPoint::dump(std::ostream& s) const
{
  return this->constraint_store_.dump_cs(s);
}

/**
 * 制約ストアに制約を追加する
 */
void RealPaverVCSPoint::add_single_constraint(const node_sptr &constraint_node,
                                              const bool neg_expression)
{
  ConstraintBuilder builder;
  rp_constraint c;
  c = builder.build_constraint(constraint_node, neg_expression);
  var_name_map_t vars;
  vars.insert(builder.vars_begin(), builder.vars_end());
  if(c) this->constraint_store_.add_constraint(c, vars);
}

/**
 * boxの精度を設定する
 */
void RealPaverVCSPoint::set_precision(const double p)
{
  this->prec_ = p;
  this->constraint_store_.set_precision(p);
}

std::ostream& operator<<(std::ostream& s, const RealPaverVCSPoint& vcs)
{
  return vcs.dump(s);
}


} // namespace realapver
} // namespace vcs
} // namespace hydla 

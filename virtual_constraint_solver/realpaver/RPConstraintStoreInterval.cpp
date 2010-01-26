#include "RPConstraintStoreInterval.h"
#include "Logger.h"
#include "realpaver.h"
#include "rp_problem_ext.h"
#include "rp_constraint_ext.h"
#include "RPConstraintSolver.h"

#include <cassert>
#include <boost/lexical_cast.hpp>

namespace hydla {
namespace vcs {
namespace realpaver {

typedef std::set<rp_constraint> ctr_set_t;

ConstraintStoreInterval::ConstraintStoreInterval(const double prec) : 
  prec_(prec)
{}

ConstraintStoreInterval::ConstraintStoreInterval(const ConstraintStoreInterval& src)
{}

ConstraintStoreInterval::~ConstraintStoreInterval()
{}

ConstraintStoreInterval& ConstraintStoreInterval::operator=(const ConstraintStoreInterval& src)
{
  return *this;
}

void ConstraintStoreInterval::build(const virtual_constraint_solver_t::variable_map_t& variable_map)
{
  typedef var_name_map_t::value_type vars_type_t;
  virtual_constraint_solver_t::variable_map_t::const_iterator it;
  for(it=variable_map.begin(); it!=variable_map.end(); it++) {
    // 変数名を作る ex. "usrVar0ht" "initValue0ht"
    std::string name(var_prefix);
    name += boost::lexical_cast<std::string>(it->first.derivative_count);
    name += it->first.name;
    std::string ini_name(init_prefix);
    ini_name += boost::lexical_cast<std::string>(it->first.derivative_count);
    ini_name += it->first.name;
    // 表に登録
    unsigned int size = this->vars_.size();
    var_property vp(it->first.derivative_count, false),
      vp_p(it->first.derivative_count, true);
    this->vars_.insert(vars_type_t(name, size, vp)); // 登録済みの変数は変更されない
    this->vars_.insert(vars_type_t(ini_name, size+1, vp_p));
    // rp_intervalからrp_constraintを作る
    rp_interval i;
    it->second.get(rp_binf(i), rp_bsup(i));
    // (-oo, +oo) ==> 作らない，登録しない
    if(rp_binf(i)==-RP_INFINITY && rp_bsup(i)==RP_INFINITY) continue;
    if(rp_interval_point(i)) {
      // rp_interval_point ==> 式1つ登録(var = val)
      rp_erep l, r;
      rp_ctr_num cnum;
      rp_constraint c;
      rp_erep_create_var(&l, this->vars_.left.at(ini_name));
      rp_erep_create_cst(&r, "", i);
      rp_ctr_num_create(&cnum, &l, RP_RELATION_EQUAL, &r);
      rp_constraint_create_num(&c, cnum);
      this->exprs_.insert(c);
    } else {
      // else ==> 式2つ登録(inf <= var, var <= sup)
      rp_interval i_tmp;
      rp_erep l, r;
      rp_ctr_num cnum;
      rp_constraint c;
      rp_erep_create_var(&l, this->vars_.left.at(ini_name));
      rp_interval_set_point(i_tmp, rp_binf(i));
      rp_erep_create_cst(&r, "", i_tmp);
      rp_ctr_num_create(&cnum, &l, RP_RELATION_SUPEQUAL, &r);
      rp_constraint_create_num(&c, cnum);
      this->exprs_.insert(c);
      rp_erep_create_var(&l, this->vars_.left.at(ini_name));
      rp_interval_set_point(i_tmp, rp_bsup(i));
      rp_erep_create_cst(&r, "", i_tmp);
      rp_ctr_num_create(&cnum, &l, RP_RELATION_INFEQUAL, &r);
      rp_constraint_create_num(&c, cnum);
      this->exprs_.insert(c);
    }
  }
  // Intervalでは時刻tも変数
  unsigned int size = this->vars_.size();
  var_property vp(0, false);
  this->vars_.insert(vars_type_t("t", size, vp)); // 登録済みの変数は変更されない
}

// 消す予定
void ConstraintStoreInterval::build_variable_map(virtual_constraint_solver_t::variable_map_t& variable_map) const
{
  assert(false);
}

/**
 * ストアの初期値制約のみをコピーして返す
 */
std::set<rp_constraint> ConstraintStoreInterval::get_store_exprs_copy() const
{
  std::set<rp_constraint> ans;
  std::set<rp_constraint>::const_iterator it = this->exprs_.begin();
  while(it != this->exprs_.end()) {
    rp_constraint c;
    rp_constraint_clone(&c, (*it));
    ans.insert(c);
    it++;
  }
  return ans;
}

/**
 * ストアの非初期値制約のみをコピーして返す
 * TODO: 名前に統一性がなさすぎる…要リファクタリング
 */
ctr_set_t ConstraintStoreInterval::get_store_non_init_constraint_copy() const
{
  ctr_set_t ans;
  ctr_set_t::const_iterator it = this->non_init_exprs_.begin();
  while(it != this->non_init_exprs_.end()) {
    rp_constraint c;
    rp_constraint_clone(&c, (*it));
    ans.insert(c);
    it++;
  }
  return ans;
}

/**
 * 非初期値制約をクリアする．VCSのadd_constraint用
 */
void ConstraintStoreInterval::clear_non_init_constraint()
{
  for(ctr_set_t::iterator it=this->non_init_exprs_.begin();
    it!=this->non_init_exprs_.end(); ++it) {
      rp_constraint c = *it;
      if(c) rp_constraint_destroy(&c);
      this->non_init_exprs_.erase(it++);
  }
}

/**
 * 非初期値制約をセットする．VCSのadd_constraint用
 * あれ？関数一つでいいんじゃね？
 */
void ConstraintStoreInterval::set_non_init_constraint(const ctr_set_t ctrs)
{
  assert(this->non_init_exprs_.size() == 0);
  this->non_init_exprs_ = ctrs;
}

void ConstraintStoreInterval::set_precision(const double p)
{
  this->prec_ = p;
}

//void ConstraintStoreInterval::add_constraint(rp_constraint c, const var_name_map_t& vars)
//{}

//void ConstraintStoreInterval::add_constraint(std::set<rp_constraint>::iterator start, std::set<rp_constraint>::iterator end, const var_name_map_t& vars)
//{}

std::ostream& ConstraintStoreInterval::dump_cs(std::ostream& s) const
{
  rp_vector_variable vec = ConstraintSolver::create_rp_vector(this->vars_, prec_);
  ctr_set_t::const_iterator ctr_it = this->exprs_.begin();
  while(ctr_it != this->exprs_.end()){
    rp::dump_constraint(s, *ctr_it, vec); // digits, mode);
    s << "\n";
    ctr_it++;
  }
  for(ctr_it=this->non_init_exprs_.begin(); ctr_it!=this->non_init_exprs_.end(); ++ctr_it){
    rp::dump_constraint(s, *ctr_it, vec);
  }
  s << "\n";
  rp_vector_destroy(&vec);
  return s;
}

} // namespace realpaver
} // namespace vcs
} // namespace hydla 

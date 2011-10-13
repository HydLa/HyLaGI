#include "./EntailmentChecker.h"
#include "Logger.h"
#include "rp_constraint_ext.h"

#include <iostream>
#include <sstream>

using namespace hydla::parse_tree;
using namespace hydla::simulator;

//小数を何桁まで表示するか
#define DISPLAY_DIGITS 10

namespace hydla {
namespace bp_simulator {

EntailmentChecker::EntailmentChecker(bool debug_mode) :
  debug_mode_(debug_mode)
{}

EntailmentChecker::~EntailmentChecker()
{
  this->finalize();
}

// 変数
void EntailmentChecker::visit(boost::shared_ptr<Variable> node)
{
  ConstraintBuilder::visit(node);
  if(this->in_prev_) {
    std::string name(node->get_name());
    for(unsigned int i=0; i< this->derivative_count_; i++) name += BP_DERIV_STR;
    name += BP_PREV_STR;
    this->prevs_in_guard_.insert(name);
  }
}

/**
 * collected_tellsからnegative_askのガード条件がentailされるどうか調べる
 * TRUEならcollected_tellsにask制約の後件を追加する
 * 
 * @param negative_ask まだ展開されていないask制約
 * @param collected_tells tell制約のリスト（展開されたask制約の「=>」の右辺がここに追加される）
 * @param constraint_store 制約ストア
 * 
 * @return entailされるかどうか {TRUE, UNKNOWN, FALSE}
 */
Trivalent EntailmentChecker::check_entailment(
  const boost::shared_ptr<Ask>& negative_ask,
  ConstraintStore& constraint_store)
{
  // constraint_store = 現制約 S
  this->constraints_ = constraint_store.get_store_exprs_copy();
  this->vars_ = constraint_store.get_store_vars();

  // 作成できたか確認
  rp_vector_variable vec = this->to_rp_vector();
  {
    HYDLA_LOGGER_DEBUG("#**** entailment check: constraints ****");
    std::stringstream ss;
    std::set<rp_constraint>::iterator it = this->constraints_.begin();
    while(it != this->constraints_.end()){
      rp::dump_constraint(ss, *it, vec, DISPLAY_DIGITS);
      ss << "\n";
      it++;
    }
    HYDLA_LOGGER_DEBUG(ss.str());
  }
  rp_vector_destroy(&vec);

  // ask条件からgとngを作る
  this->accept(negative_ask);

  // 作成できたか確認
  vec = this->to_rp_vector();
  {
    HYDLA_LOGGER_DEBUG("#**** entailment check: guards ****");
    std::stringstream ss;
    std::set<rp_constraint>::iterator it = this->guards_.begin();
    while(it != this->guards_.end()){
      rp::dump_constraint(ss, *it, vec, DISPLAY_DIGITS);
      ss << "\n";
      it++;
    }
    HYDLA_LOGGER_DEBUG(ss.str());
    ss.str("");
    HYDLA_LOGGER_DEBUG("#**** entailment check: not_guards ****");
    it = this->not_guards_.begin();
    while(it != this->not_guards_.end()){
      if(*it != NULL) rp::dump_constraint(ss, *it, vec, DISPLAY_DIGITS);
      ss << "\n";
      it++;
    }
    HYDLA_LOGGER_DEBUG(ss.str());
    ss.str("");
    HYDLA_LOGGER_DEBUG("#**** entailment check: prevs_in_guard ****");
    std::set<std::string>::iterator it2 = this->prevs_in_guard_.begin();
    while(it2 != this->prevs_in_guard_.end()){
      HYDLA_LOGGER_DEBUG(*it2);
      it2++;
    }
    HYDLA_LOGGER_DEBUG("\n");
  }
  rp_vector_destroy(&vec);

  // ask条件がprev変数に関する式であり，かつそのprev変数の値が(-oo,+oo)だった場合にはFALSE
  if(this->is_guard_about_undefined_prev()) {
    if(this->debug_mode_) std::cout << "#*** entailment check ==> FALSE(guard_about_undefined_prev) ***\n";
    this->finalize();
    return Tri_FALSE;
  }

  // solve(S & g) == empty -> FALSE
  rp_box box;
  this->create_initial_box(&box);
  std::set<rp_constraint> ctr_and_g = this->constraints_;
  ctr_and_g.insert(this->guards_.begin(), this->guards_.end());
  if(!(this->solve_hull(ctr_and_g, box))) {
    rp_box_destroy(&box);
    if(this->debug_mode_) std::cout << "#*** entailment check ==> FALSE ***\n";
    this->finalize();
    return Tri_FALSE;
  }
  rp_box_destroy(&box);

  // solve(S&ng0)==empty /\ solve(S&ng1)==empty /\ ... -> TRUE
  // ngが存在しない(gが等式)場合，TRUEではない
  bool is_TRUE = true;
  if(this->not_guards_.size() == 0) is_TRUE = false;
  std::set<rp_constraint>::iterator ctr_it = this->not_guards_.begin();
  while(ctr_it != this->not_guards_.end()) {
    this->create_initial_box(&box);
    std::set<rp_constraint> ctr_and_ng = this->constraints_;
    ctr_and_ng.insert(*ctr_it);
    if(this->solve_hull(ctr_and_ng, box)) is_TRUE = false;
    rp_box_destroy(&box);
    ctr_it++;
  }
  if(is_TRUE) {
    if(this->debug_mode_) std::cout << "#*** entailment check ==> TRUE ***\n";
    this->finalize();
    return Tri_TRUE;
  }

  // else -> UNKNOWN
  if(this->debug_mode_) std::cout << "#*** entailment check ==> UNKNOWN ***\n";
  this->finalize();
  return Tri_UNKNOWN;
}

/**
 * 変数の初期値((-oo,+oo))を入れたboxを作る
 */
void EntailmentChecker::create_initial_box(rp_box *b)
{
  rp_box_create(b, this->vars_.size());
  for(int i=0; i<rp_box_size(*b); i++) {
    rp_interval_set(rp_box_elem(*b, i), -RP_INFINITY, RP_INFINITY);
  }
}

/**
 * ask条件が未定義のprev変数に関する式であるか
 * ask条件がprev変数に関する式であり，かつそのprev変数の値が(-oo,+oo)だった場合にはtrue
 *  1. guardに現れるprev変数を列挙 = prevs_in_guard_
 *  2. ストアだけを解いてprev変数の値を決定
 *  3. 列挙されたprev変数の値が一つでも(-oo,+oo)？
 */
bool EntailmentChecker::is_guard_about_undefined_prev()
{
  bool res = false;
  rp_box box;
  this->create_initial_box(&box);
  bool is_consistent_store_only = this->solve_hull(this->constraints_, box);
  assert(is_consistent_store_only);
  std::set<std::string>::iterator it = this->prevs_in_guard_.begin();
  while(it != this->prevs_in_guard_.end()) {
    int index = this->vars_.left.at(*it);
    assert(index >= 0);
    if(rp_binf(rp_box_elem(box, index))==-RP_INFINITY
      && rp_bsup(rp_box_elem(box, index))==RP_INFINITY) res = true;
    it++;
  }
  rp_box_destroy(&box);
  return res;
}

/**
 * □SOLVEと(ほぼ)同様の計算をする
 * TODO: これで動くが，box consistencyを満たしていない
 * @param c 制約
 * @param b 初期box
 * @return b中にcを満たす範囲が存在するかどうか
 */
bool EntailmentChecker::solve_hull(std::set<rp_constraint> c, rp_box b)
{
  c.erase(static_cast<rp_constraint>(NULL));
  std::set<rp_constraint>::iterator it = c.begin();
  while(it != c.end()) {
    assert(rp_constraint_type(*it) == RP_CONSTRAINT_NUMERICAL);
    int rel = rp_ctr_num_rel(rp_constraint_num(*it));
    switch(rel) {
      case RP_RELATION_EQUAL:
        if(!rp_sat_hull_eq(rp_constraint_num(*it), b)) return false;
        break;
      case RP_RELATION_SUPEQUAL:
        if(!rp_sat_hull_sup(rp_constraint_num(*it), b)) return false;
        break;
      case RP_RELATION_INFEQUAL:
        if(!rp_sat_hull_inf(rp_constraint_num(*it), b)) return false;
        break;
    }
    it++;
  }
  return true;
}

/**
 * 終了処理，rp_constraintを解放する
 * デストラクタと同じ
 */
void EntailmentChecker::finalize()
{
  this->constraints_.erase(static_cast<rp_constraint>(NULL));
  this->guards_.erase(static_cast<rp_constraint>(NULL));
  this->not_guards_.erase(static_cast<rp_constraint>(NULL));
  std::set<rp_constraint>::iterator it;
  it = this->constraints_.begin();
  while(it != this->constraints_.end()) {
    rp_constraint c = *it;
    rp_constraint_destroy(&c);
    this->constraints_.erase(it++);
  }
  it = this->guards_.begin();
  while(it != this->guards_.end()) {
    rp_constraint c = *it;
    rp_constraint_destroy(&c);
    this->guards_.erase(it++);
  }
  it = this->not_guards_.begin();
  while(it != this->not_guards_.end()) {
    rp_constraint c = *it;
    rp_constraint_destroy(&c);
    this->not_guards_.erase(it++);
  }
}
///**
// * constraints_の中身をすべて複製したsetを返す
// */
//std::set<rp_constraint> EntailmentChecker::copy_constraints()
//{
//  std::set<rp_constraint> res;
//  std::set<rp_constraint>::iterator it = this->constraints_.begin();
//  while(it != this->constraints_.end()) {
//    rp_constraint c;
//    rp_constraint_clone(&c, *it);
//    res.insert(c);
//    it++;
//  }
//  return res;
//}

} //namespace bp_simulator
} // namespace hydla

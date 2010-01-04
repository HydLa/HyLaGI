#include "EntailmentChecker.h"

#include <iostream>

using namespace hydla::parse_tree;
using namespace hydla::simulator;

//小数を何桁まで表示するか
#define DISPLAY_DIGITS 10

namespace hydla {
namespace bp_simulator {


EntailmentChecker::EntailmentChecker() :
  is_tell_ctr_(false),
  debug_mode_(false)
{}

EntailmentChecker::EntailmentChecker(bool debug_mode) :
  is_tell_ctr_(false),
  debug_mode_(debug_mode)
{}

EntailmentChecker::~EntailmentChecker()
{}

// Ask制約
void EntailmentChecker::visit(boost::shared_ptr<Ask> node)
{
  // ask条件から
  // guard_list(andつながり)
  // not_guard_list(orつながり)
  // の両方を作る
  this->accept(node->get_guard());
}

// Tell制約 必要ない？
void EntailmentChecker::visit(boost::shared_ptr<Tell> node)
{
  this->is_tell_ctr_ = true;
  rp_constraint c;
  this->accept(node->get_child());
  rp_constraint_create_num(&c, this->ctr_);
  this->constraints_.insert(c);
  this->ctr_ = NULL;
  this->is_tell_ctr_ = false;
}

// 比較演算子
// ここで一気に二つの式を作る
void EntailmentChecker::visit(boost::shared_ptr<Equal> node)
{
  if(this->is_tell_ctr_) {
    ConstraintBuilder::visit(node);
  } else {
    // ask時
    rp_constraint a;
    this->create_ctr_num(node, RP_RELATION_EQUAL);
    rp_constraint_create_num(&a, this->ctr_);
    this->guards_.insert(a);
    this->ctr_ = NULL;
    // not_a は制約なし
  }
}

void EntailmentChecker::visit(boost::shared_ptr<UnEqual> node)               
{
  if(this->is_tell_ctr_) {
    ConstraintBuilder::visit(node);
  } else {
    // ask時
    // a は制約なし
    rp_constraint not_a;
    this->create_ctr_num(node, RP_RELATION_EQUAL);
    rp_constraint_create_num(&not_a, this->ctr_);
    this->guards_.insert(not_a);
    this->ctr_ = NULL;
  }
}

void EntailmentChecker::visit(boost::shared_ptr<Less> node)                  
{
  if(this->is_tell_ctr_) {
    ConstraintBuilder::visit(node);
  } else {
    // ask時
    rp_constraint a, not_a;
    this->create_ctr_num(node, RP_RELATION_INFEQUAL);
    rp_constraint_create_num(&a, this->ctr_);
    this->guards_.insert(a);
    this->ctr_ = NULL;
    this->create_ctr_num(node, RP_RELATION_SUPEQUAL);
    rp_constraint_create_num(&not_a, this->ctr_);
    this->not_guards_.insert(not_a);
    this->ctr_ = NULL;
  }
}

void EntailmentChecker::visit(boost::shared_ptr<LessEqual> node)             
{
  if(this->is_tell_ctr_) {
    ConstraintBuilder::visit(node);
  } else {
    // ask時
    rp_constraint a, not_a;
    this->create_ctr_num(node, RP_RELATION_INFEQUAL);
    rp_constraint_create_num(&a, this->ctr_);
    this->guards_.insert(a);
    this->ctr_ = NULL;
    this->create_ctr_num(node, RP_RELATION_SUPEQUAL);
    rp_constraint_create_num(&not_a, this->ctr_);
    this->not_guards_.insert(not_a);
    this->ctr_ = NULL;
  }
}

void EntailmentChecker::visit(boost::shared_ptr<Greater> node)               
{
  if(this->is_tell_ctr_) {
    ConstraintBuilder::visit(node);
  } else {
    // ask時
    rp_constraint a, not_a;
    this->create_ctr_num(node, RP_RELATION_SUPEQUAL);
    rp_constraint_create_num(&a, this->ctr_);
    this->guards_.insert(a);
    this->ctr_ = NULL;
    this->create_ctr_num(node, RP_RELATION_INFEQUAL);
    rp_constraint_create_num(&not_a, this->ctr_);
    this->not_guards_.insert(not_a);
    this->ctr_ = NULL;
  }
}

void EntailmentChecker::visit(boost::shared_ptr<GreaterEqual> node)          
{
  if(this->is_tell_ctr_) {
    ConstraintBuilder::visit(node);
  } else {
    // ask時
    rp_constraint a, not_a;
    this->create_ctr_num(node, RP_RELATION_SUPEQUAL);
    rp_constraint_create_num(&a, this->ctr_);
    this->guards_.insert(a);
    this->ctr_ = NULL;
    this->create_ctr_num(node, RP_RELATION_INFEQUAL);
    rp_constraint_create_num(&not_a, this->ctr_);
    this->not_guards_.insert(not_a);
    this->ctr_ = NULL;
  }
}

// 論理演算子
void EntailmentChecker::visit(boost::shared_ptr<LogicalAnd> node)            
{
  this->accept(node->get_lhs());
  this->accept(node->get_rhs());
}

// 変数
void EntailmentChecker::visit(boost::shared_ptr<Variable> node)
{
  ConstraintBuilder::visit(node);
  if(this->in_prev_) this->prevs_in_guard_.insert(node->get_name() + BP_PREV_STR);
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
  tells_t& collected_tells,
  ConstraintStore& constraint_store)
{
  // constraint_store + collected_tells = 現制約 S
  this->constraints_ = constraint_store.get_store_exprs_copy();
  this->vars_ = constraint_store.get_store_vars();
  // collected_tellsから
  tells_t::iterator tells_it = collected_tells.begin();
  while((tells_it) != collected_tells.end()){
    this->accept(*tells_it);
    tells_it++;
  }

  // 作成できたか確認
  rp_vector_variable vec = this->to_rp_vector();
  if(this->debug_mode_){
    std::cout << "#**** entailment check: constraints ****\n";
    std::set<rp_constraint>::iterator it = this->constraints_.begin();
    while(it != this->constraints_.end()){
      rp_constraint_display(stdout, *it, vec, DISPLAY_DIGITS);
      std::cout << "\n";
      it++;
    }
    std::cout << "\n";
  }
  rp_vector_destroy(&vec);

  // ask条件からgとngを作る
  this->accept(negative_ask);

  // 作成できたか確認
  vec = this->to_rp_vector();
  if(this->debug_mode_){
    std::cout << "#**** entailment check: guards ****\n";
    std::set<rp_constraint>::iterator it = this->guards_.begin();
    while(it != this->guards_.end()){
      rp_constraint_display(stdout, *it, vec, DISPLAY_DIGITS);
      std::cout << "\n";
      it++;
    }
    std::cout << "\n#**** entailment check: not_guards ****\n";
    it = this->not_guards_.begin();
    while(it != this->not_guards_.end()){
      rp_constraint_display(stdout, *it, vec, DISPLAY_DIGITS);
      std::cout << "\n";
      it++;
    }
    std::cout << "\n#**** entailment check: prevs_in_guard ****\n";
    std::set<std::string>::iterator it2 = this->prevs_in_guard_.begin();
    while(it2 != this->prevs_in_guard_.end()){
      std::cout << *it2 <<"\n";
      it2++;
    }
    std::cout << "\n";
  }
  rp_vector_destroy(&vec);

  // ask条件がprev変数に関する式であり，かつそのprev変数の値が(-oo,+oo)だった場合にはFALSE
  if(this->is_guard_about_undefined_prev()) return FALSE;

  // solve(S & g) == empty -> FALSE
  // solve(S&ng0)==empty /\ solve(S&ng1)==empty /\ ... -> TRUE
  // else -> UNKNOWN
  return FALSE;
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
  rp_box_create(&box, this->vars_.size());
  for(int i=0; i<rp_box_size(box); i++) {
    rp_interval_set(rp_box_elem(box, i), -RP_INFINITY, RP_INFINITY);
  }
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
 * @param c 制約
 * @param b 初期box
 * @return b中にcを満たす範囲が存在するかどうか
 */
bool EntailmentChecker::solve_hull(std::set<rp_constraint> c, rp_box b)
{
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
 * constraints_の中身をすべて複製したsetを返す
 */
std::set<rp_constraint> EntailmentChecker::copy_constraints()
{
  std::set<rp_constraint> res;
  std::set<rp_constraint>::iterator it = this->constraints_.begin();
  while(it != this->constraints_.end()) {
    rp_constraint c;
    rp_constraint_clone(&c, *it);
    res.insert(c);
    it++;
  }
  return res;
}

/**
 * vars_をrp_vector_variableに変換
 * 変数の値は(-oo, +oo)
 */
rp_vector_variable EntailmentChecker::to_rp_vector()
{
  rp_vector_variable vec;
  rp_vector_variable_create(&vec);
  int size = this->vars_.size();
  for(int i=0; i<size; i++){
    rp_variable v;
    rp_variable_create(&v, ((this->vars_.right.at(i)).c_str()));
    rp_interval interval;
    rp_interval_set(interval,(-1)*RP_INFINITY,RP_INFINITY);
    rp_union_insert(rp_variable_domain(v), interval);
    rp_vector_insert(vec, v);
  }
  return vec;
}

} //namespace bp_simulator
} // namespace hydla

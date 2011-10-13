#include "RPConstraintStore.h"
#include "RPConstraintSolver.h"
#include "Logger.h"
#include "realpaver.h"
#include "rp_problem_ext.h"
#include "rp_constraint_ext.h"

#include <sstream>
#include <boost/lexical_cast.hpp>

namespace hydla {
namespace vcs {
namespace realpaver {

/**
 * ポイントフェーズ用制約ストア
 */
ConstraintStore::ConstraintStore(const double prec) :
  prec_(prec)
{}

ConstraintStore::ConstraintStore(const ConstraintStore& src)
{
  this->exprs_ = src.get_store_exprs_copy();
  this->vars_ = src.vars_;
  this->prec_ = src.prec_;
}

ConstraintStore::~ConstraintStore()
{
  this->exprs_.erase(static_cast<rp_constraint>(NULL));
  std::set<rp_constraint>::iterator it = this->exprs_.begin();
  while(it != this->exprs_.end()) {
    rp_constraint c = *it;
    if(c) rp_constraint_destroy(&c);
    this->exprs_.erase(it++);
  }
}

ConstraintStore& ConstraintStore::operator =(const ConstraintStore &src)
{
  this->exprs_ = src.get_store_exprs_copy();
  this->vars_ = src.vars_;
  return *this;
}

/**
 * variable_mapからストアを構築する
 * prev_var <- var の変換はここでおこなう
 * @param variable_map 変数表
 * TODO: ちゃんと書く
 */
void ConstraintStore::build(const virtual_constraint_solver_t::variable_map_t& variable_map)
{
  typedef var_name_map_t::value_type vars_type_t;

  virtual_constraint_solver_t::variable_map_t::const_iterator it;
  for(it=variable_map.begin(); it!=variable_map.end(); ++it) {
    // 変数名を作る ex. "usrVar0ht" prevVar0ht"
    int dc = it->first.derivative_count;
    std::string dc_str(boost::lexical_cast<std::string>(dc));
    std::string name(var_prefix);
    name += dc_str;
    name += it->first.name;
    std::string prev_name(prev_prefix);
    prev_name += dc_str;
    prev_name += it->first.name;
    // 表に登録
    unsigned int size = this->vars_.size();
    var_property vp(dc, false),
      vp_p(dc, true);
    this->vars_.insert(vars_type_t(name, size, vp)); // 登録済みの変数は変更されない
    this->vars_.insert(vars_type_t(prev_name, size+1, vp_p));
    // 変数の(区間)値 x[inf, sup] から制約 inf <= x, x <= sup を作る
    rp_interval i;
    it->second.get(rp_binf(i), rp_bsup(i));
    // x(-oo, +oo) ==> 作らない，登録しない
    if(rp_binf(i)==-RP_INFINITY && rp_bsup(i)==RP_INFINITY) continue;
    if(rp_interval_point(i)) {
      // inf==sup ==> 式1つ登録 (x = inf)
      rp_erep l, r;
      rp_ctr_num cnum;
      rp_constraint c;
      rp_erep_create_var(&l, this->vars_.left.at(prev_name));
      rp_erep_create_cst(&r, "", i);
      rp_ctr_num_create(&cnum, &l, RP_RELATION_EQUAL, &r);
      rp_constraint_create_num(&c, cnum);
      this->exprs_.insert(c);
    } else {
      // else ==> 式2つ登録 (inf <= x, x <= sup)
      rp_interval i_tmp;
      rp_erep l, r;
      rp_ctr_num cnum;
      rp_constraint c;
      rp_erep_create_var(&l, this->vars_.left.at(prev_name));
      rp_interval_set_point(i_tmp, rp_binf(i));
      rp_erep_create_cst(&r, "", i_tmp);
      rp_ctr_num_create(&cnum, &l, RP_RELATION_SUPEQUAL, &r);
      rp_constraint_create_num(&c, cnum);
      this->exprs_.insert(c);
      rp_erep_create_var(&l, this->vars_.left.at(prev_name));
      rp_interval_set_point(i_tmp, rp_bsup(i));
      rp_erep_create_cst(&r, "", i_tmp);
      rp_ctr_num_create(&cnum, &l, RP_RELATION_INFEQUAL, &r);
      rp_constraint_create_num(&c, cnum);
      this->exprs_.insert(c);
    }
  }
}

/**
 * ストアからvariable_mapを作成する
 * @param variable_map 作成先の変数表
 */
void ConstraintStore::build_variable_map(virtual_constraint_solver_t::variable_map_t& variable_map) const
{
  // 問題の生成
  // TODO: precisionは？
  ctr_set_t exprs_copy = this->get_store_exprs_copy();
  rp_vector_variable vec = ConstraintSolver::create_rp_vector(this->vars_, prec_);
  // TODO: ConstraintSolver::solve_exact_hullを使うべき(あっちもメモリ解放に自信ないけど)
  rp_problem problem;
  rp_problem_create(&problem, "build_variable_map");
  rp_vector_destroy(&rp_table_symbol_vars(rp_problem_symb(problem)));
  rp_table_symbol_vars(rp_problem_symb(problem)) = vec;
  ctr_set_t::const_iterator it;
  for(it=exprs_copy.begin(); it!=exprs_copy.end(); ++it) {
    rp_vector_insert(rp_problem_ctrs(problem), *it);
    for(int i=0; i<rp_constraint_arity(*it); i++) {
      ++rp_variable_constrained(rp_problem_var(problem, rp_constraint_var(*it, i)));
    }
  }
  // 制約に依存していない変数のprecisionはRP_INFINITYに
  for(int i=0; i<rp_vector_size(rp_problem_vars(problem)); ++i) {
    if(rp_variable_constrained(rp_problem_var(problem, i))==0) {
      rp_variable_precision(rp_problem_var(problem, i)) = RP_INFINITY;
    }
  }
  rp_problem_set_initial_box(problem);
  HYDLA_LOGGER_DEBUG(
    "#*** constraint store: problem to solve ***\n",
    problem);

  // ソルバの作成
  rp_selector * select;
  rp_new(select,rp_selector_roundrobin,(&problem));
  rp_splitter * split;
  rp_new(split,rp_splitter_mixed,(&problem));
  rp_bpsolver solver(&problem,10,select,split);
  std::stringstream ss;
  rp_ofilter_text oft(&problem, &(ss), -1); // 結果表示用

  // 解いてhullを求める
  rp_box sol, tmp_box = solver.compute_next();
  assert(tmp_box != NULL);
  rp_box_clone(&sol, tmp_box);
  while((tmp_box=solver.compute_next()) != NULL) {
    //rp_box_display_simple_nl(tmp_box);
    rp_box_merge(sol, tmp_box);
  }
  oft.apply_box(sol, "");
  HYDLA_LOGGER_DEBUG(
    "#*** constraint store: variable_map(hull of ",
    solver.solution(),
    " boxes) ***",
    ss.str());
  rp_problem_destroy(&problem);

  // variable_mapを作成
  // prev変数は載せない
  var_name_map_t::right_const_iterator vnm_it;
  for(vnm_it=this->vars_.right.begin(); vnm_it!=this->vars_.right.end(); ++vnm_it) {
    if(vnm_it->info.prev_flag) continue; // prev変数は何もしない
    RPVariable bp_variable;
    std::string name(vnm_it->second);
    name.erase(0, var_prefix.length());
    std::string dc_str(boost::lexical_cast<std::string>(vnm_it->info.derivative_count));
    name.erase(0, dc_str.length());
    bp_variable.derivative_count = vnm_it->info.derivative_count;
    bp_variable.name = name;
    RPValue bp_value(rp_binf(rp_box_elem(sol, vnm_it->first)),
                     rp_bsup(rp_box_elem(sol, vnm_it->first)));
    variable_map.set_variable(bp_variable, bp_value);
  }
  // 後始末
  rp_box_destroy(&sol);
}

/**
 * ストアのコピーを返す
 * コピーでないと他のrp_**構造体に勝手にfreeされてしまう可能性がある
 */
std::set<rp_constraint> ConstraintStore::get_store_exprs_copy() const
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
 * ストアに制約を追加する
 * @param c 制約
 * @param vars 制約内の変数表
 */
void ConstraintStore::add_constraint(rp_constraint c, const var_name_map_t& vars)
  {
    this->exprs_.insert(c);
    this->vars_.insert(vars.begin(), vars.end());
}

/**
 * ストアに制約を追加する
 * @param start startからendにある制約を追加する
 * @param end startからendにある制約を追加する
 * @param vars 制約内の変数表
 */
void ConstraintStore::add_constraint(std::set<rp_constraint>::iterator start,
                                     std::set<rp_constraint>::iterator end,
                                     const var_name_map_t& vars)
{
  this->exprs_.insert(start, end);
  this->vars_.insert(vars.begin(), vars.end());
}

/**
 * ストア内容を表示する
 */
std::ostream& ConstraintStore::dump_cs(std::ostream& s) const
{
  rp_vector_variable vec = ConstraintSolver::create_rp_vector(this->vars_, prec_);
  std::set<rp_constraint>::const_iterator ctr_it = this->exprs_.begin();
  while(ctr_it != this->exprs_.end()){
    rp::dump_constraint(s, *ctr_it, vec); // digits, mode);
    s << "\n";
    ctr_it++;
  }
  s << "\n";
  rp_vector_destroy(&vec);
  return s;
} 

void ConstraintStore::set_precision(const double p)
{
  this->prec_ = p;
}

} // namespace realpaver
} // namespace vcs
} // namespace hydla

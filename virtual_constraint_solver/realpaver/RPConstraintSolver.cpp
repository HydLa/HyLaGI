#include "RPConstraintSolver.h"

#include <cassert>

#include "realpaver.h"

namespace hydla {
namespace vcs {
namespace realpaver {

typedef std::pair<double, double> interval_t;
typedef std::map<std::string, interval_t> dom_map_t;

bool
ConstraintSolver::solve_hull(rp_box* result,
                             const var_name_map_t& vars,
                             const std::set<rp_constraint>& ctrs,
                             const double precision)
{
  bool res = true;
  // TODO: どの制約からも参照されない変数のprecisionは無限大に
  rp_vector vec = ConstraintSolver::create_rp_vector(vars, precision);
  ConstraintSolver::create_initial_box(result, vars);
  for(std::set<rp_constraint>::const_iterator it=ctrs.begin();
    it!=ctrs.end(); it++) {
      if(*it==NULL) continue; // NULLポインタはスキップ
      assert(rp_constraint_type(*it) == RP_CONSTRAINT_NUMERICAL);
      int rel = rp_ctr_num_rel(rp_constraint_num(*it));
      switch(rel) {
      case RP_RELATION_EQUAL:
        if(!rp_sat_hull_eq(rp_constraint_num(*it), *result)) {
          res = false;
          rp_box_destroy(result);
        }
        break;
      case RP_RELATION_SUPEQUAL:
        if(!rp_sat_hull_sup(rp_constraint_num(*it), *result)) {
          res = false;
          rp_box_destroy(result);
        }
        break;
      case RP_RELATION_INFEQUAL:
        if(!rp_sat_hull_inf(rp_constraint_num(*it), *result)) {
          res =  false;
          rp_box_destroy(result);
        }
        break;
      }
      if(!res) break; // falseなら抜ける
  }
  rp_vector_destroy(&vec);
  return res;
}

bool
ConstraintSolver::solve_exact_hull(rp_box* result,
                                   const var_name_map_t& vars,
                                   const std::set<rp_constraint>& ctrs,
                                   const double precision)
{
  rp_problem problem;
  rp_problem_create(&problem, "solve_exact_hull");
  // 変数ベクタに変数を追加
  rp_vector_destroy(&rp_table_symbol_vars(rp_problem_symb(problem)));
  rp_table_symbol_vars(rp_problem_symb(problem)) = ConstraintSolver::create_rp_vector(vars);
  // 制約を追加
  for(std::set<rp_constraint>::const_iterator it=ctrs.begin();
    it!=ctrs.end(); it++) {
      if(*it==NULL) continue; // NULLポインタはスキップ
      rp_vector_insert(rp_problem_ctrs(problem), *it);
      for(int i=0; i<rp_constraint_arity(*it); i++) {
        ++rp_variable_constrained(rp_problem_var(problem, rp_constraint_var(*it, i)));
      }
  }
  rp_problem_set_initial_box(problem);
  // ソルバの作成
  rp_selector* select;
  rp_new(select,rp_selector_roundrobin,(&problem));
  rp_splitter* split;
  rp_new(split,rp_splitter_mixed,(&problem));
  rp_bpsolver solver(&problem,10,select,split);
  // 解いてhullを求める
  rp_box tmp_b = solver.compute_next();
  if(tmp_b==NULL) {
    // 制約以外をfree ... 自信ない
    rp_box_destroy(&rp_problem_box(problem));
    rp_free(rp_vector_ptr(rp_problem_ctrs(problem)));
    rp_free(rp_problem_ctrs(problem));
    rp_table_symbol_destroy(&rp_problem_symb(problem));
    rp_free(rp_problem_name(problem));
    rp_free(problem);
    return false;
  }

  rp_box_clone(result, tmp_b);
  while((tmp_b=solver.compute_next()) != NULL) {
    rp_box_merge(*result, tmp_b);
  }
  rp_box_destroy(&rp_problem_box(problem));
  rp_free(rp_vector_ptr(rp_problem_ctrs(problem)));
  rp_free(rp_problem_ctrs(problem));
  rp_table_symbol_destroy(&rp_problem_symb(problem));
  rp_free(rp_problem_name(problem));
  rp_free(problem);
  return true;
}

rp_vector_variable
ConstraintSolver::create_rp_vector(const var_name_map_t& vars,
                                   const double precision)
{
  rp_vector_variable vec;
  rp_vector_variable_create(&vec);
  var_name_map_t::right_const_iterator it;
  for(it=vars.right.begin(); it!=vars.right.end(); it++){
    rp_variable v;
    rp_variable_create(&v, ((it->second).c_str()));
    rp_interval interval;
    rp_interval_set_real_line(interval);
    rp_union_insert(rp_variable_domain(v), interval);
    rp_variable_precision(v) = precision;
    rp_vector_insert(vec, v);
  }
  return vec;
}

rp_vector_variable
ConstraintSolver::create_rp_vector(const var_name_map_t& vars,
                                   const dom_map_t& domain,
                                   const double precision)
{
  rp_vector_variable vec;
  rp_vector_variable_create(&vec);
  dom_map_t::const_iterator dit;
  var_name_map_t::right_const_iterator it;
  for(it=vars.right.begin(); it!=vars.right.end(); it++){
    rp_variable v;
    rp_variable_create(&v, ((it->second).c_str()));
    if((dit=domain.find(it->second))!=domain.end()) {
      rp_interval interval;
      rp_interval_set(interval, dit->second.first, dit->second.second);
      rp_union_insert(rp_variable_domain(v), interval);
    } else {
      rp_interval interval;
      rp_interval_set_real_line(interval);
      rp_union_insert(rp_variable_domain(v), interval);
    }
    rp_variable_precision(v) = precision;
    rp_vector_insert(vec, v);
  }
  return vec;
}

void ConstraintSolver::create_initial_box(rp_box* b, const var_name_map_t& vars)
{
  int s = vars.size();
  rp_box_create(b, s);
  for(int i=0; i<s; i++) {
    rp_interval_set_real_line(rp_box_elem(*b,i));
  }
}

} // namespace realapver
} // namespace vcs
} // namespace hydla

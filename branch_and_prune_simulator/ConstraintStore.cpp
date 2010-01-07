#include "ConstraintStore.h"
#include "realpaver.h"

namespace hydla {
namespace bp_simulator {

  ConstraintStore::ConstraintStore(bool debug_mode) :
  debug_mode_(debug_mode)
  {}

  ConstraintStore::ConstraintStore(const ConstraintStore& src) :
  debug_mode_(src.debug_mode_)
  {
    this->exprs_ = src.get_store_exprs_copy();
    this->vars_ = src.vars_;
  }

  ConstraintStore::~ConstraintStore()
  {
    std::set<rp_constraint>::iterator it = this->exprs_.begin();
    while(it != this->exprs_.end()) {
      rp_constraint c = *it;
      rp_constraint_destroy(&c);
      this->exprs_.erase(it++);
    }
  }

  /**
   * variable_mapからストアを構築する
   * prev_var <- var の変換はここでおこなう
   * @param variable_map 変数表
   * TODO: ちゃんと書く
   */
  void ConstraintStore::build(const variable_map_t& variable_map)
  {
  }

  /**
   * ストアからvariable_mapを作成する
   * @param variable_map 作成先の変数表
   */
  void ConstraintStore::build_variable_map(variable_map_t& variable_map) const
  {
    // 問題の生成
    // TODO: precisionは？
    std::set<rp_constraint> exprs_copy = this->get_store_exprs_copy();
    rp_vector_variable vec = this->to_rp_vector();
    rp_problem problem;
    rp_problem_create(&problem, "build_variable_map");
    rp_vector_destroy(&rp_table_symbol_vars(rp_problem_symb(problem)));
    rp_table_symbol_vars(rp_problem_symb(problem)) = vec;
    std::set<rp_constraint>::const_iterator it;
    for(it=exprs_copy.begin(); it!=exprs_copy.end(); it++) {
      rp_vector_insert(rp_problem_ctrs(problem), *it);
      for(int i=0; i<rp_constraint_arity(*it); i++) {
        ++rp_variable_constrained(rp_problem_var(problem, rp_constraint_var(*it, i)));
      }
    }
    rp_problem_set_initial_box(problem);
    if(this->debug_mode_) {
      std::cout << "#*** constraint store: problem to solve ***\n";
      rp_problem_display(stdout, problem);
      std::cout << "\n";
    }

    // ソルバの作成
    rp_selector * select;
    //rp_new(select,rp_selector_decirdom,(&problem));
    //rp_new(select,rp_selector_decirrobust,(&problem,1));
    rp_new(select,rp_selector_roundrobin,(&problem));

    rp_splitter * split;
    rp_new(split,rp_splitter_mixed,(&problem));
    //rp_new(split,rp_splitter_bisection,(&problem));

    //rp_interval_satisfaction_prover * prover;
    //rp_new(prover,rp_interval_satisfaction_prover,(&problem,100000));

    rp_bpsolver solver(&problem,10,select,split); //,prover);
    rp_ofilter_text oft(&problem, &(std::cout), -1);

    // 解いてhullを求める
    rp_box sol, tmp_box = solver.compute_next();
    assert(tmp_box != NULL);
    rp_box_clone(&sol, tmp_box);
    while((tmp_box=solver.compute_next()) != NULL) {
      rp_box_merge(sol, tmp_box);
    }
    if(this->debug_mode_) {
      std::cout << "#*** constraint store: variable_map(hull of "
        << solver.solution() << " boxes) ***\n";
      //rp_box_display_simple_nl(sol);
      oft.apply_box(sol, "");
    }
    rp_problem_destroy(&problem);

    // variable_mapを作成
    // prev変数は載せない(?)
    var_name_map_t::right_const_iterator vnm_it;
    for(vnm_it=this->vars_.right.begin(); vnm_it!=this->vars_.right.end(); vnm_it++) {
      if(!(vnm_it->info.prev_flag)) { // prev変数でなければ
        BPVariable bp_variable;
        // derivative_countだけ変数名を縮める
        std::string name(vnm_it->second);
        for(int i=vnm_it->info.derivative_count; i>0; i--) {
          int loc = name.rfind(BP_DERIV_STR);
          assert(loc != std::string::npos);
          name.erase(loc);
        }
        bp_variable.derivative_count = vnm_it->info.derivative_count;
        bp_variable.name = name;
        BPValue bp_value(rp_box_elem(sol, vnm_it->first));
        variable_map.set_variable(bp_variable, bp_value);
      }
    }
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
   * ストアを表示する
   * TODO: ダンプ形式にする？
   */
  void ConstraintStore::display(const int digits) const
  {
    rp_vector_variable vec = this->to_rp_vector();
    std::set<rp_constraint>::const_iterator ctr_it = this->exprs_.begin();
    while(ctr_it != this->exprs_.end()){
      rp_constraint_display(stdout, *ctr_it, vec, digits);
      std::cout << "\n";
      ctr_it++;
    }
    std::cout << "\n";
    rp_vector_destroy(&vec);
  }

  /**
   * vars_をrp_vector_variableに変換
   * 変数の値は(-oo, +oo)
   * TODO: ConstraintBuilderにもあるよ
   */
  rp_vector_variable ConstraintStore::to_rp_vector() const
  {
    rp_vector_variable vec;
    rp_vector_variable_create(&vec);
    var_name_map_t::right_const_iterator it;
    for(it=this->vars_.right.begin(); it!=this->vars_.right.end(); it++){
      rp_variable v;
      rp_variable_create(&v, ((it->second).c_str()));
      rp_variable_set_decision(v);
      rp_interval interval;
      rp_interval_set(interval,(-1)*RP_INFINITY,RP_INFINITY);
      rp_union_insert(rp_variable_domain(v), interval);
      rp_vector_insert(vec, v);
    }
    return vec;
  }

} // namespace bp_simulator
} // namespace hydla

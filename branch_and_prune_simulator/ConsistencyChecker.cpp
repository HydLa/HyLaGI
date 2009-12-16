#include "ConsistencyChecker.h"

#include <iostream>
#include <cassert>

#include "realpaver.h"

using namespace hydla::parse_tree;
using namespace hydla::simulator;

//小数を何桁まで表示するか
#define DISPLAY_DIGITS 10

using namespace hydla::simulator;

namespace hydla {
namespace bp_simulator {

ConsistencyChecker::ConsistencyChecker() :
  debug_mode_(false)
{}

ConsistencyChecker::ConsistencyChecker(bool debug_mode) :
  debug_mode_(debug_mode)
{}

ConsistencyChecker::~ConsistencyChecker()
{}

// Tell制約
void ConsistencyChecker::visit(boost::shared_ptr<Tell> node)                  
{
  rp_constraint c;
  this->accept(node->get_child());
  rp_constraint_create_num(&c, this->ctr_);
  this->constraints_.insert(c);
  this->ctr_ = NULL;
}

bool ConsistencyChecker::is_consistent(TellCollector::tells_t& collected_tells)
{
  // rp_constraint集合を生成
  TellCollector::tells_t::iterator tells_it = collected_tells.begin();
  while((tells_it) != collected_tells.end()){
    this->accept(*tells_it);
    tells_it++;
  }

  // 作成できたか確認
  rp_vector_variable vec = this->to_rp_vector();
  if(this->debug_mode_){
    std::cout << "#**** tells expression ****\n";
    std::set<rp_constraint>::iterator it = this->constraints_.begin();
    while(it != this->constraints_.end()){
      rp_constraint_display(stdout, *it, vec, DISPLAY_DIGITS);
      std::cout << "\n";
      it++;
    }
  }
  rp_vector_destroy(&vec);

  // 問題とソルバを作成し,解いてチェック
  rp_problem problem;
  rp_problem_create(&problem, "consistency_check");

  // 変数ベクタに変数を追加
  int size = this->vars_.size();
  for(int i=0; i<size; i++){
    rp_variable v;
    rp_variable_create(&v, ((this->vars_.right.at(i)).c_str()));
    // TODO: すべての変数は初期値[-oo,+oo]をもつ,ここで入れるとよいかも
    rp_interval interval;
    rp_interval_set(interval,(-1)*RP_INFINITY,RP_INFINITY);
    rp_union_insert(rp_variable_domain(v), interval);
    rp_vector_insert(rp_problem_vars(problem), v);
  }

  // TODO: 制約ベクタに制約を追加
  std::set<rp_constraint>::iterator it = this->constraints_.begin();
  while(it != this->constraints_.end())
  {
    rp_vector_insert(rp_problem_ctrs(problem), *it);
    for(int i=0; i<rp_constraint_arity(*it); i++)
    {
      ++rp_variable_constrained(rp_problem_var(problem, rp_constraint_var(*it, i)));
    }
    it++;
  }

  // 変数の初期値からボックスが自動作成される
  rp_problem_set_initial_box(problem);

  // TODO: ソルバを作成して求解
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

  rp_problem_display(stdout,problem);
  std::cout << "\n";

  rp_box sol;
  sol = solver.compute_next();

  // 後始末
  it = this->constraints_.begin();
  while(it != this->constraints_.end()){
    rp_constraint_destroy(((rp_constraint *)&(*it)));
    this->constraints_.erase(it++);
  }

  // return ソルバから解が一つでも出力させたか？
  return (sol != NULL);
}

/**
 * vars_をrp_vector_variableに変換
 * TODO: 変数値を正しく設定する
 */
rp_vector_variable ConsistencyChecker::to_rp_vector()
{
  rp_vector_variable vec;
  rp_vector_variable_create(&vec);
  int size = this->vars_.size();
  for(int i=0; i<size; i++){
    rp_variable v;
    rp_variable_create(&v, ((this->vars_.right.at(i)).c_str()));
    rp_vector_insert(vec, v);
  }
  return vec;
}

} //namespace bp_simulator
} // namespace hydla

#include "ConsistencyChecker.h"
#include "Logger.h"

#include <iostream>
#include <sstream>
#include <cassert>

#include "realpaver.h"
#include "rp_constraint_ext.h"

using namespace hydla::parse_tree;
using namespace hydla::simulator;

//�����������܂ŕ\�����邩
#define DISPLAY_DIGITS 10

using namespace hydla::simulator;

namespace hydla {
namespace bp_simulator {

ConsistencyChecker::ConsistencyChecker(){}

ConsistencyChecker::~ConsistencyChecker(){}

// Tell����
void ConsistencyChecker::visit(boost::shared_ptr<Tell> node)                  
{
  rp_constraint c;
  this->accept(node->get_child());
  rp_constraint_create_num(&c, this->ctr_);
  this->constraints_.insert(c);
  this->ctr_ = NULL;
}

bool ConsistencyChecker::is_consistent(tells_t& collected_tells, ConstraintStore& constraint_store)
{
  // �X�g�A�̕ϐ������R�s�[
  this->vars_ = constraint_store.get_store_vars();
  // rp_constraint�W���𐶐�
  tells_t::iterator tells_it = collected_tells.begin();
  while((tells_it) != collected_tells.end()){
    this->accept(*tells_it);
    tells_it++;
  }
  // tell����(rp_constraint)���R�s�[���Ă���
  std::set<rp_constraint> tells_ctr_copy;
  std::set<rp_constraint>::iterator ctr_it = this->constraints_.begin();
  while(ctr_it != this->constraints_.end()) {
    rp_constraint c;
    rp_constraint_clone(&c, *ctr_it);
    tells_ctr_copy.insert(c);
    ctr_it++;
  }

  // �X�g�A�̐����ǉ�
  std::set<rp_constraint> store_expr_copy = constraint_store.get_store_exprs_copy();
  this->constraints_.insert(store_expr_copy.begin(), store_expr_copy.end());

  // �쐬�ł������m�F
  rp_vector_variable vec = this->to_rp_vector();
  {
    HYDLA_LOGGER_DEBUG("#**** consistency check: constraints expression ****\n");
    std::stringstream ss;
    std::set<rp_constraint>::iterator it = this->constraints_.begin();
    while(it != this->constraints_.end()){
      rp::dump_constraint(ss, *it, vec, DISPLAY_DIGITS);
      ss << "\n";
      it++;
    }
    HYDLA_LOGGER_DEBUG(ss.str());
  }

  // ���ƃ\���o���쐬��,�����ă`�F�b�N
  rp_problem problem;
  rp_problem_create(&problem, "consistency_check");

  // �ϐ��x�N�^�ɕϐ���ǉ�
  rp_vector_destroy(&rp_table_symbol_vars(rp_problem_symb(problem)));
  rp_table_symbol_vars(rp_problem_symb(problem)) = vec;

  // ����x�N�^�ɐ����ǉ�
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

  // �ϐ��̏����l����{�b�N�X�������쐬�����
  rp_problem_set_initial_box(problem);

  // �\���o���쐬���ċ���
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

  HYDLA_LOGGER_DEBUG(
    "#**** consistency check: problem to solve ****\n",
    problem);

  rp_box sol;
  sol = solver.compute_next();

  // ��n�� problem�̃����o�͂��ׂĂ���ő|�������Csolver�͊֐��𔲂���Ə�����
  rp_problem_destroy(&problem);

  // return �\���o���������ł��o�͂��ꂽ���H
  if(sol != NULL) {
    constraint_store.add_constraint(tells_ctr_copy.begin(),
                                    tells_ctr_copy.end(), this->vars_);
    {
      HYDLA_LOGGER_DEBUG("#*** consistency check ==> Consistent ***\n",
        "#**** consistency check: new constraint_store ***\n",
        constraint_store);
    }
    return true;
  } else {
    HYDLA_LOGGER_DEBUG("#*** consistency check ==> Inconsistent ***\n");
    ctr_it = tells_ctr_copy.begin();
    while(ctr_it != tells_ctr_copy.end()) {
      rp_constraint c = *ctr_it;
      rp_constraint_destroy(&c);
      ctr_it++;
    }
    return false;
  }
}

///**
// * vars_��rp_vector_variable�ɕϊ�
// * �ϐ��̒l�͐������ݒ肳��Ă��Ȃ�
// */
//rp_vector_variable ConsistencyChecker::to_rp_vector()
//{
//  rp_vector_variable vec;
//  rp_vector_variable_create(&vec);
//  int size = this->vars_.size();
//  for(int i=0; i<size; i++){
//    rp_variable v;
//    rp_variable_create(&v, ((this->vars_.right.at(i)).c_str()));
//    rp_vector_insert(vec, v);
//  }
//  return vec;
//}

} //namespace bp_simulator
} // namespace hydla

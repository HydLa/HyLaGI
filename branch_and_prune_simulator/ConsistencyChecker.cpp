#include "ConsistencyChecker.h"

#include <iostream>
#include <cassert>

#include "realpaver.h"

using namespace hydla::parse_tree;
using namespace hydla::simulator;

//�����������܂ŕ\�����邩
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

// Tell����
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
  // rp_constraint�W���𐶐�
  TellCollector::tells_t::iterator tells_it = collected_tells.begin();
  while((tells_it) != collected_tells.end()){
    this->accept(*tells_it);
    tells_it++;
  }

  // �쐬�ł������m�F
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

  // ���ƃ\���o���쐬��,�����ă`�F�b�N
  rp_problem problem;
  rp_problem_create(&problem, "consistency_check");

  // �ϐ��x�N�^�ɕϐ���ǉ�
  int size = this->vars_.size();
  for(int i=0; i<size; i++){
    rp_variable v;
    rp_variable_create(&v, ((this->vars_.right.at(i)).c_str()));
    // TODO: ���ׂĂ̕ϐ��͏����l[-oo,+oo]������,�����œ����Ƃ悢����
    rp_interval interval;
    rp_interval_set(interval,(-1)*RP_INFINITY,RP_INFINITY);
    rp_union_insert(rp_variable_domain(v), interval);
    rp_vector_insert(rp_problem_vars(problem), v);
  }

  // TODO: ����x�N�^�ɐ����ǉ�
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

  // TODO: �\���o���쐬���ċ���
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

  // ��n��
  it = this->constraints_.begin();
  while(it != this->constraints_.end()){
    rp_constraint_destroy(((rp_constraint *)&(*it)));
    this->constraints_.erase(it++);
  }

  // return �\���o���������ł��o�͂��������H
  return (sol != NULL);
}

/**
 * vars_��rp_vector_variable�ɕϊ�
 * TODO: �ϐ��l�𐳂����ݒ肷��
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

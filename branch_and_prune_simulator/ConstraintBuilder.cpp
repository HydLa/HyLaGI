#include "ConstraintBuilder.h"

#include <iostream>
#include <cassert>

using namespace hydla::parse_tree;
using namespace hydla::simulator;

#define DISPLAY_DIGITS 10

using namespace hydla::simulator;

namespace hydla {
namespace bp_simulator {

ConstraintBuilder::ConstraintBuilder() :
  in_differential_(false),
  in_prev_(false)
{}

ConstraintBuilder::~ConstraintBuilder()
{}

// Tell����
//void ConstraintBuilder::visit(boost::shared_ptr<Tell> node)
//{
//}

// �_�����Z�q
//void ConstraintBuilder::visit(boost::shared_ptr<LogicalAnd> node)
//{
//}

//void ConstraintBuilder::visit(boost::shared_ptr<LogcialOr> node)
//{
//}

// ��r���Z�q
void ConstraintBuilder::visit(boost::shared_ptr<Equal> node)
{
  this->create_ctr_num(node, RP_RELATION_EQUAL);
}

void ConstraintBuilder::visit(boost::shared_ptr<UnEqual> node)
{
  // TODO: ���S�ɊԈ���Ă���CUnEqual�̋�ԋߎ��́u����Ȃ��v
  //this->create_ctr_num(node, RP_RELATION_UNEQUAL);
  this->create_ctr_num(node, RP_RELATION_EQUAL);
}

void ConstraintBuilder::visit(boost::shared_ptr<Less> node)
{
  //this->create_ctr_num(node, RP_RELATION_INF);
  this->create_ctr_num(node, RP_RELATION_INFEQUAL);
}

void ConstraintBuilder::visit(boost::shared_ptr<LessEqual> node)
{
  this->create_ctr_num(node, RP_RELATION_INFEQUAL);
}

void ConstraintBuilder::visit(boost::shared_ptr<Greater> node)
{
  //this->create_ctr_num(node, RP_RELATION_SUP);
  this->create_ctr_num(node, RP_RELATION_SUPEQUAL);
}

void ConstraintBuilder::visit(boost::shared_ptr<GreaterEqual> node)
{
  this->create_ctr_num(node, RP_RELATION_SUPEQUAL);
}
  
// �Z�p�񍀉��Z�q
void ConstraintBuilder::visit(boost::shared_ptr<Plus> node)
{
  create_binary_erep(node, RP_SYMBOL_ADD);
}

void ConstraintBuilder::visit(boost::shared_ptr<Subtract> node)
{
  create_binary_erep(node, RP_SYMBOL_SUB);
}

void ConstraintBuilder::visit(boost::shared_ptr<Times> node)
{
  create_binary_erep(node, RP_SYMBOL_MUL);
}

void ConstraintBuilder::visit(boost::shared_ptr<Divide> node)
{
  // TODO: 0���Z�ȂǓ���ȏ��Z�ɂ���
  create_binary_erep(node, RP_SYMBOL_DIV);
}
  
// �Z�p�P�����Z�q
void ConstraintBuilder::visit(boost::shared_ptr<Negative> node)
{
  create_unary_erep(node, RP_SYMBOL_NEG);
}

void ConstraintBuilder::visit(boost::shared_ptr<Positive> node)
{
  this->accept(node->get_child());
}

// ����
void ConstraintBuilder::visit(boost::shared_ptr<Differential> node)
{
  this->in_differential_ = true;
  this->accept(node->get_child());
  this->in_differential_ = false;
}

// ���Ɍ�
void ConstraintBuilder::visit(boost::shared_ptr<Previous> node)
{
  this->in_prev_ = true;
  this->accept(node->get_child());
  this->in_prev_ = false;
}

// �ϐ�
void ConstraintBuilder::visit(boost::shared_ptr<Variable> node)
{
  typedef boost::bimaps::bimap<std::string, int>::value_type vars_type_t;

  // �ϐ��\�ɓo�^ ������ύX�c�H
  std::string name(node->get_name());
  unsigned int size = this->vars_.size();
  //assert(!(this->in_differential_ & this->in_prev_)); // �ǂ��炩��false�c�Ȃ킯�Ȃ�
  if(this->in_differential_) name += BP_DERIV_STR;
  if(this->in_prev_) name += BP_PREV_STR;
  this->vars_.insert(vars_type_t(name, size)); // �o�^�ς݂̕ϐ��͕ύX����Ȃ�

  // TODO: ����̕ϐ��͒萔�������Ȃ���prove�ł��Ȃ��\��
  rp_erep rep;
  rp_erep_create_var(&rep, this->vars_.left.at(name));
  this->rep_stack_.push(rep);
}

// ����
void ConstraintBuilder::visit(boost::shared_ptr<Number> node)
{
  rp_interval i;
  rp_interval_from_str(const_cast<char *>(node->get_number().c_str()), i);
  rp_erep rep;
  rp_erep_create_cst(&rep, "", i);
  this->rep_stack_.push(rep);
}

/**
 * �P�����Z��rp_erep������ăX�^�b�N�ɐς�
 */
void ConstraintBuilder::create_unary_erep(boost::shared_ptr<UnaryNode> node, int op)
{
  rp_erep child, rep;

  this->accept(node->get_child());
  child = this->rep_stack_.top();
  assert(!(this->rep_stack_.empty()));
  this->rep_stack_.pop();

  rp_erep_create_unary(&rep, op, child);
  this->rep_stack_.push(rep);
  rp_erep_destroy(&child);
  assert(child);
}

/**
 * �񍀉��Z��rp_erep������ăX�^�b�N�ɐς�
 */
void ConstraintBuilder::create_binary_erep(boost::shared_ptr<BinaryNode> node, int op)
{
  rp_erep l, r, rep;

  this->accept(node->get_lhs());
  l = this->rep_stack_.top();
  assert(!(this->rep_stack_.empty()));
  this->rep_stack_.pop();

  this->accept(node->get_rhs());
  r = this->rep_stack_.top();
  assert(!(this->rep_stack_.empty()));
  this->rep_stack_.pop();

  rp_erep_create_binary(&rep,op,l,r);
  this->rep_stack_.push(rep);
  rp_erep_destroy(&l);
  rp_erep_destroy(&r);
  assert(l); assert(r);
}

/**
 * rp_ctr_num�����
 */
void ConstraintBuilder::create_ctr_num(boost::shared_ptr<BinaryNode> node, int rel)
{
  rp_erep l, r;

  this->accept(node->get_lhs());
  l = this->rep_stack_.top();
  assert(!(this->rep_stack_.empty()));
  this->rep_stack_.pop();

  this->accept(node->get_rhs());
  r = this->rep_stack_.top();
  assert(!(this->rep_stack_.empty()));
  this->rep_stack_.pop();

  rp_ctr_num_create(&(this->ctr_), &l, rel, &r);
}

/**
 * vars_��rp_vector_variable�ɕϊ�
 * �ϐ��̒l��(-oo, +oo)
 */
rp_vector_variable ConstraintBuilder::to_rp_vector() const
{
  rp_vector_variable vec;
  rp_vector_variable_create(&vec);
  int size = this->vars_.size();
  boost::bimaps::bimap<std::string, int>::right_const_iterator it;
  for(it=this->vars_.right.begin(); it!=this->vars_.right.end(); it++){
    rp_variable v;
    rp_variable_create(&v, ((it->second).c_str()));
    rp_interval interval;
    rp_interval_set(interval,(-1)*RP_INFINITY,RP_INFINITY);
    rp_union_insert(rp_variable_domain(v), interval);
    rp_vector_insert(vec, v);
  }
  return vec;
}

} //namespace bp_simulator
} // namespace hydla

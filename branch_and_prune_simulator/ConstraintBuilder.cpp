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
  derivative_count_(0),
  in_prev_(false)
{}

ConstraintBuilder::~ConstraintBuilder()
{}

// ”äŠr‰‰Zq
void ConstraintBuilder::visit(boost::shared_ptr<Equal> node)
{
  this->create_ctr_num(node, RP_RELATION_EQUAL);
}

void ConstraintBuilder::visit(boost::shared_ptr<UnEqual> node)
{
  this->ctr_ = NULL;
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
  
// Zp“ñ€‰‰Zq
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
  // TODO: 0œZ‚È‚Ç“Áê‚ÈœZ‚É‚Â‚¢‚Ä
  create_binary_erep(node, RP_SYMBOL_DIV);
}
  
// Zp’P€‰‰Zq
void ConstraintBuilder::visit(boost::shared_ptr<Negative> node)
{
  create_unary_erep(node, RP_SYMBOL_NEG);
}

void ConstraintBuilder::visit(boost::shared_ptr<Positive> node)
{
  this->accept(node->get_child());
}

// ”÷•ª
void ConstraintBuilder::visit(boost::shared_ptr<Differential> node)
{
  this->derivative_count_++;
  this->accept(node->get_child());
  this->derivative_count_--;

}

// ¶‹ÉŒÀ
void ConstraintBuilder::visit(boost::shared_ptr<Previous> node)
{
  this->in_prev_ = true;
  this->accept(node->get_child());
  this->in_prev_ = false;
}

// •Ï”
void ConstraintBuilder::visit(boost::shared_ptr<Variable> node)
{
  typedef boost::bimaps::bimap<std::string, int>::value_type vars_type_t;

  // •Ï”•\‚É“o˜^ ‚¢‚¸‚ê•ÏXcH
  std::string name(node->get_name());
  unsigned int size = this->vars_.size();
  for(unsigned int i=0; i< this->derivative_count_; i++) name += BP_DERIV_STR;
  if(this->in_prev_) name += BP_PREV_STR;
  this->vars_.insert(vars_type_t(name, size)); // “o˜^Ï‚İ‚Ì•Ï”‚Í•ÏX‚³‚ê‚È‚¢

  // TODO: “Á’è‚Ì•Ï”‚Í’è”ˆµ‚¢‚µ‚È‚¢‚Æprove‚Å‚«‚È‚¢‰Â”\«
  rp_erep rep;
  rp_erep_create_var(&rep, this->vars_.left.at(name));
  this->rep_stack_.push(rep);
}

// ”š
void ConstraintBuilder::visit(boost::shared_ptr<Number> node)
{
  rp_interval i;
  rp_interval_from_str(const_cast<char *>(node->get_number().c_str()), i);
  rp_erep rep;
  rp_erep_create_cst(&rep, "", i);
  this->rep_stack_.push(rep);
}

/**
 * ’P€‰‰Z‚Ìrp_erep‚ğì‚Á‚ÄƒXƒ^ƒbƒN‚ÉÏ‚Ş
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
 * “ñ€‰‰Z‚Ìrp_erep‚ğì‚Á‚ÄƒXƒ^ƒbƒN‚ÉÏ‚Ş
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
 * rp_ctr_num‚ğì‚é
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
 * vars_‚ğrp_vector_variable‚É•ÏŠ·
 * •Ï”‚Ì’l‚Í(-oo, +oo)
 */
rp_vector_variable ConstraintBuilder::to_rp_vector() const
{
  rp_vector_variable vec;
  rp_vector_variable_create(&vec);
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

/******************** GuardConstraintBuilder ********************/

void GuardConstraintBuilder::visit(boost::shared_ptr<Ask> node)
{
  this->accept(node->get_guard());
}

void GuardConstraintBuilder::visit(boost::shared_ptr<Equal> node)
{
  rp_constraint a;
  this->create_ctr_num(node, RP_RELATION_EQUAL);
  rp_constraint_create_num(&a, this->ctr_);
  this->guards_.insert(a);
  this->ctr_ = NULL;
  // not_a ‚Íu§–ñ‚È‚µv‚ğ•\‚·‚½‚ßNULL‚ğ‘}“ü‚·‚éCˆµ‚¢‚É’ˆÓI
  this->not_guards_.insert(static_cast<rp_constraint>(NULL));
}

void GuardConstraintBuilder::visit(boost::shared_ptr<UnEqual> node)
{
  // a ‚Í§–ñ‚È‚µ
  rp_constraint not_a;
  this->create_ctr_num(node, RP_RELATION_EQUAL);
  rp_constraint_create_num(&not_a, this->ctr_);
  this->not_guards_.insert(not_a);
  this->ctr_ = NULL;
}

void GuardConstraintBuilder::visit(boost::shared_ptr<Less> node)
{
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

void GuardConstraintBuilder::visit(boost::shared_ptr<LessEqual> node)
{
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

void GuardConstraintBuilder::visit(boost::shared_ptr<Greater> node)
{
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

void GuardConstraintBuilder::visit(boost::shared_ptr<GreaterEqual> node)
{
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

void GuardConstraintBuilder::visit(boost::shared_ptr<LogicalAnd> node)
{
  this->accept(node->get_lhs());
  this->accept(node->get_rhs());
}

void GuardConstraintBuilder::create_guard_expr(boost::shared_ptr<Ask> node,
                                               std::set<rp_constraint>& guards,
                                               std::set<rp_constraint>& not_guards,
                                               boost::bimaps::bimap<std::string, int>& vars)
{
  this->accept(node);
  guards.insert(this->guards_.begin(), this->guards_.end());
  not_guards.insert(this->not_guards_.begin(), this->not_guards_.end());
  vars.insert(this->vars_.begin(), this->vars_.end());
}

} //namespace bp_simulator
} // namespace hydla

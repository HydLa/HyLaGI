#include "ConsistencyChecker.h"

#include <iostream>
#include <cassert>

using namespace hydla::parse_tree;
using namespace hydla::simulator;

#define DISPLAY_DIGITS 10

using namespace hydla::simulator;

namespace hydla {
namespace bp_simulator {

ConsistencyChecker::ConsistencyChecker() :
  debug_mode_(false),
  in_differential_(false),
  in_prev_(false)
{}

ConsistencyChecker::ConsistencyChecker(bool debug_mode) :
  debug_mode_(debug_mode),
  in_differential_(false),
  in_prev_(false)
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

// ��r���Z�q
void ConsistencyChecker::visit(boost::shared_ptr<Equal> node)                 
{
  this->create_ctr_num(node, RP_RELATION_EQUAL);
}

void ConsistencyChecker::visit(boost::shared_ptr<UnEqual> node)               
{
  //this->create_ctr_num(node, RP_RELATION_UNEQUAL);
  this->create_ctr_num(node, RP_RELATION_EQUAL);
}

void ConsistencyChecker::visit(boost::shared_ptr<Less> node)                  
{
  //this->create_ctr_num(node, RP_RELATION_INF);
  this->create_ctr_num(node, RP_RELATION_INFEQUAL);
}

void ConsistencyChecker::visit(boost::shared_ptr<LessEqual> node)             
{
  this->create_ctr_num(node, RP_RELATION_INFEQUAL);
}

void ConsistencyChecker::visit(boost::shared_ptr<Greater> node)               
{
  //this->create_ctr_num(node, RP_RELATION_SUP);
  this->create_ctr_num(node, RP_RELATION_SUPEQUAL);
}

void ConsistencyChecker::visit(boost::shared_ptr<GreaterEqual> node)          
{
  this->create_ctr_num(node, RP_RELATION_SUPEQUAL);
}
  
// �Z�p�񍀉��Z�q
void ConsistencyChecker::visit(boost::shared_ptr<Plus> node)                  
{
  create_binary_erep(node, RP_SYMBOL_ADD);
}

void ConsistencyChecker::visit(boost::shared_ptr<Subtract> node)              
{
  create_binary_erep(node, RP_SYMBOL_SUB);
}

void ConsistencyChecker::visit(boost::shared_ptr<Times> node)                 
{
  create_binary_erep(node, RP_SYMBOL_MUL);
}

void ConsistencyChecker::visit(boost::shared_ptr<Divide> node)                
{
  // TODO: 0���Z�ȂǓ���ȏ��Z�ɂ���
  create_binary_erep(node, RP_SYMBOL_DIV);
}
  
// �Z�p�P�����Z�q
void ConsistencyChecker::visit(boost::shared_ptr<Negative> node)              
{
  create_unary_erep(node, RP_SYMBOL_NEG);
}

void ConsistencyChecker::visit(boost::shared_ptr<Positive> node)              
{
  this->accept(node->get_child());
}

// ����
void ConsistencyChecker::visit(boost::shared_ptr<Differential> node)          
{
  this->in_differential_ = true;
  this->accept(node->get_child());
  this->in_differential_ = false;
}

// ���Ɍ�
void ConsistencyChecker::visit(boost::shared_ptr<Previous> node)              
{
  this->in_prev_ = true;
  this->accept(node->get_child());
  this->in_prev_ = false;
}

// �ϐ�
void ConsistencyChecker::visit(boost::shared_ptr<Variable> node)              
{
  typedef boost::bimaps::bimap<std::string, int>::value_type vars_type_t;

  // �ϐ��\�ɓo�^ ������ύX�c�H
  std::string name(node->get_name());
  unsigned int size = this->vars_.size();
  assert(!(this->in_differential_ & this->in_prev_)); // �ǂ��炩��false
  if(this->in_differential_) name += "_d";
  if(this->in_prev_) name += "_p";
  this->vars_.insert(vars_type_t(name, size)); // �o�^�ς݂̕ϐ��͕ύX����Ȃ�

  // TODO: ����̕ϐ��͒萔�������Ȃ���prove�ł��Ȃ��\��
  rp_erep rep;
  rp_erep_create_var(&rep, this->vars_.left.at(name));
  this->rep_stack_.push(rep);
}

// ����
void ConsistencyChecker::visit(boost::shared_ptr<Number> node)                
{
  rp_interval i;
  rp_interval_from_str(const_cast<char *>(node->get_number().c_str()), i);
  rp_erep rep;
  rp_erep_create_cst(&rep, "", i);
  this->rep_stack_.push(rep);
}

bool ConsistencyChecker::is_consistent(TellCollector::tells_t& collected_tells)
{
  //typedef std::set<boost::shared_ptr<hydla::parse_tree::Tell> > collected_tells_t;
  TellCollector::tells_t::iterator tells_it = collected_tells.begin();
  while((tells_it) != collected_tells.end()){
    this->accept(*tells_it);
    tells_it++;
  }
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

  std::set<rp_constraint>::iterator it = this->constraints_.begin();
  while(it != this->constraints_.end()){
    rp_constraint_destroy(&(*it));
    this->constraints_.erase(it++);
  }

  // return []solve(???) != true
  return true;
}

/**
 * �P�����Z��rp_erep������ăX�^�b�N�ɐς�
 */
void ConsistencyChecker::create_unary_erep(boost::shared_ptr<UnaryNode> node, int op)
{
  rp_erep child, rep;

  this->accept(node->get_child());
  rp_erep_set(&child, this->rep_stack_.top());
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
void ConsistencyChecker::create_binary_erep(boost::shared_ptr<BinaryNode> node, int op)
{
  rp_erep l, r, rep;

  this->accept(node->get_lhs());
  rp_erep_set(&l, this->rep_stack_.top());
  assert(!(this->rep_stack_.empty()));
  this->rep_stack_.pop();

  this->accept(node->get_rhs());
  rp_erep_set(&r, this->rep_stack_.top());
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
void ConsistencyChecker::create_ctr_num(boost::shared_ptr<BinaryNode> node, int rel)
{
  rp_erep l, r;

  this->accept(node->get_lhs());
  rp_erep_set(&l, this->rep_stack_.top());
  assert(!(this->rep_stack_.empty()));
  this->rep_stack_.pop();

  this->accept(node->get_rhs());
  rp_erep_set(&r, this->rep_stack_.top());
  assert(!(this->rep_stack_.empty()));
  this->rep_stack_.pop();

  rp_ctr_num_create(&(this->ctr_), &l, rel, &r);
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

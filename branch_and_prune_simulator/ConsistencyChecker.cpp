#include "ConsistencyChecker.h"

#include <iostream>
#include <cassert>

using namespace hydla::parse_tree;
using namespace hydla::simulator;

namespace hydla {
namespace bp_simulator {

ConsistencyChecker::ConsistencyChecker() :
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

// �_�����Z�q
void ConsistencyChecker::visit(boost::shared_ptr<LogicalAnd> node)            
{
}

void ConsistencyChecker::visit(boost::shared_ptr<LogicalOr> node)             
{
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
  // �ϐ��\�ɓo�^ ������ύX�c�H
  std::string name(node->get_name());
  unsigned int size = this->vars_.size();
  assert(!(this->in_differential_ & this->in_prev_)); // �ǂ��炩��false
  if(this->in_differential_) name += "_d";
  if(this->in_prev_) name += "_p";
  this->vars_.insert(std::pair<std::string, int>(name, size)); // �o�^�ς݂̕ϐ��͕ύX����Ȃ�

  // TODO: ����̕ϐ��͒萔�������Ȃ���prove�ł��Ȃ��\��
  rp_erep rep;
  rp_erep_create_var(&rep, this->vars_[name]);
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


bool ConsistencyChecker::is_consistent(collected_tells_t& collected_tells)
{
  //typedef std::set<boost::shared_ptr<hydla::parse_tree::Tell> > collected_tells_t;
  // tell����̏W������expr�𓾂�Mathematica�ɓn��
  collected_tells_t::iterator tells_it = collected_tells.begin();
  while((tells_it) != collected_tells.end())
  {
    this->accept(*tells_it);
    tells_it++;
  }
//
//
//  // vars��n��
//  int var_num = vars_.size();
//  ml_.MLPutFunction("List", var_num);
//  std::map<std::string, int>::iterator vars_it = vars_.begin();
//  const char* sym;
//  std::cout << "vars: ";
//  while(vars_it!=vars_.end())
//  {
//    sym = ((*vars_it).first).c_str();
//    if((*vars_it).second==0)
//    {
//      ml_.MLPutSymbol(sym);
//    }
//    else
//    {
//      ml_.MLPutNext(MLTKFUNC);   // The func we are putting has head Derivative[*number*], arg f
//      ml_.MLPutArgCount(1);      // this 1 is for the 'f'
//      ml_.MLPutNext(MLTKFUNC);   // The func we are putting has head Derivative, arg 2
//      ml_.MLPutArgCount(1);      // this 1 is for the '*number*'
//      ml_.MLPutSymbol("Derivative");
//      ml_.MLPutInteger(1);
//      ml_.MLPutSymbol(sym);
//      //ml_.MLPutSymbol("t");
//    }
//    std::cout << sym << " ";
//    vars_it++;
//  }
//
//  // ml_.MLEndPacket();
//
//  // �v�f�̑S�폜
//  vars_.clear();
//
//  std::cout << std::endl;
//
///*
//// �Ԃ��Ă���p�P�b�g�����
//PacketChecker pc(ml_);
//pc.check();
//*/
//
//  ml_.skip_pkt_until(RETURNPKT);
//  
//  int num;
//  ml_.MLGetInteger(&num);
//  //std::cout << "#num:" << num << std::endl;
//  
//  // Mathematica����1�iTrue��\���j���Ԃ��true���A0�iFalse��\���j���Ԃ��false��Ԃ�
//  return num==1;
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

} //namespace bp_simulator
} // namespace hydla

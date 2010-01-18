#include "PacketSender.h"

#include <iostream>
#include <cassert>

#include "Logger.h"

using namespace hydla::parse_tree;

namespace hydla {
namespace vcs {
namespace mathematica {

/** Mathematica�ɑ���ۂɕϐ����ɂ���ړ��� "usrVar" */
const std::string PacketSender::var_prefix("usrVar");

/**
 * ��(�m�[�h)��Mathematica�֑��M����N���X�D
 * @param ml Mathlink�C���X�^���X�̎Q��
 * @param phase {NP_POINT_PHASE | NP_INTERVAL_PHASE} �g�p����ۂ̃t�F�[�Y
 */
PacketSender::PacketSender(MathLink& ml, now_phase_t phase) :
  ml_(ml),
  phase_(phase),
  differential_count_(0),
  in_prev_(false)
{}

PacketSender::~PacketSender(){}


// Ask����
void PacketSender::visit(boost::shared_ptr<Ask> node)                   
{
  // ask����͑���Ȃ�
  assert(0);
}

// Tell����
void PacketSender::visit(boost::shared_ptr<Tell> node)                  
{
  // tell����͑���Ȃ�
  assert(0);
}

// ��r���Z�q
void PacketSender::visit(boost::shared_ptr<Equal> node)                 
{
  HYDLA_LOGGER_DEBUG("put: Equal");
  ml_.put_function("Equal", 2);

  accept(node->get_lhs());
  accept(node->get_rhs());
}

void PacketSender::visit(boost::shared_ptr<UnEqual> node)               
{
  HYDLA_LOGGER_DEBUG("put: UnEqual");
  ml_.put_function("UnEqual", 2);

  accept(node->get_lhs());
  accept(node->get_rhs());
}

void PacketSender::visit(boost::shared_ptr<Less> node)                  
{
  HYDLA_LOGGER_DEBUG("put: Less");
  ml_.put_function("Less", 2);

  accept(node->get_lhs());
  accept(node->get_rhs());
}

void PacketSender::visit(boost::shared_ptr<LessEqual> node)             
{
  HYDLA_LOGGER_DEBUG("put: LessEqual");
  ml_.put_function("LessEqual", 2);

  accept(node->get_lhs());    
  accept(node->get_rhs());
}

void PacketSender::visit(boost::shared_ptr<Greater> node)               
{
  HYDLA_LOGGER_DEBUG("put: Greater");
  ml_.put_function("Greater", 2);

  accept(node->get_lhs());
  accept(node->get_rhs());
}

void PacketSender::visit(boost::shared_ptr<GreaterEqual> node)          
{
  HYDLA_LOGGER_DEBUG("put: GreaterEqual");
  ml_.put_function("GreaterEqual", 2);

  accept(node->get_lhs());
  accept(node->get_rhs());
}

// �_�����Z�q
void PacketSender::visit(boost::shared_ptr<LogicalAnd> node)            
{
  HYDLA_LOGGER_DEBUG("put: And");
  ml_.put_function("And", 2);

  accept(node->get_lhs());
  accept(node->get_rhs());
}

void PacketSender::visit(boost::shared_ptr<LogicalOr> node)             
{
  HYDLA_LOGGER_DEBUG("put: Or");
  ml_.put_function("Or", 2);

  accept(node->get_lhs());
  accept(node->get_rhs());
}
  
// �Z�p�񍀉��Z�q
void PacketSender::visit(boost::shared_ptr<Plus> node)                  
{
  HYDLA_LOGGER_DEBUG("put: Plus");
  ml_.put_function("Plus", 2);

  accept(node->get_lhs());
  accept(node->get_rhs());
}

void PacketSender::visit(boost::shared_ptr<Subtract> node)              
{
  HYDLA_LOGGER_DEBUG("put: Subtract");
  ml_.put_function("Subtract", 2);

  accept(node->get_lhs());
  accept(node->get_rhs());
}

void PacketSender::visit(boost::shared_ptr<Times> node)                 
{
  HYDLA_LOGGER_DEBUG("put: Times");
  ml_.put_function("Times", 2);

  accept(node->get_lhs());
  accept(node->get_rhs());
}

void PacketSender::visit(boost::shared_ptr<Divide> node)                
{
  HYDLA_LOGGER_DEBUG("put: Divide");
  ml_.put_function("Divide", 2);

  accept(node->get_lhs());
  accept(node->get_rhs());
}
  
// �Z�p�P�����Z�q
void PacketSender::visit(boost::shared_ptr<Negative> node)              
{
  HYDLA_LOGGER_DEBUG("put: Minus");
  ml_.put_function("Minus", 1);

  accept(node->get_child());
}

void PacketSender::visit(boost::shared_ptr<Positive> node)              
{
  accept(node->get_child());
}

// ����
void PacketSender::visit(boost::shared_ptr<Differential> node)          
{
  differential_count_++;
  accept(node->get_child());
  differential_count_--;
}

// ���Ɍ�
void PacketSender::visit(boost::shared_ptr<Previous> node)              
{
  in_prev_ = true;
  accept(node->get_child());
  in_prev_ = false;
}
  
// �ϐ�
void PacketSender::visit(boost::shared_ptr<Variable> node)              
{
  // �ϐ��̑��M
  var_info_t new_var = 
    boost::make_tuple(node->get_name(), differential_count_, in_prev_);

  if(phase_==NP_INTERVAL_PHASE) {
    // �ϐ����̍Ō��[t]������
    ml_.MLPutNext(MLTKFUNC);
    ml_.MLPutArgCount(1);
    put_var(new_var);
    ml_.put_symbol("t");
  }
  else {
    put_var(new_var);
  }

  // put�����ϐ��̏���ێ�
  vars_.insert(new_var);
}

// ����
void PacketSender::visit(boost::shared_ptr<Number> node)                
{    
  HYDLA_LOGGER_DEBUG("put: Number : ", node->get_number());
  ml_.MLPutInteger(atoi(node->get_number().c_str()));
}

void PacketSender::put_var(const var_info_t var)
{
  std::string name(PacketSender::var_prefix + var.get<0>());
  int diff_count = var.get<1>();
  bool prev      = var.get<2>();
  
  HYDLA_LOGGER_DEBUG(
    "PacketSender::put_var: ",
    "name: ", name,
    "  diff_count: ", diff_count,
    "  prev: ", prev);
  
//   // �ϐ����̍Ō�ɕK��[t]������
//   if(phase_==NP_INTERVAL_PHASE) {
//     ml_.MLPutNext(MLTKFUNC);
//     ml_.MLPutArgCount(1);
//   }

  // �ϐ���put
  if(diff_count > 0){
    // �����ϐ��Ȃ� (Derivative[��])[�ϐ���]��put
    ml_.MLPutNext(MLTKFUNC);   // The func we are putting has head Derivative[*number*], arg f
    ml_.MLPutArgCount(1);      // this 1 is for the 'f'
    ml_.MLPutNext(MLTKFUNC);   // The func we are putting has head Derivative, arg 2
    ml_.MLPutArgCount(1);      // this 1 is for the '*number*'
    ml_.put_symbol("Derivative");
    ml_.MLPutInteger(diff_count);
  }
   
  // prev�ϐ��Ƃ��đ��邩�ǂ���
  if(prev && phase_==NP_POINT_PHASE) {
    ml_.put_function("prev", 1);
    ml_.put_symbol(name);
  }
  else {
    ml_.put_symbol(name);
  }

//   // [t]�̕�
//   if(this->phase_==NP_INTERVAL_PHASE) {
//     ml_.put_symbol("t");
//   }
}


/**
 * ���鎮(�m�[�h)��put����
 * @param node put��������(�m�[�h)
 */
void PacketSender::put_node(const node_sptr& node)
{
  differential_count_ = 0;
  in_prev_ = false;

  accept(node);
}

/**
 * �ϐ��̈ꗗ�𑗐M
 */
void PacketSender::put_vars()
{
  HYDLA_LOGGER_DEBUG(
    "---- PacketSender::put_vars ----\n",
    "var size:", vars_.size());
  
  ml_.put_function("List", vars_.size());

  PacketSender::vars_const_iterator it  = vars_begin();
  PacketSender::vars_const_iterator end = vars_end();
  if(phase_==NP_INTERVAL_PHASE) {
    // �ϐ����̍Ō��[t]������
    for(; it!=end; ++it) {
      ml_.MLPutNext(MLTKFUNC);
      ml_.MLPutArgCount(1);
      put_var(*it);
      ml_.put_symbol("t");
    }
  }
  else {
    for(; it!=end; ++it) {
      put_var(*it);
    }
  }
}

/**
 * �������(���ɕϐ����)�����Z�b�g����D
 * ����put����蒼�������Ƃ��Ȃǂ�
 */
void PacketSender::clear()
{
  differential_count_ = 0;
  in_prev_ = false;

  vars_.clear();
}

} // namespace mathematica
} // namespace simulator
} // namespace hydla 

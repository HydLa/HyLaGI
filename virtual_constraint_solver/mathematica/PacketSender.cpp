#include "PacketSender.h"
#include <iostream>
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
  // �K�[�h�����̂�put����
  debug_string_ += "guard:";
  accept(node->get_guard());    
  HYDLA_LOGGER_DEBUG(debug_string_);
  debug_string_.erase();
}

// Tell����
void PacketSender::visit(boost::shared_ptr<Tell> node)                  
{
  accept(node->get_child());
  HYDLA_LOGGER_DEBUG(debug_string_);
  debug_string_.erase();
}

// ��r���Z�q
void PacketSender::visit(boost::shared_ptr<Equal> node)                 
{
  ml_.put_function("Equal", 2);

  accept(node->get_lhs());
  debug_string_ += "=";
  accept(node->get_rhs());
}

void PacketSender::visit(boost::shared_ptr<UnEqual> node)               
{
  ml_.put_function("UnEqual", 2);

  accept(node->get_lhs());
  debug_string_ += "!=";
  accept(node->get_rhs());
}

void PacketSender::visit(boost::shared_ptr<Less> node)                  
{
  ml_.put_function("Less", 2);

  accept(node->get_lhs());
  debug_string_ += "<";
  accept(node->get_rhs());
}

void PacketSender::visit(boost::shared_ptr<LessEqual> node)             
{
  ml_.put_function("LessEqual", 2);

  accept(node->get_lhs());    
  debug_string_ += "<=";
  accept(node->get_rhs());
}

void PacketSender::visit(boost::shared_ptr<Greater> node)               
{
  ml_.put_function("Greater", 2);

  accept(node->get_lhs());
  debug_string_ += ">";
  accept(node->get_rhs());
}

void PacketSender::visit(boost::shared_ptr<GreaterEqual> node)          
{
  ml_.put_function("GreaterEqual", 2);

  accept(node->get_lhs());
  debug_string_ += ">=";
  accept(node->get_rhs());
}

// �_�����Z�q
void PacketSender::visit(boost::shared_ptr<LogicalAnd> node)            
{
  ml_.put_function("And", 2);

  accept(node->get_lhs());
  debug_string_ += " & ";
  accept(node->get_rhs());
}

void PacketSender::visit(boost::shared_ptr<LogicalOr> node)             
{
  //ml_.put_function("Or", 2);

  accept(node->get_lhs());
  accept(node->get_rhs());
}
  
// �Z�p�񍀉��Z�q
void PacketSender::visit(boost::shared_ptr<Plus> node)                  
{
  ml_.put_function("Plus", 2);

  accept(node->get_lhs());
  debug_string_ += "+";
  accept(node->get_rhs());
}

void PacketSender::visit(boost::shared_ptr<Subtract> node)              
{
  ml_.put_function("Subtract", 2);

  accept(node->get_lhs());
  debug_string_ += "-";
  accept(node->get_rhs());
}

void PacketSender::visit(boost::shared_ptr<Times> node)                 
{
  ml_.put_function("Times", 2);

  accept(node->get_lhs());
  debug_string_ += "*";
  accept(node->get_rhs());
}

void PacketSender::visit(boost::shared_ptr<Divide> node)                
{
  ml_.put_function("Divide", 2);

  accept(node->get_lhs());
  debug_string_ += "/";
  accept(node->get_rhs());
}
  
// �Z�p�P�����Z�q
void PacketSender::visit(boost::shared_ptr<Negative> node)              
{
  ml_.put_function("Minus", 1);

  debug_string_ += "-";
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
  if(this->phase_==NP_POINT_PHASE) {
    ml_.put_function("prev", 1);
    this->debug_string_ += "prev[";
    accept(node->get_child());
    this->debug_string_ += "]";
  } else {
    accept(node->get_child());
  }
  in_prev_ = false;
}
  
// �ϐ�
void PacketSender::visit(boost::shared_ptr<Variable> node)              
{
  if(this->phase_==NP_INTERVAL_PHASE) {
    // �ϐ����̍Ō�ɕK��[t]������
    ml_.MLPutNext(MLTKFUNC);
    ml_.MLPutArgCount(1);
  }

  // �ϐ���put
  if(differential_count_ > 0){
    // �����ϐ��Ȃ� (Derivative[��])[�ϐ���]��put
    ml_.MLPutNext(MLTKFUNC);   // The func we are putting has head Derivative[*number*], arg f
    ml_.MLPutArgCount(1);      // this 1 is for the 'f'
    ml_.MLPutNext(MLTKFUNC);   // The func we are putting has head Derivative, arg 2
    ml_.MLPutArgCount(1);      // this 1 is for the '*number*'
    ml_.put_symbol("Derivative");
    ml_.MLPutInteger(differential_count_);
    ml_.put_symbol((PacketSender::var_prefix + node->get_name()).c_str());
  }else{
    ml_.put_symbol((PacketSender::var_prefix + node->get_name()).c_str());
  }
  if(this->phase_==NP_INTERVAL_PHASE) ml_.put_symbol("t");

  // put�����ϐ��̏���ێ�
  if(in_prev_){
    vars_.insert(std::make_pair(PacketSender::var_prefix + node->get_name(),
      -1*(differential_count_ +1)));
  }else{
    vars_.insert(std::make_pair(PacketSender::var_prefix + node->get_name(),
      differential_count_ + 1));
  }

  // �f�o�b�O������̐���
  if(differential_count_ > 0){
    this->debug_string_ += "Derivative[";
    this->debug_string_ += differential_count_ ;
    this->debug_string_ += "][";
    this->debug_string_ += (node->get_name() + "]");
  }else{
    this->debug_string_ += node->get_name();
  }
  if(this->phase_==NP_INTERVAL_PHASE) this->debug_string_ += "[t]";
}

// ����
void PacketSender::visit(boost::shared_ptr<Number> node)                
{    
  ml_.MLPutInteger(atoi(node->get_number().c_str()));
  this->debug_string_ += node->get_number();
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
  ml_.put_function("List", vars_.size());

  std::string debug_str("vars: ");

  PacketSender::vars_const_iterator it;
  for(it=this->vars_begin(); it!=this->vars_end(); it++) {
    const std::string name(PacketSender::get_var_name(*it));
    const int d_count = PacketSender::get_var_differential_count(*it);
    const bool is_prev = PacketSender::is_var_prev(*it);

    // �ϐ���prev && PointPhase -> prev�֐���put
    if(is_prev && this->phase_==NP_POINT_PHASE) {
      ml_.put_function("prev", 1);
      debug_str += "prev[";
    }

    // Interval Phase�Ȃ�֐��ϐ��Ƃ��Ă�[t]��put���鏀�����K�v
    if(this->phase_==NP_INTERVAL_PHASE) {
      ml_.MLPutNext(MLTKFUNC);
      ml_.MLPutArgCount(1);
    }

    // �����ϐ��Ȃ� (Derivative[��])[�ϐ���]��put
    if(d_count > 0) {
      ml_.MLPutNext(MLTKFUNC);   // The func we are putting has head Derivative[*number*], arg f
      ml_.MLPutArgCount(1);      // this 1 is for the 'f'
      ml_.MLPutNext(MLTKFUNC);   // The func we are putting has head Derivative, arg 2
      ml_.MLPutArgCount(1);      // this 1 is for the '*number*'
      ml_.put_symbol("Derivative");
      ml_.MLPutInteger(d_count);
      ml_.put_symbol(name);
      debug_str += "Derivative[";
      debug_str += d_count;
      debug_str += "][" + name + "]";
    } else {
      // ����ɕϐ�����put
      ml_.put_symbol(name);
      debug_str += name;
    }

    // Interval Phase�Ȃ�[t]��put
    if(this->phase_==NP_INTERVAL_PHASE) {
      ml_.put_symbol("t");
      debug_str += "[t]";
    }

    // �ϐ���prev && PointPhase -> prev�֐���put����
    if(is_prev && this->phase_==NP_POINT_PHASE) debug_str += "]";
    debug_str += ", ";

  } // for

  HYDLA_LOGGER_DEBUG(debug_str);
}

/**
 * �������(���ɕϐ����)�����Z�b�g����D
 * ����put����蒼�������Ƃ��Ȃǂ�
 */
void PacketSender::clear()
{
  vars_.clear();
  vars_str_.erase();
  differential_count_ = 0;
  in_prev_ = false;
  debug_string_.erase();
}

} // namespace mathematica
} // namespace simulator
} // namespace hydla 

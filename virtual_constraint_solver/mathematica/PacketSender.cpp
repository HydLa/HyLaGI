#include "PacketSender.h"

#include <iostream>
#include <cassert>

#include "Logger.h"

using namespace hydla::parse_tree;
using namespace hydla::logger;

namespace hydla {
namespace vcs {
namespace mathematica {

/** Mathematica�ɑ���ۂɕϐ����ɂ���ړ��� "usrVar" */
const std::string PacketSender::var_prefix("usrVar");

/**
 * ��(�m�[�h)��Mathematica�֑��M����N���X�D
 * @param ml Mathlink�C���X�^���X�̎Q��
 */
PacketSender::PacketSender(MathLink& ml) :
  ml_(ml),
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


void PacketSender::visit(boost::shared_ptr<Power> node)                
{
  HYDLA_LOGGER_DEBUG("put: Power");
  ml_.put_function("Power", 2);

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
    boost::make_tuple(node->get_name(), 
                      differential_count_, 
                      in_prev_ && !ignore_prev_);

  put_var(new_var, variable_arg_);
}

// ����
void PacketSender::visit(boost::shared_ptr<Number> node)                
{    
  HYDLA_LOGGER_DEBUG("put: Number : ", node->get_number());
  ml_.MLPutInteger(atoi(node->get_number().c_str()));
}

void PacketSender::put_var(const var_info_t var, VariableArg variable_arg)
{
  std::string name(PacketSender::var_prefix + var.get<0>());
  int diff_count = var.get<1>();
  bool prev      = var.get<2>();
  
  if(Logger::mathsendflag==1){
	    HYDLA_LOGGER_AREA(
    "PacketSender::put_var: ",
    "name: ", name,
    "\tdiff_count: ", diff_count,
    "\tprev: ", prev,
    "\tvariable_arg: ", variable_arg);
  }



  HYDLA_LOGGER_DEBUG(
    "PacketSender::put_var: ",
    "name: ", name,
    "\tdiff_count: ", diff_count,
    "\tprev: ", prev,
    "\tvariable_arg: ", variable_arg);
  
  // �ϐ����̍Ō�ɕK��[t]������
  if(variable_arg != VA_None) {
    ml_.MLPutNext(MLTKFUNC);
    ml_.MLPutArgCount(1);
  }

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
  if(prev) {
    ml_.put_function("prev", 1);
    ml_.put_symbol(name);
  }
  else {
    ml_.put_symbol(name);
  }

  switch(variable_arg) {
    case VA_None:
      // do nothing
      break;
      
    case VA_Time:
      ml_.put_symbol("t");
      break;

    case VA_Zero:
      ml_.put_integer(0);
      break;
      
    default:
      assert(0);
  }

  // put�����ϐ��̏���ێ�
  vars_.insert(var);
}


/**
 * ���鎮(�m�[�h)��put����
 * @param node put��������(�m�[�h)
 */
void PacketSender::put_node(const node_sptr& node, 
                            VariableArg variable_arg, 
                            bool ignore_prev,
                            bool entailed)
{
  differential_count_ = 0;
  in_prev_ = false;
  variable_arg_ = variable_arg;
  ignore_prev_ = ignore_prev;
  if(!entailed){
    HYDLA_LOGGER_DEBUG("put: Not");
    ml_.put_function("Not", 1);
  }
  accept(node);
}

/**
 * �ϐ��̈ꗗ�𑗐M
 */
void PacketSender::put_vars(VariableArg variable_arg, 
                            bool ignore_prev)
{
  if(Logger::mathsendflag==1){
	    HYDLA_LOGGER_AREA(
    "---- PacketSender::put_vars ----\n",
    "var size:", vars_.size());
  }

  HYDLA_LOGGER_DEBUG(
    "---- PacketSender::put_vars ----\n",
    "var size:", vars_.size());
  
  ml_.put_function("List", vars_.size());

  PacketSender::vars_const_iterator it  = vars_begin();
  PacketSender::vars_const_iterator end = vars_end();
  for(; it!=end; ++it) {
    put_var(boost::make_tuple(
              it->get<0>(),
              it->get<1>(),
              it->get<2>() && !ignore_prev), 
            variable_arg);
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

namespace {

struct MaxDiffMapDumper
{
  template<typename T>
  MaxDiffMapDumper(T it, T end)
  {
    for(; it!=end; ++it) {
      s << "name: " << it->first
        << "diff: " << it->second
        << "\n";      
    }
  }

  std::stringstream s;
};

}

void PacketSender::create_max_diff_map(max_diff_map_t& max_diff_map)
{
  vars_const_iterator vars_it  = vars_begin();
  vars_const_iterator vars_end_it = vars_end();

  for(; vars_it!=vars_end_it; ++vars_it) {
    std::string name(vars_it->get<0>());
    int derivative_count = vars_it->get<1>();

    max_diff_map_t::iterator it = max_diff_map.find(name);
    if(it==max_diff_map.end()) {
      max_diff_map.insert(
        std::make_pair(name, derivative_count));
    }
    else if(it->second < derivative_count) {
      it->second = derivative_count;
    }    
  }

  HYDLA_LOGGER_DEBUG(
    "-- max diff map --\n",
    MaxDiffMapDumper(max_diff_map.begin(), 
                     max_diff_map.end()).s.str());

}

} // namespace mathematica
} // namespace simulator
} // namespace hydla 

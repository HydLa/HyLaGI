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

/** Mathematica�ɑ���ۂɒ萔���ɂ���ړ��� */
const std::string PacketSender::par_prefix("p");

/**
 * ��(�m�[�h)��Mathematica�֑��M����N���X�D
 * @param ml Mathlink�C���X�^���X�̎Q��
 */
PacketSender::PacketSender(MathLink& ml) :
  ml_(&ml),
  differential_count_(0),
  in_prev_(false)
{}

PacketSender::PacketSender() :
  ml_(NULL),
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

#define DEFINE_VISIT_BINARY(NODE_NAME, FUNC_NAME)                       \
void PacketSender::visit(boost::shared_ptr<NODE_NAME> node)             \
{                                                                       \
  HYDLA_LOGGER_REST("put:" #NODE_NAME);                                 \
  ml_->put_function(#FUNC_NAME, 2);                                      \
  accept(node->get_lhs());                                              \
  accept(node->get_rhs());                                              \
}

#define DEFINE_VISIT_UNARY(NODE_NAME, FUNC_NAME)                        \
void PacketSender::visit(boost::shared_ptr<NODE_NAME> node)             \
{                                                                       \
  HYDLA_LOGGER_REST("put:" #NODE_NAME);                                 \
  ml_->put_function(#FUNC_NAME, 1);                                      \
  accept(node->get_child());                                            \
}

#define DEFINE_VISIT_FACTOR(NODE_NAME, FUNC_NAME)                       \
void PacketSender::visit(boost::shared_ptr<NODE_NAME> node)             \
{                                                                       \
  HYDLA_LOGGER_REST("put:" #NODE_NAME);                                 \
  ml_->put_symbol(#FUNC_NAME);                                           \
}

DEFINE_VISIT_BINARY(Equal, Equal)
DEFINE_VISIT_BINARY(UnEqual, Unequal)
DEFINE_VISIT_BINARY(Less, Less)
DEFINE_VISIT_BINARY(LessEqual, LessEqual)
DEFINE_VISIT_BINARY(Greater, Greater)
DEFINE_VISIT_BINARY(GreaterEqual, GreaterEqual)



// �_�����Z�q
DEFINE_VISIT_BINARY(LogicalAnd, And)
DEFINE_VISIT_BINARY(LogicalOr, Or)

  
// �Z�p�񍀉��Z�q
DEFINE_VISIT_BINARY(Plus, Plus)
DEFINE_VISIT_BINARY(Subtract, Subtract)
DEFINE_VISIT_BINARY(Times, Times)
DEFINE_VISIT_BINARY(Divide, Divide)
DEFINE_VISIT_BINARY(Power, Power)

  
// �Z�p�P�����Z�q

DEFINE_VISIT_UNARY(Negative, Minus)
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


// �ے�
DEFINE_VISIT_UNARY(Not, Not)


// �O�p�֐�
DEFINE_VISIT_UNARY(Sin, Sin)
DEFINE_VISIT_UNARY(Cos, Cos)
DEFINE_VISIT_UNARY(Tan, Tan)
// �t�O�p�֐�
DEFINE_VISIT_UNARY(Asin, ArcSin)
DEFINE_VISIT_UNARY(Acos, ArcCos)
DEFINE_VISIT_UNARY(Atan, ArcTan)
// �~����
DEFINE_VISIT_FACTOR(Pi, Pi)
// �ΐ�
DEFINE_VISIT_BINARY(Log, Log)
DEFINE_VISIT_UNARY(Ln, Log)
// ���R�ΐ��̒�
DEFINE_VISIT_FACTOR(E, E)

// Print
void PacketSender::visit(boost::shared_ptr<Print> node)
{    
 //do nothing 
  ml_->put_symbol("True");
}
void PacketSender::visit(boost::shared_ptr<PrintPP> node)
{    
 //do nothing 
  ml_->put_symbol("True");
}
void PacketSender::visit(boost::shared_ptr<PrintIP> node)
{    
 //do nothing 
  ml_->put_symbol("True");
}
    
void PacketSender::visit(boost::shared_ptr<Scan> node)
{    
 //do nothing 
  ml_->put_symbol("True");
}
void PacketSender::visit(boost::shared_ptr<Exit> node)
{    
 //do nothing 
  ml_->put_symbol("True");
}
void PacketSender::visit(boost::shared_ptr<Abort> node)
{    
 //do nothing 
  ml_->put_symbol("True");
}
//�C�ӂ̕�����

void PacketSender::visit(boost::shared_ptr<ArbitraryBinary> node)
{    
  HYDLA_LOGGER_REST("put: ArbitraryFactor : ", node->get_string());
  ml_->put_function(node->get_string(),2);
  accept(node->get_lhs());
  accept(node->get_rhs());
}

void PacketSender::visit(boost::shared_ptr<ArbitraryUnary> node)
{    
  HYDLA_LOGGER_REST("put: ArbitraryUnary : ", node->get_string());
  ml_->put_function(node->get_string(),1);
  accept(node->get_child());
}

void PacketSender::visit(boost::shared_ptr<ArbitraryFactor> node)
{    
  HYDLA_LOGGER_REST("put: ArbitraryFactor : ", node->get_string());
  ml_->put_symbol(node->get_string());
}


void PacketSender::visit(boost::shared_ptr<ArbitraryNode> node)
{
  HYDLA_LOGGER_REST("put: ArbitraryNode : ", node->get_string());
  if(node->arguments_.size() == 0){
    ml_->put_symbol(node->get_string());
  }else{
    if(node->get_string() == "Function"){ //Function��HoldAll�Ȋ֐��̂��߁CEvaluate���Ȃ���Number��ToExpression�������Ȃ�
      ml_->put_function(node->get_string(), 1);
      ml_->put_function("Evaluate", node->arguments_.size());
    }else{
      ml_->put_function(node->get_string(), node->arguments_.size());
    }
    for(int i=0; i < node->arguments_.size();i++){
      accept(node->arguments_[i]);
    }
  }
}


// �ϐ�
void PacketSender::visit(boost::shared_ptr<Variable> node)              
{
  // �ϐ��̑��M
  var_info_t new_var = 
    boost::make_tuple(node->get_name(), 
                      differential_count_, 
                      (in_prev_ && variable_arg_ != VA_Time) || variable_arg_ == VA_Prev);

    put_var(new_var, variable_arg_);
}

// ����
void PacketSender::visit(boost::shared_ptr<Number> node)                
{
  HYDLA_LOGGER_REST("put: Number : ", node->get_number());
  // ml_->MLPutInteger(atoi(node->get_number().c_str())); //���l���ł����ƃI�[�o�[�t���[����

  ml_->put_function("ToExpression", 1);

  ml_->put_string(node->get_number());
}


// �L���萔
void PacketSender::visit(boost::shared_ptr<Parameter> node)
{    
  HYDLA_LOGGER_REST("put: Parameter : ", node->get_name());
  put_par(par_prefix + node->get_name());
}

// t
void PacketSender::visit(boost::shared_ptr<SymbolicT> node)                
{    
  HYDLA_LOGGER_REST("put: t");
  ml_->put_symbol("t");
}

void PacketSender::put_var(const var_info_t var, VariableArg variable_arg)
{
  std::string name(PacketSender::var_prefix + var.get<0>());
  int diff_count = var.get<1>();
  bool prev      = var.get<2>();
  
  HYDLA_LOGGER_REST(
    "PacketSender::put_var: ",
    "name: ", name,
    "\tdiff_count: ", diff_count,
    "\tprev: ", prev,
    "\tvariable_arg: ", variable_arg);

  // prev�ϐ��Ƃ��đ��邩�ǂ���
  if(prev) {
    ml_->put_function("prev", 2);
    ml_->put_symbol(name);
    ml_->MLPutInteger(diff_count);
  }else{
  
    // �ϐ�����[t]��[0]������
    if(variable_arg == VA_Time ||  variable_arg == VA_Zero) {
      ml_->MLPutNext(MLTKFUNC);
      ml_->MLPutArgCount(1);
    }

    // �ϐ���put
    if(diff_count > 0){
      // �����ϐ��Ȃ� (Derivative[��])[�ϐ���]��put
      ml_->MLPutNext(MLTKFUNC);   // The func we are putting has head Derivative[*number*], arg f
      ml_->MLPutArgCount(1);      // this 1 is for the 'f'
      ml_->MLPutNext(MLTKFUNC);   // The func we are putting has head Derivative, arg 2
      ml_->MLPutArgCount(1);      // this 1 is for the '*number*'
      ml_->put_symbol("Derivative");
      ml_->MLPutInteger(diff_count);
    }
    ml_->put_symbol(name);
  
    switch(variable_arg) {
      case VA_None:
      case VA_Prev:
        break;
        
      case VA_Time:
        ml_->put_symbol("t");
        break;
      case VA_Zero:
        ml_->put_integer(0);
        break;
      default:
        assert(0);
    }
  }

  // put�����ϐ��̏���ێ�
  vars_.insert(var);
  
  HYDLA_LOGGER_REST(
    "PacketSender::put_var: vars_size()",
    vars_.size());
}


void PacketSender::put_par(const std::string &name)
{
  HYDLA_LOGGER_REST(
    "PacketSender::put_par: ",
    "name: ", name);

  ml_->put_symbol(name);


  // put�����ϐ��̏���ێ�
  pars_.insert(name);
}


/**
 * ���鎮(�m�[�h)��put����
 * @param node put��������(�m�[�h)
 */
void PacketSender::put_node(const node_sptr& node, 
                            VariableArg variable_arg)
{
  HYDLA_LOGGER_REST("*** Begin PacketSender::put_node ***");
  HYDLA_LOGGER_REST("node:", *node);
  differential_count_ = 0;
  in_prev_ = false;
  variable_arg_ = variable_arg;
  accept(node);
  HYDLA_LOGGER_REST("*** END PacketSender::put_node ***");
}


/**
 * ���鎮�̃��X�g��put����
 */
void PacketSender::put_nodes(const std::vector<node_sptr>& constraints,
                            VariableArg variable_arg)
{
  
  HYDLA_LOGGER_REST("*** Begin PacketSender::put_nodes ***");
  ml_->put_function("And", constraints.size());
  for(std::vector<node_sptr>::const_iterator it = constraints.begin(); it != constraints.end(); it++){
    put_node(*it, variable_arg);
  }
  HYDLA_LOGGER_REST("*** End PacketSender::put_nodes: ***");
}

/**
 * �ϐ��̈ꗗ�𑗐M�D
 */
void PacketSender::put_vars(VariableArg variable_arg)
{
  HYDLA_LOGGER_REST("*** Begin PacketSender::put_vars ***",
    "var size:", vars_.size());
  
  ml_->put_function("List", vars_.size());

  PacketSender::vars_const_iterator it  = vars_begin();
  PacketSender::vars_const_iterator end = vars_end();
  for(; it!=end; ++it) {
    put_var(*it, variable_arg);
  }
  HYDLA_LOGGER_REST("*** End PacketSender::put_vars: ***");
}


void PacketSender::put_pars()
{
  HYDLA_LOGGER_REST(
    "---- PacketSender::put_pars ----\n",
    "par size:", pars_.size());
  
  ml_->put_function("List", pars_.size());

  for(std::set<std::string>::iterator it = pars_.begin(); it!=pars_.end(); ++it) {
    put_par(*it);
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
  pars_.clear();
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

  HYDLA_LOGGER_REST(
    "-- max diff map --\n",
    MaxDiffMapDumper(max_diff_map.begin(), 
                     max_diff_map.end()).s.str());
}

} // namespace mathematica
} // namespace simulator
} // namespace hydla 

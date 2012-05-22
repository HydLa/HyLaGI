#include "PacketSender.h"

#include <iostream>
#include <cassert>

#include "Logger.h"
#include "TreeInfixPrinter.h"

using namespace hydla::parse_tree;
using namespace hydla::logger;

namespace hydla {
namespace vcs {
namespace mathematica {

/** Mathematica�ɑ���ۂɕϐ����ɂ���ړ��� "usrVar" */
const std::string PacketSender::var_prefix("usrVar");

/** Mathematica�ɑ���ۂɒ萔���ɂ���֐��� */
const std::string PacketSender::par_prefix("parameter");

PacketSender::function_map_t PacketSender::function_map_;


void PacketSender::initialize(){
  //HydLa��Mathematica�̊֐����̑Ή��֌W������Ă����D
  typedef function_map_t::value_type value_t;
  function_map_.insert(value_t(function_t("Sin", 1), function_t("Sin", 1)));
  function_map_.insert(value_t(function_t("Sinh", 1), function_t("Sinh", 1)));
  function_map_.insert(value_t(function_t("Asin", 1), function_t("ArcSin", 1)));
  function_map_.insert(value_t(function_t("Asinh", 1), function_t("ArcSinh", 1)));
  function_map_.insert(value_t(function_t("Cos", 1), function_t("Cos", 1)));
  function_map_.insert(value_t(function_t("Cosh", 1), function_t("Cosh", 1)));
  function_map_.insert(value_t(function_t("Acos", 1), function_t("ArcCos", 1)));
  function_map_.insert(value_t(function_t("Acosh", 1), function_t("ArcCosh", 1)));
  function_map_.insert(value_t(function_t("Tan", 1), function_t("Tan", 1)));
  function_map_.insert(value_t(function_t("Tanh", 1), function_t("Tanh", 1)));
  function_map_.insert(value_t(function_t("Atan", 1), function_t("Arctan", 1)));
  function_map_.insert(value_t(function_t("Atanh", 1), function_t("ArcTanh", 1)));
  function_map_.insert(value_t(function_t("Log", 2), function_t("Log", 2)));
  function_map_.insert(value_t(function_t("Ln", 1), function_t("Log", 1)));
}


/**
 * ��(�m�[�h)��Mathematica�֑��M����N���X�D
 * @param ml Mathlink�C���X�^���X�̎Q��
 */
PacketSender::PacketSender(MathLink& ml) :
  ml_(&ml),
  differential_count_(0),
  in_prev_(false)
{}

PacketSender::~PacketSender(){}


/// Ask����
void PacketSender::visit(boost::shared_ptr<Ask> node)                   
{
  // ask����͑���Ȃ�
  assert(0);
}

/// Tell����
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



/// �_�����Z�q
DEFINE_VISIT_BINARY(LogicalAnd, And)
DEFINE_VISIT_BINARY(LogicalOr, Or)

  
/// �Z�p�񍀉��Z�q
DEFINE_VISIT_BINARY(Plus, Plus)
DEFINE_VISIT_BINARY(Subtract, Subtract)
DEFINE_VISIT_BINARY(Times, Times)
DEFINE_VISIT_BINARY(Divide, Divide)
DEFINE_VISIT_BINARY(Power, Power)

  
/// �Z�p�P�����Z�q

DEFINE_VISIT_UNARY(Negative, Minus)
void PacketSender::visit(boost::shared_ptr<Positive> node)              
{
  accept(node->get_child());
}

/// ����
void PacketSender::visit(boost::shared_ptr<Differential> node)          
{
  differential_count_++;
  accept(node->get_child());
  differential_count_--;
}

/// ���Ɍ�
void PacketSender::visit(boost::shared_ptr<Previous> node)              
{
  in_prev_ = true;
  accept(node->get_child());
  in_prev_ = false;
}


/// �ے�
DEFINE_VISIT_UNARY(Not, Not)


/// �֐�
void PacketSender::visit(boost::shared_ptr<Function> node)              
{
  int size = node->get_arguments_size();
  function_map_t::left_iterator it = function_map_.left.find(function_t(node->get_string(), size));
  assert(it != function_map_.left.end());
  assert(it->second.second == size);
  HYDLA_LOGGER_REST("put: Function : ", it->second.first);
  ml_->put_function(it->second.first, size);
  for(int i=0; i<size;i++){
    accept(node->get_argument(i));
  }
}


void PacketSender::visit(boost::shared_ptr<UnsupportedFunction> node)              
{
  ml_->put_function(node->get_string(), node->get_arguments_size());
  for(int i=0; i<node->get_arguments_size();i++){
    accept(node->get_argument(i));
  }
}

/// �~����
DEFINE_VISIT_FACTOR(Pi, Pi)
/// ���R�ΐ��̒�
DEFINE_VISIT_FACTOR(E, E)



// �ϐ�
void PacketSender::visit(boost::shared_ptr<Variable> node)              
{
  // �ϐ��̑��M
  VariableArg va;
  if(variable_arg_==VA_None&&in_prev_)
    va = VA_Prev;
  else{
    va = variable_arg_;
  }
  var_info_t new_var = 
    boost::make_tuple(node->get_name(), differential_count_, va);

  put_var(new_var);
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
  put_par(node->get_name(), node->get_derivative_count(), node->get_phase_id());
}

// t
void PacketSender::visit(boost::shared_ptr<SymbolicT> node)                
{    
  HYDLA_LOGGER_REST("put: t");
  ml_->put_symbol("t");
}

void PacketSender::put_var(const var_info_t var)
{
  HYDLA_LOGGER_REST("#*** Begin PacketSender::put_var ***");
  std::string name = var.get<0>();;
  int diff_count = var.get<1>();
  VariableArg variable_arg = var.get<2>();

  HYDLA_LOGGER_REST(
    "PacketSender::put_var: ",
    "name: ", name,
    "\tdiff_count: ", diff_count,
    "\tvariable_arg: ", variable_arg);

  if(variable_arg == VA_Prev){
    ml_->put_function("prev", 2);
    ml_->put_symbol(name);
    ml_->MLPutInteger(diff_count);
  }else{
    name = PacketSender::var_prefix + name;
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
  
  HYDLA_LOGGER_REST("#*** End PacketSender::put_var ***");
}


void PacketSender::put_var(const std::string& variable_name, const int &diff_count, VariableArg variable_arg)
{
  put_var(boost::make_tuple(variable_name, diff_count, variable_arg));
}


void PacketSender::put_par(const par_info_t par)
{
  HYDLA_LOGGER_REST("#*** Begin PacketSender::put_var ***");
  std::string name = par.get<0>();;
  int diff_count = par.get<1>(), id = par.get<2>();

  HYDLA_LOGGER_REST(
    "PacketSender::put_par: ",
    "name: ", name,
    "\tdiff_count: ", diff_count,
    "\tid: ", id);
  
  ml_->put_function(par_prefix, 3);
  ml_->put_symbol(name);
  ml_->put_integer(diff_count);
  ml_->put_integer(id);

  // put�����ϐ��̏���ێ�
  pars_.insert(par);
}


void PacketSender::put_par(const std::string &name, const int &diff_count, const int &id)
{
  put_par(boost::make_tuple(name, diff_count, id));
}


/**
 * ���鎮(�m�[�h)��put����
 * @param node put��������(�m�[�h)
 */
void PacketSender::put_node(const node_sptr& node, 
                            VariableArg variable_arg)
{
  HYDLA_LOGGER_REST("#*** Begin PacketSender::put_node ***");
  HYDLA_LOGGER_REST("node:", TreeInfixPrinter().get_infix_string(node));
  differential_count_ = 0;
  in_prev_ = false;
  variable_arg_ = variable_arg;
  accept(node);
  HYDLA_LOGGER_REST("#*** END PacketSender::put_node ***");
}


/**
 * ���鎮�̃��X�g��put����
 */
void PacketSender::put_nodes(const std::vector<node_sptr>& constraints,
                            VariableArg variable_arg)
{
  HYDLA_LOGGER_REST("#*** Begin PacketSender::put_nodes ***");
  ml_->put_function("And", constraints.size());
  for(std::vector<node_sptr>::const_iterator it = constraints.begin(); it != constraints.end(); it++){
    put_node(*it, variable_arg);
  }
  HYDLA_LOGGER_REST("#*** End PacketSender::put_nodes: ***");
}

/**
 * �ϐ��̈ꗗ�𑗐M�D
 */
void PacketSender::put_vars()
{
  HYDLA_LOGGER_REST("#*** Begin PacketSender::put_vars ***",
    "var size:", vars_.size());
  
  ml_->put_function("List", vars_.size());

  PacketSender::vars_const_iterator it  = vars_begin();
  PacketSender::vars_const_iterator end = vars_end();
  for(; it!=end; ++it) {
    put_var(*it);
  }
  HYDLA_LOGGER_REST("#*** End PacketSender::put_vars: ***");
}


void PacketSender::put_pars()
{
  HYDLA_LOGGER_REST("#*** Begin PacketSender::put_pars ***");
  
  ml_->put_function("List", pars_.size());

  for(PacketSender::pars_const_iterator it = pars_.begin(); it!=pars_.end(); ++it) {
    put_par(*it);
  }
  HYDLA_LOGGER_REST("#*** End PacketSender::put_pars ***");
}



  // �R�}���h��
void PacketSender::visit(boost::shared_ptr<hydla::parse_tree::PrintPP> node){ml_->put_symbol("True");}
void PacketSender::visit(boost::shared_ptr<hydla::parse_tree::PrintIP> node){ml_->put_symbol("True");}
void PacketSender::visit(boost::shared_ptr<hydla::parse_tree::Scan> node){ml_->put_symbol("True");}

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

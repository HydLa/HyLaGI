#include "PacketSender.h"

#include <iostream>
#include <cassert>

#include "Logger.h"

using namespace hydla::parse_tree;
using namespace hydla::logger;

namespace hydla {
namespace vcs {
namespace mathematica {

/** Mathematicaに送る際に変数名につける接頭語 "usrVar" */
const std::string PacketSender::var_prefix("usrVar");

/** Mathematicaに送る際に定数名につける接頭語 */
const std::string PacketSender::par_prefix("p");

/**
 * 式(ノード)をMathematicaへ送信するクラス．
 * @param ml Mathlinkインスタンスの参照
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


// Ask制約
void PacketSender::visit(boost::shared_ptr<Ask> node)                   
{
  // ask制約は送れない
  assert(0);
}

void PacketSender::visit(boost::shared_ptr<Constraint> node)                  
{
  //assert(0);
  accept(node->get_child());
}

// Tell制約
void PacketSender::visit(boost::shared_ptr<Tell> node)                  
{
  
  // tell制約は送れない
  //assert(0);
  accept(node->get_child());
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



// 論理演算子
DEFINE_VISIT_BINARY(LogicalAnd, And)
DEFINE_VISIT_BINARY(LogicalOr, Or)

  
// 算術二項演算子
DEFINE_VISIT_BINARY(Plus, Plus)
DEFINE_VISIT_BINARY(Subtract, Subtract)
DEFINE_VISIT_BINARY(Times, Times)
DEFINE_VISIT_BINARY(Divide, Divide)
DEFINE_VISIT_BINARY(Power, Power)

  
// 算術単項演算子

DEFINE_VISIT_UNARY(Negative, Minus)
void PacketSender::visit(boost::shared_ptr<Positive> node)              
{
  accept(node->get_child());
}

// 微分
void PacketSender::visit(boost::shared_ptr<Differential> node)          
{
  differential_count_++;
  accept(node->get_child());
  differential_count_--;
}

// 左極限
void PacketSender::visit(boost::shared_ptr<Previous> node)              
{
  in_prev_ = true;
  accept(node->get_child());
  in_prev_ = false;
}


// 否定
DEFINE_VISIT_UNARY(Not, Not)


// 三角関数
DEFINE_VISIT_UNARY(Sin, Sin)
DEFINE_VISIT_UNARY(Cos, Cos)
DEFINE_VISIT_UNARY(Tan, Tan)
// 逆三角関数
DEFINE_VISIT_UNARY(Asin, ArcSin)
DEFINE_VISIT_UNARY(Acos, ArcCos)
DEFINE_VISIT_UNARY(Atan, ArcTan)
// 円周率
DEFINE_VISIT_FACTOR(Pi, Pi)
// 対数
DEFINE_VISIT_BINARY(Log, Log)
DEFINE_VISIT_UNARY(Ln, Log)
// 自然対数の底
DEFINE_VISIT_FACTOR(E, E)

//任意の文字列

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



// 変数
void PacketSender::visit(boost::shared_ptr<Variable> node)              
{
  // 変数の送信
  var_info_t new_var = 
    boost::make_tuple(node->get_name(), 
                      differential_count_, 
                      in_prev_ && !ignore_prev_);

    put_var(new_var, variable_arg_);
}

// 数字
void PacketSender::visit(boost::shared_ptr<Number> node)                
{
  HYDLA_LOGGER_REST("put: Number : ", node->get_number());
  // ml_->MLPutInteger(atoi(node->get_number().c_str())); //数値がでかいとオーバーフローする

  ml_->put_function("ToExpression", 1);

  ml_->put_string(node->get_number());
}


// 記号定数
void PacketSender::visit(boost::shared_ptr<Parameter> node)
{    
  HYDLA_LOGGER_REST("put: Parameter : ", node->get_name());
  ml_->put_symbol(par_prefix + node->get_name());
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
  
  // 変数名の最後に必ず[t]がつく分
  if(variable_arg != VA_None) {
    ml_->MLPutNext(MLTKFUNC);
    ml_->MLPutArgCount(1);
  }

  // 変数のput
  if(diff_count > 0){
    // 微分変数なら (Derivative[回数])[変数名]をput
    ml_->MLPutNext(MLTKFUNC);   // The func we are putting has head Derivative[*number*], arg f
    ml_->MLPutArgCount(1);      // this 1 is for the 'f'
    ml_->MLPutNext(MLTKFUNC);   // The func we are putting has head Derivative, arg 2
    ml_->MLPutArgCount(1);      // this 1 is for the '*number*'
    ml_->put_symbol("Derivative");
    ml_->MLPutInteger(diff_count);
  }
   
  // prev変数として送るかどうか
  if(prev) {
    ml_->put_function("prev", 1);
    ml_->put_symbol(name);
  }
  else {
    ml_->put_symbol(name);
  }

  switch(variable_arg) {
    case VA_None:
      // do nothing
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

  // putした変数の情報を保持
  vars_.insert(var);
}



/**
 * ある式(ノード)をputする
 * @param node putしたい式(ノード)
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
    HYDLA_LOGGER_REST("put: Not");
    ml_->put_function("Not", 1);
  }
  accept(node);
}

/**
 * 変数の一覧を送信．
 */
void PacketSender::put_vars(VariableArg variable_arg, 
                            bool ignore_prev)
{
  HYDLA_LOGGER_REST(
    "---- PacketSender::put_vars ----\n",
    "var size:", vars_.size());
  
  ml_->put_function("List", vars_.size());

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
 * 内部情報(特に変数情報)をリセットする．
 * 式のputをやり直したいときなどに
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

  HYDLA_LOGGER_REST(
    "-- max diff map --\n",
    MaxDiffMapDumper(max_diff_map.begin(), 
                     max_diff_map.end()).s.str());

}

} // namespace mathematica
} // namespace simulator
} // namespace hydla 

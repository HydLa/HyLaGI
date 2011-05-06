#include "MathematicaExpressionConverter.h"
#include "PacketSender.h"

#include <stdlib.h>
#include <cassert>
#include <iostream>
#include <sstream>

namespace hydla {
namespace vcs {
namespace mathematica {

MathematicaExpressionConverter::string_map_t MathematicaExpressionConverter::string_map_;

using namespace hydla::parse_tree;

//���O�Ƃ��ẮCSymbolicValue��symbolic�CMathematica�̕�����mathe���炢�œ��ꂷ�ׂ������D�ǂ�����Expression�Ȃ킯�����D

void MathematicaExpressionConverter::initialize(){
  //�m�[�h�ƕ�����̑Ή��֌W������Ă����D
  string_map_.insert(std::make_pair("Plus", function_and_node(for_binary_node, NODE_PLUS)));
  string_map_.insert(std::make_pair("Subtract", function_and_node(for_binary_node, NODE_SUBTRACT)));
  string_map_.insert(std::make_pair("Times", function_and_node(for_binary_node, NODE_TIMES)));
  string_map_.insert(std::make_pair("Divide", function_and_node(for_binary_node, NODE_DIVIDE)));
  string_map_.insert(std::make_pair("Power", function_and_node(for_binary_node, NODE_POWER)));
  string_map_.insert(std::make_pair("Rational", function_and_node(for_binary_node, NODE_DIVIDE)));//������ƈӖ��͈قȂ�݂��������ǁC���ʂ͓����ɂȂ�͂��Ȃ̂ł����
  string_map_.insert(std::make_pair("Derivative", function_and_node(for_derivative, NODE_DIFFERENTIAL)));
  string_map_.insert(std::make_pair("prev", function_and_node(for_unary_node, NODE_PREVIOUS)));
  string_map_.insert(std::make_pair("Sqrt", function_and_node(for_unary_node, NODE_SQRT)));
  //string_map_.insert(std::make_pair("Pi", boost::make_shared<Power>()));
  //string_map_.insert(std::make_pair("E", boost::make_shared<Power>()));
}

MathematicaExpressionConverter::value_t MathematicaExpressionConverter::convert_math_string_to_symbolic_value(const std::string &expr){

  HYDLA_LOGGER_DEBUG("#*** convert string to value ***\n",
                     expr);

  value_t value;
  std::string::size_type now = 0;
  value.set(convert_math_string_to_symbolic_tree(expr, now));
  HYDLA_LOGGER_DEBUG("#*** convert result value ***\n",
                     value.get_string());
  return value;
}

MathematicaExpressionConverter::node_sptr MathematicaExpressionConverter::convert_math_string_to_symbolic_tree(const std::string &expr, std::string::size_type &now){
  now = expr.find_first_not_of(" ", now);
  std::string::size_type prev = now;
  if((expr[now]>='0'&&expr[now]<='9')){//���̐��l�̏ꍇ
    now = expr.find_first_of("],", now);
    return node_sptr(new hydla::parse_tree::Number(expr.substr(prev,now-prev)));
  }
  if(expr[now]=='-'){ //���̐��l�̏ꍇ
    now = expr.find_first_of("],", now);
    return node_sptr(new hydla::parse_tree::Negative(node_sptr(new hydla::parse_tree::Number(expr.substr(prev+1,now-prev-1)))));
  }

  now = expr.find_first_of("[],", prev);
  string_map_t::iterator it = string_map_.find(expr.substr(prev,now-prev));
  if(it == string_map_.end()){
    if(expr.substr(prev,now-prev)=="t"){//����
      return node_sptr(new hydla::parse_tree::SymbolicT());
    }
    if(expr[prev]=='p'){//�萔��
      return node_sptr(new hydla::parse_tree::Parameter(expr.substr(prev+1,now-prev-1)));
    }
    if(expr.substr(prev,now-prev)==PacketSender::var_prefix){//�ϐ���
      return node_sptr(new hydla::parse_tree::Variable(expr.substr(prev,now-prev)));
    }
    
    //��m�[�h�DNumber�m�[�h�ɓ���Ă����΁C�\���o��Mathematica�ł�����菈�����p�����邱�Ƃ͂ł���͂������E�E�E�����I�ɂ͑S�Ή�������
    int depth = 0;
    while(1){
      if(expr[now]=='['){
        depth++;
      }else{
        depth--;
        if(depth<=0){
          if(now == std::string::npos){
            now = expr.length() - 1;
          }else if(depth<0){
            now--;
          }
          break;
        }
      }
      now = expr.find_first_of("[]", now+1);
      assert(now != std::string::npos);
    }
    return node_sptr(new hydla::parse_tree::Number(expr.substr(prev, (now++)-prev+1)));
  }
  
  //�����q�m�[�h������͂��DDerivative�̏ꍇ�͏������ʂ�  
  now++;//[�̕��O�i
  return (*(it->second.function))(expr, now, it->second.node);
}


//����ȍ~��for�`�ɂ��ẮC�񋓌^�g�킸�C�����Ɗy�ɂł������ȋC������
MathematicaExpressionConverter::node_sptr 
  MathematicaExpressionConverter::for_derivative(
    const std::string &expr,
    std::string::size_type &now,
    const MathematicaExpressionConverter::nodeType &nt){
  //�܂�������
  std::string::size_type prev = now;
  int derivative_count;
  now = expr.find("]", 0);
  derivative_count = atoi(expr.substr(prev,now-prev).c_str());
  now+=2;//][
  //���ɒ��g
  node_sptr tmp_node = convert_math_string_to_symbolic_tree(expr, now);
  for(int i=0;i<derivative_count-1;i++){
    tmp_node.reset(new hydla::parse_tree::Differential(tmp_node));
  }
  now++;//]
  return node_sptr(new hydla::parse_tree::Differential(tmp_node));
}

MathematicaExpressionConverter::node_sptr
  MathematicaExpressionConverter::for_unary_node(
    const std::string &expr,
    std::string::size_type &now,
    const MathematicaExpressionConverter::nodeType &nt){
  //���g
  node_sptr tmp_node = convert_math_string_to_symbolic_tree(expr, now);
  now++;//]
  switch(nt){
    default:
    assert(0);
    
    case NODE_PREVIOUS:
    return node_sptr(new hydla::parse_tree::Previous(tmp_node));
    
    case NODE_SQRT:
    return node_sptr(new hydla::parse_tree::Power
      (tmp_node, 
       node_sptr(new hydla::parse_tree::Divide(
          node_sptr(new hydla::parse_tree::Number("1")),
          node_sptr(new hydla::parse_tree::Number("2")) ) ) ) );
  }
}

MathematicaExpressionConverter::node_sptr
  MathematicaExpressionConverter::for_binary_node(
    const std::string &expr,
    std::string::size_type &now,
    const MathematicaExpressionConverter::nodeType &nt){
  //BinaryNode����邽�߂̊֐������ǁCPlus��Times�̓��X�g�ŕ�����������݂�����������ʂɃ��[�v

  //��
  node_sptr lhs = convert_math_string_to_symbolic_tree(expr, now);
  now++;//,
  while(1){
   //�E
   node_sptr rhs = convert_math_string_to_symbolic_tree(expr, now);
   now++;//]��,
   switch(nt){
     case NODE_PLUS:
     if(expr[now-1]==']'){//�����ŏI��
       return node_sptr(new hydla::parse_tree::Plus(lhs, rhs));
     }
     else{
       lhs = node_sptr(new hydla::parse_tree::Plus(lhs, rhs));
     }
     break;

     case NODE_TIMES:
     if(expr[now-1]==']')//�����ŏI��
       return node_sptr(new hydla::parse_tree::Times(lhs, rhs));
     else
       lhs = node_sptr(new hydla::parse_tree::Times(lhs, rhs));
     break;

     case NODE_SUBTRACT:
     return node_sptr(new hydla::parse_tree::Subtract(lhs, rhs));
     case NODE_DIVIDE:
     return node_sptr(new hydla::parse_tree::Divide(lhs, rhs));
     case NODE_POWER:
     return node_sptr(new hydla::parse_tree::Power(lhs, rhs));

     default:
     assert(0);
   }
  }
}

std::string MathematicaExpressionConverter::get_relation_math_string(value_range_t::Relation rel){
  switch(rel){
    case value_range_t::EQUAL:
      return "Equal";
    case value_range_t::NOT_EQUAL:
      return "UnEqual";
    case value_range_t::GREATER_EQUAL:
      return "GreaterEqual";
    case value_range_t::LESS_EQUAL:
      return "LessEqual";
    case value_range_t::GREATER:
      return "Greater";
    case value_range_t::LESS:
      return "Less";
    default:
      assert(0);
  }
}


void MathematicaExpressionConverter::set_parameter_on_value(MathematicaExpressionConverter::value_t &val,const std::string &par_name){
  val.set(node_sptr(new hydla::parse_tree::Parameter(par_name)));
  return;
}

std::string MathematicaExpressionConverter::convert_symbolic_value_to_math_string(const MathematicaExpressionConverter::value_t &val){
  string_for_math_string_.clear();
  differential_count_=0;
  in_prev_=0;
  accept(val.get_node());
  return string_for_math_string_;
}

// ��r���Z�q
void MathematicaExpressionConverter::visit(boost::shared_ptr<Equal> node)                 
{
  string_for_math_string_.append("Equal[");
  accept(node->get_lhs());
  string_for_math_string_.append(", ");
  accept(node->get_rhs());
  string_for_math_string_.append("]");
}

void MathematicaExpressionConverter::visit(boost::shared_ptr<UnEqual> node)               
{
  string_for_math_string_.append("UnEqual[");
  accept(node->get_lhs());
  string_for_math_string_.append(", ");
  accept(node->get_rhs());
  string_for_math_string_.append("]");
}

void MathematicaExpressionConverter::visit(boost::shared_ptr<Less> node)                  
{
  string_for_math_string_.append("Less[");
  accept(node->get_lhs());
  string_for_math_string_.append(", ");
  accept(node->get_rhs());
  string_for_math_string_.append("]");
}

void MathematicaExpressionConverter::visit(boost::shared_ptr<LessEqual> node)             
{
  string_for_math_string_.append("LessEqual[");
  accept(node->get_lhs());
  string_for_math_string_.append(", ");
  accept(node->get_rhs());
  string_for_math_string_.append("]");
}

void MathematicaExpressionConverter::visit(boost::shared_ptr<Greater> node)               
{
  string_for_math_string_.append("Greater[");
  accept(node->get_lhs());
  string_for_math_string_.append(", ");
  accept(node->get_rhs());
  string_for_math_string_.append("]");
}

void MathematicaExpressionConverter::visit(boost::shared_ptr<GreaterEqual> node)          
{
  string_for_math_string_.append("GreaterEqual[");
  accept(node->get_lhs());
  string_for_math_string_.append(", ");
  accept(node->get_rhs());
  string_for_math_string_.append("]");
}

// �_�����Z�q
void MathematicaExpressionConverter::visit(boost::shared_ptr<LogicalAnd> node)            
{
  string_for_math_string_.append("And[");
  accept(node->get_lhs());
  string_for_math_string_.append(", ");
  accept(node->get_rhs());
  string_for_math_string_.append("]");
}

void MathematicaExpressionConverter::visit(boost::shared_ptr<LogicalOr> node)             
{
  string_for_math_string_.append("Or[");
  accept(node->get_lhs());
  string_for_math_string_.append(", ");
  accept(node->get_rhs());
  string_for_math_string_.append("]");
}
  
// �Z�p�񍀉��Z�q
void MathematicaExpressionConverter::visit(boost::shared_ptr<Plus> node)                  
{
  string_for_math_string_.append("Plus[");
  accept(node->get_lhs());
  string_for_math_string_.append(", ");
  accept(node->get_rhs());
  string_for_math_string_.append("]");
}

void MathematicaExpressionConverter::visit(boost::shared_ptr<Subtract> node)              
{
  string_for_math_string_.append("Subtract[");
  accept(node->get_lhs());
  string_for_math_string_.append(", ");
  accept(node->get_rhs());
  string_for_math_string_.append("]");
}

void MathematicaExpressionConverter::visit(boost::shared_ptr<Times> node)                 
{
  string_for_math_string_.append("Times[");
  accept(node->get_lhs());
  string_for_math_string_.append(", ");
  accept(node->get_rhs());
  string_for_math_string_.append("]");
}

void MathematicaExpressionConverter::visit(boost::shared_ptr<Divide> node)                
{
  string_for_math_string_.append("Divide[");
  accept(node->get_lhs());
  string_for_math_string_.append(", ");
  accept(node->get_rhs());
  string_for_math_string_.append("]");
}


void MathematicaExpressionConverter::visit(boost::shared_ptr<Power> node)                
{
  string_for_math_string_.append("Power[");
  accept(node->get_lhs());
  string_for_math_string_.append(", ");
  accept(node->get_rhs());
  string_for_math_string_.append("]");
}
  
// �Z�p�P�����Z�q
void MathematicaExpressionConverter::visit(boost::shared_ptr<Negative> node)              
{
  string_for_math_string_.append("Minus[");

  accept(node->get_child());
  string_for_math_string_.append("]");
}

void MathematicaExpressionConverter::visit(boost::shared_ptr<Positive> node)              
{
  accept(node->get_child());
}

// ����
void MathematicaExpressionConverter::visit(boost::shared_ptr<Differential> node)          
{
  differential_count_++;
  accept(node->get_child());
  differential_count_--;
}

// ���Ɍ�
void MathematicaExpressionConverter::visit(boost::shared_ptr<Previous> node)              
{
  in_prev_++;
  accept(node->get_child());
  in_prev_--;
}
  
// �ϐ�
void MathematicaExpressionConverter::visit(boost::shared_ptr<Variable> node)              
{
  std::ostringstream tmp;
  if(differential_count_){
    tmp << "Derivative[" << differential_count_ << "][";
  }
  
  if(in_prev_){
    tmp << "prev[" << node->get_name() << "]";
  }else{
    tmp << node->get_name();
  }
  
  if(differential_count_){
   tmp << "]";
  }

  string_for_math_string_.append(tmp.str());
}

// ����
void MathematicaExpressionConverter::visit(boost::shared_ptr<Number> node)                
{    
  string_for_math_string_.append(node->get_number());
}


// �L���萔
void MathematicaExpressionConverter::visit(boost::shared_ptr<Parameter> node)                
{    
  string_for_math_string_.append(PacketSender::par_prefix + node->get_name());
}

// t
void MathematicaExpressionConverter::visit(boost::shared_ptr<SymbolicT> node)                
{    
  string_for_math_string_.append("t");
}


} // namespace mathematica
} // namespace vcs
} // namespace hydla 


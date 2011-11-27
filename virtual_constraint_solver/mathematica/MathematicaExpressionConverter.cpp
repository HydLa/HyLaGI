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

std::map<std::string, std::string> MathematicaExpressionConverter::variable_parameter_map_;

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
}

MathematicaExpressionConverter::value_t MathematicaExpressionConverter::convert_math_string_to_symbolic_value(const std::string &expr){

  HYDLA_LOGGER_REST("#*** convert string to value ***\n",
                     expr);
  value_t value;
  std::string::size_type now = 0;
  value.set(convert_math_string_to_symbolic_tree(expr, now));
  HYDLA_LOGGER_REST("#*** convert result value ***\n",
                     value.get_string());
  return value;
}

void MathematicaExpressionConverter::add_parameter_name(std::string variable_name, std::string parameter_name){
  variable_parameter_map_.insert(std::make_pair(variable_name, parameter_name));
}


void MathematicaExpressionConverter::clear_parameter_name(){
  variable_parameter_map_.clear();
}


/**
 * �ivairable�j���inode�j�̌`�̃m�[�h��Ԃ�
 */
MathematicaExpressionConverter::node_sptr MathematicaExpressionConverter::make_equal(const variable_t &variable, const node_sptr& node, const bool& prev){
  HYDLA_LOGGER_REST("*** Begin:MathematicaExpressionConverter::make_equal ***\n");
  node_sptr new_node(new Variable(variable.get_name()));
  for(int i=0;i<variable.get_derivative_count();i++){
    new_node = node_sptr(new Differential(new_node));
  }
  if(prev){
    new_node = node_sptr(new Previous(new_node));
  }
  HYDLA_LOGGER_REST("*** End:MathematicaExpressionConverter::make_equal ***\n");
  return node_sptr(new Equal(new_node, node));
}

MathematicaExpressionConverter::node_sptr MathematicaExpressionConverter::convert_math_string_to_symbolic_tree(const std::string &expr, std::string::size_type &now){
  now = expr.find_first_not_of(" ", now);
  std::string::size_type prev = now;
  if(expr[now]>='0' && expr[now]<='9'){//���̐��l�̏ꍇ
    now = expr.find_first_of("],", now);
    if(now == std::string::npos)now = expr.length();
    return node_sptr(new hydla::parse_tree::Number(expr.substr(prev,now-prev)));
  }
  if(expr[now]=='-'){ //���̐��l�̏ꍇ
    now = expr.find_first_of("],", now);
    if(now == std::string::npos)now = expr.length();
    return node_sptr(new hydla::parse_tree::Negative(node_sptr(new hydla::parse_tree::Number(expr.substr(prev+1,now-prev-1)))));
  }

  now = expr.find_first_of("[],", prev);
  if(now == std::string::npos)now = expr.length();
  string_map_t::iterator it = string_map_.find(expr.substr(prev,now-prev));
  if(it == string_map_.end()){
    if(expr.substr(prev,now-prev)=="t"){//����
      return node_sptr(new hydla::parse_tree::SymbolicT());
    }
    if(expr.length() >= prev + 1 && expr.substr(prev, 2) =="Pi"){ // �~����
      return node_sptr(new hydla::parse_tree::Pi());
    }
    if(expr[prev]=='p'){//�萔��
      return node_sptr(new hydla::parse_tree::Parameter(expr.substr(prev+1,now-prev-1)));
    }
    if(expr[prev]=='E'){//���R�ΐ��̒�
      return node_sptr(new hydla::parse_tree::E());
    }
    if(now-prev > 6 && expr.substr(prev, 6) == PacketSender::var_prefix){//�ϐ���
      std::string variable_name;
	    if(now != std::string::npos){
        variable_name = expr.substr(prev + 6, now-(prev+6));
		    if(expr[now] == '['){
          // [t] �� [0]�����Ă���Γǂ݂Ƃ΂�
			    now = expr.find_first_of("]", now);
			    now++;
        }
	    }else{
        variable_name = expr.substr(prev + 6, std::string::npos);
      }
      
	    std::map<std::string, std::string>::iterator it = variable_parameter_map_.find(variable_name);
	    if(it != variable_parameter_map_.end()){
		    return node_sptr(new hydla::parse_tree::Parameter(it->second));
      }
      return node_sptr(new hydla::parse_tree::Variable(variable_name));
    }
    
    //���Ή��m�[�h�D�\���o��Mathematica�ŗL����菈�����p���ł���悤�ɂ���
    boost::shared_ptr<hydla::parse_tree::ArbitraryNode> new_node(new hydla::parse_tree::ArbitraryNode(expr.substr(prev, (now)-prev)));
    if(expr[now]!='['){
      now++;
      return new_node;
    }else{
      now++;
      while(1){
        new_node->add_argument(convert_math_string_to_symbolic_tree(expr, now));
        if(expr[now] == ']')
          break;
        now++; // ','�̕�
      }
      now++;
      return new_node;
    }
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
  now+=2;// ']' ��'['�̕�
  //���ɒ��g
  node_sptr tmp_node = convert_math_string_to_symbolic_tree(expr, now);
  for(int i=0;i<derivative_count-1;i++){
    tmp_node.reset(new hydla::parse_tree::Differential(tmp_node));
  }
  now++;// ']'�̕�
  return node_sptr(new hydla::parse_tree::Differential(tmp_node));
}

MathematicaExpressionConverter::node_sptr
  MathematicaExpressionConverter::for_unary_node(
    const std::string &expr,
    std::string::size_type &now,
    const MathematicaExpressionConverter::nodeType &nt){
  //���g
  node_sptr tmp_node = convert_math_string_to_symbolic_tree(expr, now);
  now++;// ']'�̕�
  switch(nt){
    default:
    assert(0);
    return node_sptr(new hydla::parse_tree::Previous(tmp_node));
    
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
  now++; // ','�̕�
  while(1){
   //�E
   node_sptr rhs = convert_math_string_to_symbolic_tree(expr, now);
   now++; // ']' ��','�̕�
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
     return node_sptr(new hydla::parse_tree::Power(lhs, rhs));
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
      return "";
  }
}

MathematicaExpressionConverter::node_sptr MathematicaExpressionConverter::get_relation_node(value_range_t::Relation rel, const node_sptr& lhs, const node_sptr& rhs){
  switch(rel){
    case value_range_t::EQUAL:
      return node_sptr(new Equal(lhs, rhs));
    case value_range_t::NOT_EQUAL:
      return node_sptr(new UnEqual(lhs, rhs));
    case value_range_t::GREATER_EQUAL:
      return node_sptr(new GreaterEqual(lhs, rhs));
    case value_range_t::LESS_EQUAL:
      return node_sptr(new LessEqual(lhs, rhs));
    case value_range_t::GREATER:
      return node_sptr(new Greater(lhs, rhs));
    case value_range_t::LESS:
      return node_sptr(new Less(lhs, rhs));
    default:
      assert(0);
    return node_sptr();
  }
}

MathematicaExpressionConverter::value_range_t::Relation MathematicaExpressionConverter::get_relation_from_code(const int &relop_code){
  switch(relop_code){
    case 0: // Equal
      return value_range_t::EQUAL;
      break;

    case 1: // Less
      return value_range_t::LESS;    
      break;

    case 2: // Greater
      return value_range_t::GREATER;
      break;

    case 3: // LessEqual
      return value_range_t::LESS_EQUAL;
      break;

    case 4: // GreaterEqual
      return value_range_t::GREATER_EQUAL;
    default:
      assert(0);
      return value_range_t::GREATER_EQUAL;
  }
}

void MathematicaExpressionConverter::set_parameter_on_value(MathematicaExpressionConverter::value_t &val,const std::string &par_name){
  val.set(node_sptr(new hydla::parse_tree::Parameter(par_name)));
  return;
}


} // namespace mathematica
} // namespace vcs
} // namespace hydla 


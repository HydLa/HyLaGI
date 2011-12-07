#include "SExpConverter.h"


using namespace hydla::parse_tree; // Negative��Number�p

namespace hydla {
namespace vcs {
namespace reduce {

SExpConverter::string_map_t SExpConverter::string_map_;

std::map<std::string, std::string> SExpConverter::variable_parameter_map_;

SExpConverter::SExpConverter() 
{}

SExpConverter::~SExpConverter(){}

void SExpConverter::initialize(){
  //�m�[�h�ƕ�����̑Ή��֌W������Ă����D
  string_map_.insert(std::make_pair("plus", function_and_node(for_binary_node, NODE_PLUS)));
  string_map_.insert(std::make_pair("difference", function_and_node(for_binary_node, NODE_SUBTRACT)));
  string_map_.insert(std::make_pair("times", function_and_node(for_binary_node, NODE_TIMES)));
  string_map_.insert(std::make_pair("quotient", function_and_node(for_binary_node, NODE_DIVIDE)));
  string_map_.insert(std::make_pair("expt", function_and_node(for_binary_node, NODE_POWER)));
  string_map_.insert(std::make_pair("df", function_and_node(for_derivative, NODE_DIFFERENTIAL)));
  string_map_.insert(std::make_pair("prev", function_and_node(for_unary_node, NODE_PREVIOUS)));
  string_map_.insert(std::make_pair("sqrt", function_and_node(for_unary_node, NODE_SQRT)));
  string_map_.insert(std::make_pair("minus", function_and_node(for_unary_node, NODE_NEGATIVE)));
  string_map_.insert(std::make_pair("sin", function_and_node(for_unary_node, NODE_SIN)));
  string_map_.insert(std::make_pair("cos", function_and_node(for_unary_node, NODE_COS)));
  string_map_.insert(std::make_pair("tan", function_and_node(for_unary_node, NODE_TAN)));
  string_map_.insert(std::make_pair("asin", function_and_node(for_unary_node, NODE_ASIN)));
  string_map_.insert(std::make_pair("acos", function_and_node(for_unary_node, NODE_ACOS)));
  string_map_.insert(std::make_pair("atan", function_and_node(for_unary_node, NODE_ATAN)));
}

SExpConverter::value_t SExpConverter::convert_s_exp_to_symbolic_value(SExpParser &sp, const_tree_iter_t iter){

  HYDLA_LOGGER_REST("#*** convert s-expression to value ***\n",
                    iter);

  value_t value;
  value.set(convert_s_exp_to_symbolic_tree(sp, iter));
  HYDLA_LOGGER_REST("#*** convert result value ***\n",
                    value.get_string());

  return value;
}

void SExpConverter::add_parameter_name(std::string variable_name, std::string parameter_name){
  variable_parameter_map_.insert(std::make_pair(variable_name, parameter_name));
}


void SExpConverter::clear_parameter_name(){
  variable_parameter_map_.clear();
}

SExpConverter::node_sptr SExpConverter::make_equal(const variable_t &variable, const node_sptr& node, const bool& prev, const bool& init_var){
  HYDLA_LOGGER_REST("*** Begin:SExpConverter::make_equal ***\n");
  node_sptr new_node(new Variable(variable.get_name(), init_var));
  for(int i=0;i<variable.get_derivative_count();i++){
    new_node = node_sptr(new Differential(new_node));
  }
  if(prev){
    new_node = node_sptr(new Previous(new_node));
  }
  HYDLA_LOGGER_REST("*** End:SExpConverter::make_equal ***\n");
  return node_sptr(new Equal(new_node, node));
}

SExpConverter::node_sptr SExpConverter::convert_s_exp_to_symbolic_tree(SExpParser &sp, const_tree_iter_t iter){

  switch(iter->value.id().to_long()) {
    case SExpGrammar::RI_Number: {
      std::stringstream number_ss;
      std::string number_str = std::string(iter->value.begin(),iter->value.end());
      int number_value;
      number_ss << number_str;
      number_ss >> number_value;
      if(number_value < 0) {
        std::string positive_number_str = std::string(iter->value.begin()+1,iter->value.end());
        return node_sptr(new Negative(node_sptr(new Number(positive_number_str))));
      }
      else {
        assert(number_value >= 0);
        return node_sptr(new Number(number_str));
      }
      break;
    }
    // header��identifier�Ƃŕ�������
    default:
      std::string value_str = std::string(iter->value.begin(), iter->value.end());
      // �ϐ����̐擪�ɃX�y�[�X�����邱�Ƃ�����̂ŏ�������
      // TODO:S���p�[�T���C�����ăX�y�[�X����Ȃ��悤�ɂ���
      if(value_str.at(0) == ' ') value_str.erase(0,1);

      string_map_t::const_iterator strmap_it = string_map_.find(value_str);
      if(strmap_it == string_map_.end()){
        if(value_str=="t"){//����
          return node_sptr(new hydla::parse_tree::SymbolicT());
        }
        if(value_str=="pi"){ // �~����
          return node_sptr(new hydla::parse_tree::Pi());
        }
        if(value_str.at(0)=='p'){//�萔��
          return node_sptr(new hydla::parse_tree::Parameter(value_str.substr(1,value_str.length()-1)));
        }
        if(value_str=="e"){//���R�ΐ��̒�
          return node_sptr(new hydla::parse_tree::E());
        }
        
        // TODO:�ϐ����₻��ȊO��factor�ւ̑Ή�
        assert(0);
      }

      return (*(strmap_it->second.function))(sp, iter, strmap_it->second.node);
  }
}

SExpConverter::node_sptr SExpConverter::for_derivative(
  SExpParser &sp,
  const_tree_iter_t iter,
  const SExpConverter::nodeType &nt){
  //�܂�������
  int derivative_count = sp.get_derivative_count(iter);
  //���ɒ��g
  node_sptr tmp_node = convert_s_exp_to_symbolic_tree(sp, iter->children.begin());
  for(int i=0;i<derivative_count-1;i++){
    tmp_node.reset(new hydla::parse_tree::Differential(tmp_node));
  }

  return node_sptr(new hydla::parse_tree::Differential(tmp_node));
}

SExpConverter::node_sptr SExpConverter::for_unary_node(
  SExpParser &sp,
  const_tree_iter_t iter,
  const SExpConverter::nodeType &nt){
  //���g
  node_sptr tmp_node = convert_s_exp_to_symbolic_tree(sp, iter->children.begin());
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

    case NODE_NEGATIVE:
      return node_sptr(new hydla::parse_tree::Negative(tmp_node));

    case NODE_SIN:
      return node_sptr(new hydla::parse_tree::Sin(tmp_node));

    case NODE_COS:
      return node_sptr(new hydla::parse_tree::Cos(tmp_node));

    case NODE_TAN:
      return node_sptr(new hydla::parse_tree::Tan(tmp_node));

    case NODE_ASIN:
      return node_sptr(new hydla::parse_tree::Asin(tmp_node));

    case NODE_ACOS:
      return node_sptr(new hydla::parse_tree::Acos(tmp_node));

    case NODE_ATAN:
      return node_sptr(new hydla::parse_tree::Atan(tmp_node));
  }
}

SExpConverter::node_sptr SExpConverter::for_binary_node(
  SExpParser &sp,
  const_tree_iter_t iter,
  const SExpConverter::nodeType &nt){
  //BinaryNode����邽�߂̊֐������ǁCplus��times�̓��X�g�ŕ�����������݂�����������ʂɃ��[�v

  //��
  node_sptr lhs = convert_s_exp_to_symbolic_tree(sp, iter->children.begin());
  size_t args_count = 0;
  while(1){
    args_count++;
    //�E
    node_sptr rhs = convert_s_exp_to_symbolic_tree(sp, iter->children.begin()+args_count);
    switch(nt){
      case NODE_PLUS:
        if(args_count == iter->children.size()-1){//�����ŏI��
          return node_sptr(new hydla::parse_tree::Plus(lhs, rhs));
        }
        else{
          lhs = node_sptr(new hydla::parse_tree::Plus(lhs, rhs));
        }
        break;

      case NODE_TIMES:
        if(args_count == iter->children.size()-1)//�����ŏI��
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

SExpConverter::value_range_t::Relation SExpConverter::get_relation_from_code(const int &relop_code){
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

void SExpConverter::set_parameter_on_value(SExpConverter::value_t &val,const std::string &par_name){
  val.set(node_sptr(new hydla::parse_tree::Parameter(par_name)));
  return;
}

} // namespace reduce
} // namespace vcs
} // namespace hydla

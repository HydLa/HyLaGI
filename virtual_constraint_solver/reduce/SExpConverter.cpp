#include "SExpConverter.h"


using namespace hydla::parse_tree; // NegativeとNumber用

namespace hydla {
namespace vcs {
namespace reduce {

SExpConverter::string_map_t SExpConverter::string_map_;

SExpConverter::SExpConverter() 
{}

SExpConverter::~SExpConverter(){}

void SExpConverter::initialize(){
  //ノードと文字列の対応関係を作っておく．
  string_map_.insert(std::make_pair("plus", function_and_node(for_binary_node, NODE_PLUS)));
  string_map_.insert(std::make_pair("difference", function_and_node(for_binary_node, NODE_SUBTRACT)));
  string_map_.insert(std::make_pair("times", function_and_node(for_binary_node, NODE_TIMES)));
  string_map_.insert(std::make_pair("quotient", function_and_node(for_binary_node, NODE_DIVIDE)));
  string_map_.insert(std::make_pair("expt", function_and_node(for_binary_node, NODE_POWER)));
  string_map_.insert(std::make_pair("df", function_and_node(for_derivative, NODE_DIFFERENTIAL)));
  string_map_.insert(std::make_pair("prev", function_and_node(for_unary_node, NODE_PREVIOUS)));
  string_map_.insert(std::make_pair("sqrt", function_and_node(for_unary_node, NODE_SQRT)));
  string_map_.insert(std::make_pair("minus", function_and_node(for_unary_node, NODE_NEGATIVE)));
}

SExpConverter::value_t SExpConverter::convert_s_exp_to_symbolic_value(const SExpParser& sp, const_tree_iter_t iter){
  HYDLA_LOGGER_FUNC_BEGIN(REST);
  HYDLA_LOGGER_REST("--- convert s-expression to value ---\n", iter);

  value_t value(new hydla::simulator::symbolic::SymbolicValue(convert_s_exp_to_symbolic_tree(sp, iter)));

  HYDLA_LOGGER_REST("--- convert result value ---\n", value->get_string());
  HYDLA_LOGGER_FUNC_END(REST);
  return value;
}

//TODO init_var引数をなくす
node_sptr SExpConverter::make_equal(const variable_t &variable, const node_sptr& node, const bool& prev, const bool& init_var){
  HYDLA_LOGGER_FUNC_BEGIN(REST);
  node_sptr new_node(new Variable(variable.get_name()));
  for(int i=0;i<variable.get_derivative_count();i++){
    new_node = node_sptr(new Differential(new_node));
  }
  if(prev){
    new_node = node_sptr(new Previous(new_node));
  }
  HYDLA_LOGGER_FUNC_END(REST);
  return node_sptr(new Equal(new_node, node));
}

node_sptr SExpConverter::make_equal(hydla::simulator::DefaultParameter &variable, const node_sptr& node, const bool& prev, const bool& init_var){
  HYDLA_LOGGER_FUNC_BEGIN(REST);
  node_sptr new_node(new Variable(variable.get_name()));
  for(int i=0;i<variable.get_derivative_count();i++){
    new_node = node_sptr(new Differential(new_node));
  }
  if(prev){
    new_node = node_sptr(new Previous(new_node));
  }
  HYDLA_LOGGER_FUNC_END(REST);
  return node_sptr(new Equal(new_node, node));
}

SExpConverter::node_sptr SExpConverter::convert_s_exp_to_symbolic_tree(const SExpParser& sp, const_tree_iter_t iter){

  switch(iter->value.id().to_long()) {
    case SExpGrammar::RI_Number: 
      {
        std::string number_str = std::string(iter->value.begin(),iter->value.end());
        assert(number_str.length()>0);
        if(number_str[0] == '-'){ // number_value < 0
          const std::string positive_number_str = std::string(iter->value.begin()+1,iter->value.end());
          return node_sptr(new Negative(node_sptr(new Number(positive_number_str))));
        }else{
          return node_sptr(new Number(number_str));
        }
        break;
      }

    // headerとidentifierとで分けたい
    default:
      std::string value_str = std::string(iter->value.begin(), iter->value.end());
      // 変数名の先頭にスペースが入ることがあるので除去する
      // TODO:S式パーサを修正してスペース入らないようにする
      if(value_str.at(0) == ' ') value_str.erase(0,1);

      string_map_t::const_iterator strmap_it = string_map_.find(value_str);
      if(strmap_it == string_map_.end()){
        if(value_str=="t"){//時刻
          return node_sptr(new hydla::parse_tree::SymbolicT());
        }
        if(value_str=="pi"){ // 円周率
          return node_sptr(new hydla::parse_tree::Pi());
        }
        if(value_str.at(0)=='p'){//定数名
          assert(0);
          // TODO
          // return node_sptr(new hydla::parse_tree::Parameter(value_str.substr(1,value_str.length()-1)));
        }
        if(value_str=="e"){//自然対数の底
          return node_sptr(new hydla::parse_tree::E());
        }
        
        // TODO:変数名やそれ以外のfactorへの対応
        assert(0);
      }

      return (*(strmap_it->second.function))(sp, iter, strmap_it->second.node);
      break;
  }

  assert(0);
  return node_sptr();
}


SExpConverter::node_sptr SExpConverter::for_derivative(
  const SExpParser &sp,
  const_tree_iter_t iter,
  const SExpConverter::nodeType &nt){
  //まず微分回数
  int derivative_count = sp.get_derivative_count(iter);
  //次に中身
  node_sptr tmp_node = convert_s_exp_to_symbolic_tree(sp, iter->children.begin());
  for(int i=0;i<derivative_count-1;i++){
    tmp_node.reset(new hydla::parse_tree::Differential(tmp_node));
  }

  return node_sptr(new hydla::parse_tree::Differential(tmp_node));
}

SExpConverter::node_sptr SExpConverter::for_unary_node(
  const SExpParser &sp,
  const_tree_iter_t iter,
  const SExpConverter::nodeType &nt){
  //中身
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
  }
}

SExpConverter::node_sptr SExpConverter::for_binary_node(
  const SExpParser &sp,
  const_tree_iter_t iter,
  const SExpConverter::nodeType &nt){
  //BinaryNodeを作るための関数だけど，plusとtimesはリストで複数引数取れるみたいだから特別にループ

  //左
  node_sptr lhs = convert_s_exp_to_symbolic_tree(sp, iter->children.begin());
  size_t args_count = 0;
  while(1){
    args_count++;
    //右
    node_sptr rhs = convert_s_exp_to_symbolic_tree(sp, iter->children.begin()+args_count);
    switch(nt){
      case NODE_PLUS:
        if(args_count == iter->children.size()-1){//ここで終了
          return node_sptr(new hydla::parse_tree::Plus(lhs, rhs));
        }
        else{
          lhs = node_sptr(new hydla::parse_tree::Plus(lhs, rhs));
        }
        break;

      case NODE_TIMES:
        if(args_count == iter->children.size()-1)//ここで終了
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



void SExpConverter::set_range(const value_t &val, value_range_t &range, const int& relop){
  switch(relop){
    case 0://Equal
    range.set_unique(val);
    break;
    
    case 1://Less
    range.set_upper_bound(val, false);
    break;
    case 2://Greater
    range.set_lower_bound(val, false);
    break;
    case 3://LessEqual
    range.set_upper_bound(val, true);
    break;
    case 4://GreaterEqual
    range.set_lower_bound(val, true);
    break;
  }
}

void SExpConverter::set_parameter_on_value(value_t &val,const parameter_t &par){
  assert(0);
  //TODO
  //val.set(node_sptr(new hydla::parse_tree::Parameter(par.get_name())));
  return;
}

} // namespace reduce
} // namespace vcs
} // namespace hydla

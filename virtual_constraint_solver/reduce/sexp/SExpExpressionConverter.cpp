#include "SExpExpressionConverter.h"

#include "../REDUCEStringSender.h"

using namespace hydla::parse_tree; // NegativeとNumber用

namespace hydla {
namespace vcs {
namespace reduce {

SExpExpressionConverter::SExpExpressionConverter() 
{}

SExpExpressionConverter::~SExpExpressionConverter(){}

//TODO init_var引数をなくす
SExpExpressionConverter::node_sptr SExpExpressionConverter::make_equal(const variable_t &variable, const node_sptr& node, const bool& prev, const bool& init_var){
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

SExpExpressionConverter::node_sptr SExpExpressionConverter::make_equal(hydla::simulator::DefaultParameter &variable, const node_sptr& node, const bool& prev, const bool& init_var){
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

void SExpExpressionConverter::set_range(const value_t &val, value_range_t &range, const int& relop){
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

void SExpExpressionConverter::set_parameter_on_value(value_t &val,const parameter_t &par){
  assert(0);
  //TODO
  //val.set(node_sptr(new hydla::parse_tree::Parameter(par.get_name())));
  return;
}

//
// const_tree_iter_tの問い合わせ
// 

SExpExpressionConverter::string_map_t SExpExpressionConverter::string_map_;

void SExpExpressionConverter::initialize(){
  //ノードと文字列の対応関係を作っておく．
  string_map_.insert(std::make_pair("plus", function_and_node(&SExpExpressionConverter::for_binary_node, NODE_PLUS)));
  string_map_.insert(std::make_pair("difference", function_and_node(&SExpExpressionConverter::for_binary_node, NODE_SUBTRACT)));
  string_map_.insert(std::make_pair("times", function_and_node(&SExpExpressionConverter::for_binary_node, NODE_TIMES)));
  string_map_.insert(std::make_pair("quotient", function_and_node(&SExpExpressionConverter::for_binary_node, NODE_DIVIDE)));
  string_map_.insert(std::make_pair("expt", function_and_node(&SExpExpressionConverter::for_binary_node, NODE_POWER)));
  string_map_.insert(std::make_pair("df", function_and_node(&SExpExpressionConverter::for_derivative, NODE_DIFFERENTIAL)));
  string_map_.insert(std::make_pair("prev", function_and_node(&SExpExpressionConverter::for_unary_node, NODE_PREVIOUS)));
  string_map_.insert(std::make_pair("sqrt", function_and_node(&SExpExpressionConverter::for_unary_node, NODE_SQRT)));
  string_map_.insert(std::make_pair("minus", function_and_node(&SExpExpressionConverter::for_unary_node, NODE_NEGATIVE)));
}

int SExpExpressionConverter::get_derivative_count(const_tree_iter_t var_iter){
  int var_derivative_count = 0;
  std::string var_str = std::string(var_iter->value.begin(), var_iter->value.end());

  // dfの先頭にスペースが入ることがあるので除去する
  // TODO:S式パーサを修正してスペース入らないようにする
  if(var_str.at(0) == ' ') var_str.erase(0,1);

  // 微分を含む変数
  if(var_str=="df"){
    size_t df_child_size = var_iter->children.size();

    // 1回微分の場合は微分回数部分が省略されている
    if(df_child_size==2){
      var_derivative_count = 1;
    }
    else{
      assert(df_child_size==3);
      std::istringstream dc_ss(to_string(caddr(var_iter)));
      dc_ss >> var_derivative_count;
    }
  }

  return var_derivative_count;
}

std::string SExpExpressionConverter::to_string(const_tree_iter_t iter, bool isFirst){
  std::string str(iter->value.begin(), iter->value.end());

  // サーバーモードではminus使用不可
  str = (str=="minus") ? "-" : str;

  if(iter->children.size() > 0){
    str += '(';
    for(int i = 0; i < (int)iter->children.size(); ++i){
      str += to_string(iter->children.begin() + i, false) + ' ';
    }
    str = str.substr(0, str.length() - 1);
    str += ')';
  }
  if(isFirst && str.find("list")==0){
    str =  '(' + str + ')';
  }

  return str;
}

SExpExpressionConverter::value_t SExpExpressionConverter::to_value(const_tree_iter_t iter){
  HYDLA_LOGGER_FUNC_BEGIN(REST);
  // TODOHYDLA_LOGGER_REST("--- convert s-expression to value ---\n", iter);

  value_t value(new hydla::simulator::symbolic::SymbolicValue(to_symbolic_tree(iter)));

  HYDLA_LOGGER_REST("--- convert result value ---\n", value->get_string());
  HYDLA_LOGGER_FUNC_END(REST);
  return value;
}

SExpExpressionConverter::const_tree_iter_t SExpExpressionConverter::car(const_tree_iter_t iter){
  assert(iter->children.size() >= 1);
  return iter->children.begin();
}

SExpExpressionConverter::const_tree_iter_t SExpExpressionConverter::cadr(const_tree_iter_t iter){
  assert(iter->children.size() >= 2);
  return iter->children.begin() + 1;
}

SExpExpressionConverter::const_tree_iter_t SExpExpressionConverter::caddr(const_tree_iter_t iter){
  assert(iter->children.size() >= 3);
  return iter->children.begin() + 2;
}

SExpExpressionConverter::const_tree_iter_t SExpExpressionConverter::cadddr(const_tree_iter_t iter){
  assert(iter->children.size() >= 4);
  return iter->children.begin() + 3;
}


SExpExpressionConverter::node_sptr SExpExpressionConverter::for_derivative(
  const_tree_iter_t iter,
  const SExpExpressionConverter::nodeType &nt){
  //まず微分回数
  int derivative_count = get_derivative_count(iter);
  //次に中身
  node_sptr tmp_node = to_symbolic_tree(iter->children.begin());
  for(int i=0;i<derivative_count-1;i++){
    tmp_node.reset(new hydla::parse_tree::Differential(tmp_node));
  }

  return node_sptr(new hydla::parse_tree::Differential(tmp_node));
}

SExpExpressionConverter::node_sptr SExpExpressionConverter::for_unary_node(
  const_tree_iter_t iter,
  const SExpExpressionConverter::nodeType &nt){
  //中身
  node_sptr tmp_node = to_symbolic_tree(iter->children.begin());
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

SExpExpressionConverter::node_sptr SExpExpressionConverter::for_binary_node(
  const_tree_iter_t iter,
  const SExpExpressionConverter::nodeType &nt){
  //BinaryNodeを作るための関数だけど，plusとtimesはリストで複数引数取れるみたいだから特別にループ

  //左
  node_sptr lhs = to_symbolic_tree(iter->children.begin());
  size_t args_count = 0;
  while(1){
    args_count++;
    //右
    node_sptr rhs = to_symbolic_tree(iter->children.begin()+args_count);
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

SExpExpressionConverter::node_sptr SExpExpressionConverter::to_symbolic_tree(const_tree_iter_t iter){
  switch(iter->value.id().to_long()) {
    case SExpGrammar::RI_Number: 
    {
      std::string number_str = to_string(iter);
      assert(number_str.length()>0);
      if(number_str[0] == '-'){ // number_value < 0
        const std::string positive_number_str = number_str.substr(1);
        return node_sptr(new Negative(node_sptr(new Number(positive_number_str))));
      }else{
        return node_sptr(new Number(number_str));
      }
      break;
    }

    // headerとidentifierとで分けたい
    case SExpGrammar::RI_Identifier:
    {
      std::string value_str = std::string(iter->value.begin(), iter->value.end());
      // 変数名の先頭にスペースが入ることがあるので除去する
      // TODO:S式パーサを修正してスペース入らないようにする
      if(value_str.at(0) == ' ') value_str.erase(0,1);

      string_map_t::const_iterator strmap_it = string_map_.find(value_str);
      if(strmap_it == string_map_.end()){
        if(value_str=="t"){//時刻
          return node_sptr(new hydla::parse_tree::SymbolicT());
        }
        if(value_str=="pi"){// 円周率
          return node_sptr(new hydla::parse_tree::Pi());
        }
        if(value_str=="e"){//自然対数の底
          return node_sptr(new hydla::parse_tree::E());
        }
        if(value_str.find(hydla::vcs::reduce::REDUCEStringSender::par_prefix) == 0){//定数名
          // "parameter_name_diffcount_id"の格好で識別子を受け取ったはず
          std::replace(value_str.begin(), value_str.end(), '_', ' ');
          std::istringstream iss(value_str.substr(hydla::vcs::reduce::REDUCEStringSender::par_prefix.size() + 1));

          std::string name;
          int derivative_count, id;
          iss >> name >> derivative_count >> id;
          return node_sptr(new hydla::parse_tree::Parameter(name, derivative_count, id));
        }

        // TODO:変数名やそれ以外のfactorへの対応
        assert(0);
      }

      return (strmap_it->second.function)(iter, strmap_it->second.node);
      break;

    }

    default:
      assert(0);
      break;
  }

  assert(0);
  return SExpExpressionConverter::node_sptr();
}

} // namespace reduce
} // namespace vcs
} // namespace hydla

#include "SExpParseTree.h"

#include "../../../common/Logger.h"
#include "SExpGrammar.h"
#include <iostream>
#include <sstream>

using namespace hydla::parse_tree; // NegativeとNumber用

namespace hydla {
namespace parser {

const std::string SExpParseTree::empty_list_s_exp("list");
SExpParseTree::string_map_t SExpParseTree::string_map_;

SExpParseTree::SExpParseTree(const std::string& input_str)
  : identifier_(input_str), ast_tree_(SExpParser::parse(input_str.c_str())) {}
SExpParseTree::SExpParseTree(const SExpParseTree& sp)
  : identifier_(sp.identifier_), ast_tree_(sp.ast_tree_) {}

SExpParseTree::~SExpParseTree(){}

void SExpParseTree::initialize(){
  //ノードと文字列の対応関係を作っておく．
  string_map_.insert(std::make_pair("plus", function_and_node(&SExpParseTree::for_binary_node, NODE_PLUS)));
  string_map_.insert(std::make_pair("difference", function_and_node(&SExpParseTree::for_binary_node, NODE_SUBTRACT)));
  string_map_.insert(std::make_pair("times", function_and_node(&SExpParseTree::for_binary_node, NODE_TIMES)));
  string_map_.insert(std::make_pair("quotient", function_and_node(&SExpParseTree::for_binary_node, NODE_DIVIDE)));
  string_map_.insert(std::make_pair("expt", function_and_node(&SExpParseTree::for_binary_node, NODE_POWER)));
  string_map_.insert(std::make_pair("df", function_and_node(&SExpParseTree::for_derivative, NODE_DIFFERENTIAL)));
  string_map_.insert(std::make_pair("prev", function_and_node(&SExpParseTree::for_unary_node, NODE_PREVIOUS)));
  string_map_.insert(std::make_pair("sqrt", function_and_node(&SExpParseTree::for_unary_node, NODE_SQRT)));
  string_map_.insert(std::make_pair("minus", function_and_node(&SExpParseTree::for_unary_node, NODE_NEGATIVE)));
}

bool SExpParseTree::operator==(const SExpParseTree& rhs){
  return identifier_ == rhs.get_id();
}

bool SExpParseTree::operator!=(const SExpParseTree& rhs){
  return !(identifier_ == rhs.get_id());
}

std::string SExpParseTree::get_id() const {
  return identifier_;
}

std::string SExpParseTree::get_string_from_tree(const_tree_iter_t iter){ 
  std::ostringstream ret_str;

  // サーバーモードではminus使用不可
  std::string str = std::string(iter->value.begin(), iter->value.end());
  if(str == "minus") ret_str << "-";
  else ret_str << str;

  if(iter->children.size()>0){
    ret_str << "(";
    for(size_t i=0; i<iter->children.size();i++){
      if(i!=0) ret_str << ",";
      ret_str << get_string_from_tree(iter->children.begin()+i);
    }
    ret_str << ")";
  }

  return ret_str.str();
}

int SExpParseTree::get_derivative_count(const_tree_iter_t var_iter) const {
  int var_derivative_count;
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
      std::stringstream dc_ss;
      std::string dc_str = std::string((var_iter->children.begin()+2)->value.begin(),
                                       (var_iter->children.begin()+2)->value.end());
      dc_ss << dc_str;
      dc_ss >> var_derivative_count;
    }
  }
  else {
    var_derivative_count = 0;
  }

  return var_derivative_count;
}

void SExpParseTree::dump_tree(const_tree_iter_t iter, int nest) {
  for(int i = 0; i < nest; ++i)
    std::cout << "  ";

  if (iter->value.id() == SExpGrammar::RI_Identifier)
    std::cout << "識別子 " << "\"" << std::string(iter->value.begin(), iter->value.end()) << "\"";
  else if (iter->value.id() == SExpGrammar::RI_Number)
    std::cout << "整数 " << "\"" << std::string(iter->value.begin(), iter->value.end()) << "\"";
  else if (iter->value.id() == SExpGrammar::RI_String)
    std::cout << "文字列 "<< "\"" << std::string(iter->value.begin(), iter->value.end()) << "\"";
  else if (iter->value.id() == SExpGrammar::RI_List)
    std::cout << "リスト ";
  else if (iter->value.id() == SExpGrammar::RI_Data)
    std::cout << "データ ";
  else
    std::cout << "Node '" << std::string(iter->value.begin(), iter->value.end()) << "'";

  std::cout << std::endl;

  for(size_t j = 0; j < iter->children.size(); ++j)
    dump_tree(iter->children.begin()+j, nest+1);
}

SExpParseTree::const_tree_iter_t SExpParseTree::root() const {
  return ast_tree_.trees.begin();
}

SExpParseTree::const_tree_iter_t SExpParseTree::car(const_tree_iter_t iter) const {
  assert(iter->children.size() >= 1);
  return iter->children.begin();
}

SExpParseTree::const_tree_iter_t SExpParseTree::cadr(const_tree_iter_t iter) const {
  assert(iter->children.size() >= 2);
  return iter->children.begin() + 1;
}

SExpParseTree::const_tree_iter_t SExpParseTree::caddr(const_tree_iter_t iter) const {
  assert(iter->children.size() >= 3);
  return iter->children.begin() + 2;
}

SExpParseTree::const_tree_iter_t SExpParseTree::cadddr(const_tree_iter_t iter) const {
  assert(iter->children.size() >= 4);
  return iter->children.begin() + 3;
}

SExpParseTree::const_tree_iter_t SExpParseTree::get_tree_iterator() const {
  return ast_tree_.trees.begin();
}

std::string SExpParseTree::to_string(const_tree_iter_t iter, bool isFirst) const {
  std::string str(iter->value.begin(), iter->value.end());
  if(iter->children.size() > 0){
    str += '(';
    for(int i = 0; i < iter->children.size(); ++i){
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

SExpParseTree::value_t SExpParseTree::to_value(const_tree_iter_t iter) const {
  HYDLA_LOGGER_FUNC_BEGIN(REST);
  // TODOHYDLA_LOGGER_REST("--- convert s-expression to value ---\n", iter);

  value_t value(new hydla::simulator::symbolic::SymbolicValue(to_symbolic_tree(iter)));

  HYDLA_LOGGER_REST("--- convert result value ---\n", value->get_string());
  HYDLA_LOGGER_FUNC_END(REST);
  return value;
}

SExpParseTree::node_sptr SExpParseTree::to_symbolic_tree(const_tree_iter_t iter) const {
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

      return (this->*(strmap_it->second.function))(iter, strmap_it->second.node);
      break;
  }

  assert(0);
  return SExpParseTree::node_sptr();
}

SExpParseTree::node_sptr SExpParseTree::for_derivative(
  const_tree_iter_t iter,
  const SExpParseTree::nodeType &nt) const {
  //まず微分回数
  int derivative_count = get_derivative_count(iter);
  //次に中身
  node_sptr tmp_node = to_symbolic_tree(iter->children.begin());
  for(int i=0;i<derivative_count-1;i++){
    tmp_node.reset(new hydla::parse_tree::Differential(tmp_node));
  }

  return node_sptr(new hydla::parse_tree::Differential(tmp_node));
}

SExpParseTree::node_sptr SExpParseTree::for_unary_node(
  const_tree_iter_t iter,
  const SExpParseTree::nodeType &nt) const {
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

SExpParseTree::node_sptr SExpParseTree::for_binary_node(
  const_tree_iter_t iter,
  const SExpParseTree::nodeType &nt) const {
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

SExpParseTree::SExpParseTree(){}

} // namespace vcs
} // namespace hydla


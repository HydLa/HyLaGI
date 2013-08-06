#include "SExpParseTree.h"

#include "../../../common/Logger.h"
#include <iostream>
#include <sstream>

using namespace hydla::parse_tree; // Negative$B$H(BNumber$BMQ(B

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
  //$B%N!<%I$HJ8;zNs$NBP1~4X78$r:n$C$F$*$/!%(B
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

  // $B%5!<%P!<%b!<%I$G$O(Bminus$B;HMQIT2D(B
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

  // df$B$N@hF,$K%9%Z!<%9$,F~$k$3$H$,$"$k$N$G=|5n$9$k(B
  // TODO:S$B<0%Q!<%5$r=$@5$7$F%9%Z!<%9F~$i$J$$$h$&$K$9$k(B
  if(var_str.at(0) == ' ') var_str.erase(0,1);

  // $BHyJ,$r4^$`JQ?t(B
  if(var_str=="df"){
    size_t df_child_size = var_iter->children.size();

    // 1$B2sHyJ,$N>l9g$OHyJ,2s?tItJ,$,>JN,$5$l$F$$$k(B
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
    std::cout << "$B<1JL;R(B " << "\"" << std::string(iter->value.begin(), iter->value.end()) << "\"";
  else if (iter->value.id() == SExpGrammar::RI_Number)
    std::cout << "$B@0?t(B " << "\"" << std::string(iter->value.begin(), iter->value.end()) << "\"";
  else if (iter->value.id() == SExpGrammar::RI_String)
    std::cout << "$BJ8;zNs(B "<< "\"" << std::string(iter->value.begin(), iter->value.end()) << "\"";
  else if (iter->value.id() == SExpGrammar::RI_List)
    std::cout << "$B%j%9%H(B ";
  else if (iter->value.id() == SExpGrammar::RI_Data)
    std::cout << "$B%G!<%?(B ";
  else
    std::cout << "Node '" << std::string(iter->value.begin(), iter->value.end()) << "'";

  std::cout << std::endl;

  for(size_t j = 0; j < iter->children.size(); ++j)
    dump_tree(iter->children.begin()+j, nest+1);
}

SExpParseTree::const_tree_iter_t SExpParseTree::get_tree_iterator() const {
  return ast_tree_.trees.begin();
}

SExpParseTree::value_t SExpParseTree::to_symbolic_value(const_tree_iter_t iter) const {
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

    // header$B$H(Bidentifier$B$H$GJ,$1$?$$(B
    default:
      std::string value_str = std::string(iter->value.begin(), iter->value.end());
      // $BJQ?tL>$N@hF,$K%9%Z!<%9$,F~$k$3$H$,$"$k$N$G=|5n$9$k(B
      // TODO:S$B<0%Q!<%5$r=$@5$7$F%9%Z!<%9F~$i$J$$$h$&$K$9$k(B
      if(value_str.at(0) == ' ') value_str.erase(0,1);

      string_map_t::const_iterator strmap_it = string_map_.find(value_str);
      if(strmap_it == string_map_.end()){
        if(value_str=="t"){//$B;~9o(B
          return node_sptr(new hydla::parse_tree::SymbolicT());
        }
        if(value_str=="pi"){ // $B1_<~N((B
          return node_sptr(new hydla::parse_tree::Pi());
        }
        if(value_str.at(0)=='p'){//$BDj?tL>(B
          assert(0);
          // TODO
          // return node_sptr(new hydla::parse_tree::Parameter(value_str.substr(1,value_str.length()-1)));
        }
        if(value_str=="e"){//$B<+A3BP?t$NDl(B
          return node_sptr(new hydla::parse_tree::E());
        }
        
        // TODO:$BJQ?tL>$d$=$l0J30$N(Bfactor$B$X$NBP1~(B
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
  //$B$^$:HyJ,2s?t(B
  int derivative_count = get_derivative_count(iter);
  //$B<!$KCf?H(B
  node_sptr tmp_node = to_symbolic_tree(iter->children.begin());
  for(int i=0;i<derivative_count-1;i++){
    tmp_node.reset(new hydla::parse_tree::Differential(tmp_node));
  }

  return node_sptr(new hydla::parse_tree::Differential(tmp_node));
}

SExpParseTree::node_sptr SExpParseTree::for_unary_node(
  const_tree_iter_t iter,
  const SExpParseTree::nodeType &nt) const {
  //$BCf?H(B
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
  //BinaryNode$B$r:n$k$?$a$N4X?t$@$1$I!$(Bplus$B$H(Btimes$B$O%j%9%H$GJ#?t0z?t<h$l$k$_$?$$$@$+$iFCJL$K%k!<%W(B

  //$B:8(B
  node_sptr lhs = to_symbolic_tree(iter->children.begin());
  size_t args_count = 0;
  while(1){
    args_count++;
    //$B1&(B
    node_sptr rhs = to_symbolic_tree(iter->children.begin()+args_count);
    switch(nt){
      case NODE_PLUS:
        if(args_count == iter->children.size()-1){//$B$3$3$G=*N;(B
          return node_sptr(new hydla::parse_tree::Plus(lhs, rhs));
        }
        else{
          lhs = node_sptr(new hydla::parse_tree::Plus(lhs, rhs));
        }
        break;

      case NODE_TIMES:
        if(args_count == iter->children.size()-1)//$B$3$3$G=*N;(B
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


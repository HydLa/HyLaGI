#include "TreeInfixPrinter.h"

namespace hydla {
namespace symbolic_simulator{


//valueとって文字列に変換する
std::ostream& TreeInfixPrinter::print_infix(const TreeInfixPrinter::node_sptr & node, std::ostream& s){
  need_par_ = PAR_NONE;
  output_stream_ = &s;
  //s << "print_infix\n";
  accept(node);
  return s;
}


void TreeInfixPrinter::print_binary_node(const hydla::parse_tree::BinaryNode &node, const std::string &symbol, const needParenthesis &pre_par, const needParenthesis &post_par){

  //(*output_stream_) << "binary_node\n";
  need_par_ = pre_par;
  accept(node.get_lhs());
  (*output_stream_) << symbol;
  need_par_ = post_par;
  accept(node.get_rhs());
}
void TreeInfixPrinter::print_unary_node(const hydla::parse_tree::UnaryNode &node, const std::string &pre, const std::string &post){
  //(*output_stream_) << "unary_node\n";
  (*output_stream_) << pre;
  accept(node.get_child());
  (*output_stream_) << post;
}

#define DEFINE_INFIX_VISIT_BINARY(NAME, SYMBOL)  \
void TreeInfixPrinter::visit(boost::shared_ptr<hydla::parse_tree::NAME> node){\
  print_binary_node(*node, #SYMBOL);\
}



//弱合成
DEFINE_INFIX_VISIT_BINARY(Weaker, <<)

// 比較演算子
DEFINE_INFIX_VISIT_BINARY(Equal, =)
DEFINE_INFIX_VISIT_BINARY(UnEqual, !=)
DEFINE_INFIX_VISIT_BINARY(Less, <)
DEFINE_INFIX_VISIT_BINARY(LessEqual, <=)
DEFINE_INFIX_VISIT_BINARY(Greater, >)
DEFINE_INFIX_VISIT_BINARY(GreaterEqual, >=)
// 論理演算子
DEFINE_INFIX_VISIT_BINARY(LogicalAnd, &)

void TreeInfixPrinter::visit(boost::shared_ptr<hydla::parse_tree::LogicalOr> node){
  (*output_stream_) << "(";
  print_binary_node(*node, "|");
  (*output_stream_) << ")";
}

  // Ask制約
DEFINE_INFIX_VISIT_BINARY(Ask, =>)


#define DEFINE_INFIX_VISIT_UNARY(NAME, PRE, POST)  \
void TreeInfixPrinter::visit(boost::shared_ptr<hydla::parse_tree::NAME> node){\
  print_unary_node(*node, PRE, POST);\
}

//単項演算子 "+"
DEFINE_INFIX_VISIT_UNARY(Positive, "", "'")

//微分
DEFINE_INFIX_VISIT_UNARY(Differential, "", "'")
//左極限
DEFINE_INFIX_VISIT_UNARY(Previous, "", "-")
//否定
DEFINE_INFIX_VISIT_UNARY(Not, "!(", ")")
//時相演算子
DEFINE_INFIX_VISIT_UNARY(Always, "[](", ")")
// Tell制約
DEFINE_INFIX_VISIT_UNARY(Tell, "", "")
// 制約式
DEFINE_INFIX_VISIT_UNARY(Constraint, "", ".")

//並列合成
void TreeInfixPrinter::visit(boost::shared_ptr<hydla::parse_tree::Parallel> node){
  print_binary_node(*node, ",");
}


// 算術二項演算子
void TreeInfixPrinter::visit(boost::shared_ptr<hydla::parse_tree::Plus> node){
  if(need_par_>=PAR_N_P_S){
    (*output_stream_) << "(";
     print_binary_node(*node, "+", PAR_NONE, PAR_N);
    (*output_stream_) << ")";
  }else{
     print_binary_node(*node, "+", PAR_NONE, PAR_N);
  }
}
void TreeInfixPrinter::visit(boost::shared_ptr<hydla::parse_tree::Subtract> node){
  if(need_par_>=PAR_N_P_S){
    (*output_stream_) << "(";
     print_binary_node(*node, "-", PAR_NONE, PAR_N_P_S);
    (*output_stream_) << ")";
  }else{
     print_binary_node(*node, "-", PAR_NONE, PAR_N_P_S);
  }
}
void TreeInfixPrinter::visit(boost::shared_ptr<hydla::parse_tree::Times> node){
  if(need_par_>=PAR_N_P_S_T_D_P){
    (*output_stream_) << "(";
     print_binary_node(*node, "*", PAR_N_P_S, PAR_N_P_S);
    (*output_stream_) << ")";
  }else{
     print_binary_node(*node, "*", PAR_N_P_S, PAR_N_P_S);
  }
}
void TreeInfixPrinter::visit(boost::shared_ptr<hydla::parse_tree::Divide> node){
  if(need_par_>=PAR_N_P_S_T_D_P){
    (*output_stream_) << "(";
     print_binary_node(*node, "/", PAR_N_P_S, PAR_N_P_S);
    (*output_stream_) << ")";
  }else{
     print_binary_node(*node, "/", PAR_N_P_S, PAR_N_P_S);
  }
}
void TreeInfixPrinter::visit(boost::shared_ptr<hydla::parse_tree::Power> node){
  if(need_par_>=PAR_N_P_S_T_D_P){
    (*output_stream_) << "(";
     print_binary_node(*node, "^", PAR_N_P_S_T_D_P, PAR_N_P_S_T_D_P);
    (*output_stream_) << ")";
  }else{
     print_binary_node(*node, "^", PAR_N_P_S_T_D_P, PAR_N_P_S_T_D_P);
  }
}

// 算術単項演算子"-"
void TreeInfixPrinter::visit(boost::shared_ptr<hydla::parse_tree::Negative> node){
  if(need_par_>=PAR_N){
    (*output_stream_) << "(";
    need_par_ = PAR_N_P_S_T_D_P;
    print_unary_node(*node, "-", "");
    (*output_stream_) << ")";
  }else{
    print_unary_node(*node, "-", "");
  }
}
// 変数
void TreeInfixPrinter::visit(boost::shared_ptr<hydla::parse_tree::Variable> node){
  (*output_stream_) << node->get_name();
}

// 数字
void TreeInfixPrinter::visit(boost::shared_ptr<hydla::parse_tree::Number> node){
  (*output_stream_) << node->get_number();
}

// 記号定数
void TreeInfixPrinter::visit(boost::shared_ptr<hydla::parse_tree::Parameter> node){
  (*output_stream_) << "p" << node->get_name();
}

// t
void TreeInfixPrinter::visit(boost::shared_ptr<hydla::parse_tree::SymbolicT> node){
  (*output_stream_) << "t";
}


// 制約定義
void TreeInfixPrinter::visit(boost::shared_ptr<hydla::parse_tree::ConstraintDefinition> node){
  (*output_stream_) << node->get_name() << "(";
  hydla::parse_tree::Definition::bound_variables_iterator it = node->bound_variable_begin();
  hydla::parse_tree::Definition::bound_variables_iterator end = node->bound_variable_end();
  if(it != end){
    while(1){
      (*output_stream_) << *it;
      if(++it==end)break;
      (*output_stream_) << ", ";
    }
  }
  (*output_stream_) << ") ";
  if(node->get_child())print_unary_node(*node, "<=>", "");
}
  
// プログラム定義
void TreeInfixPrinter::visit(boost::shared_ptr<hydla::parse_tree::ProgramDefinition> node){
  (*output_stream_) << node->get_name() << "(";
  hydla::parse_tree::Definition::bound_variables_iterator it = node->bound_variable_begin();
  hydla::parse_tree::Definition::bound_variables_iterator end = node->bound_variable_end();
  if(it != end){
    while(1){
      (*output_stream_) << *it;
      if(++it==end)break;
      (*output_stream_) << ", ";
    }
  }
  (*output_stream_) << ") ";
  if(node->get_child()) print_unary_node(*node, "<=>", "");
}


// 制約呼び出し
void TreeInfixPrinter::visit(boost::shared_ptr<hydla::parse_tree::ConstraintCaller> node){
  (*output_stream_) << node->get_name() << "(";
  hydla::parse_tree::Caller::actual_args_iterator it = node->actual_arg_begin();
  hydla::parse_tree::Caller::actual_args_iterator end = node->actual_arg_end();
  if(it != end){
    while(1){
      accept(*it);
      if(++it==end)break;
      (*output_stream_) << ", ";
    }
  }
  (*output_stream_) << ") ";
  if(node->get_child()) print_unary_node(*node, "{", "}");
}

// プログラム呼び出し
void TreeInfixPrinter::visit(boost::shared_ptr<hydla::parse_tree::ProgramCaller> node){
  (*output_stream_) << node->get_name() << "(";
  hydla::parse_tree::Caller::actual_args_iterator it = node->actual_arg_begin();
  hydla::parse_tree::Caller::actual_args_iterator end = node->actual_arg_end();
  if(it != end){
    while(1){
      accept(*it);
      if(++it==end)break;
      (*output_stream_) << ", ";
    }
  }
  (*output_stream_) << ")";
  if(node->get_child())print_unary_node(*node, "{", "}");
}


} // namespace symbolic_simulator
} // namespace hydla
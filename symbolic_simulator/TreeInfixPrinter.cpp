#include "TreeInfixPrinter.h"

namespace hydla {
namespace symbolic_simulator{


//value‚Æ‚Á‚Ä•¶š—ñ‚É•ÏŠ·‚·‚é
std::ostream& TreeInfixPrinter::print_infix(const TreeInfixPrinter::node_sptr & node, std::ostream& s){
  need_par_ = PAR_NONE;
  output_stream_ = &s;
  accept(node);
  return s;
}


void TreeInfixPrinter::print_binary_node(const hydla::parse_tree::BinaryNode &node, const std::string &symbol, const needParenthesis &pre_par, const needParenthesis &post_par){
  need_par_ = pre_par;
  accept(node.get_lhs());
  (*output_stream_) << symbol;
  need_par_ = post_par;
  accept(node.get_rhs());
}
void TreeInfixPrinter::print_unary_node(const hydla::parse_tree::UnaryNode &node, const std::string &pre, const std::string &post){
  (*output_stream_) << pre;
  accept(node.get_child());
  (*output_stream_) << post;
}


// ”äŠr‰‰Zq
void TreeInfixPrinter::visit(boost::shared_ptr<hydla::parse_tree::Equal> node){
  print_binary_node(*node, "=");
}
void TreeInfixPrinter::visit(boost::shared_ptr<hydla::parse_tree::UnEqual> node){
  print_binary_node(*node, "!=");
}
void TreeInfixPrinter::visit(boost::shared_ptr<hydla::parse_tree::Less> node){
  print_binary_node(*node, "<");
}
void TreeInfixPrinter::visit(boost::shared_ptr<hydla::parse_tree::LessEqual> node){
  print_binary_node(*node, "<=");
}
void TreeInfixPrinter::visit(boost::shared_ptr<hydla::parse_tree::Greater> node){
  print_binary_node(*node, ">");
}
void TreeInfixPrinter::visit(boost::shared_ptr<hydla::parse_tree::GreaterEqual> node){
  print_binary_node(*node, ">=");
}

// ˜_—‰‰Zq
void TreeInfixPrinter::visit(boost::shared_ptr<hydla::parse_tree::LogicalAnd> node){
  print_binary_node(*node, "&");
}
void TreeInfixPrinter::visit(boost::shared_ptr<hydla::parse_tree::LogicalOr> node){
  print_binary_node(*node, "|");
}

// Zp“ñ€‰‰Zq
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
     print_binary_node(*node, "-", PAR_NONE, PAR_N);
    (*output_stream_) << ")";
  }else{
     print_binary_node(*node, "-", PAR_NONE, PAR_N);
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

// Zp’P€‰‰Zq
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
void TreeInfixPrinter::visit(boost::shared_ptr<hydla::parse_tree::Positive> node){
  print_unary_node(*node, "", "");
}

// ”÷•ª
void TreeInfixPrinter::visit(boost::shared_ptr<hydla::parse_tree::Differential> node){
  print_unary_node(*node, "'", "");
}

// ¶‹ÉŒÀ
void TreeInfixPrinter::visit(boost::shared_ptr<hydla::parse_tree::Previous> node){
  print_unary_node(*node, "", "-");
}

// •Ï”
void TreeInfixPrinter::visit(boost::shared_ptr<hydla::parse_tree::Variable> node){
  (*output_stream_) << node->get_name();
}

// ”š
void TreeInfixPrinter::visit(boost::shared_ptr<hydla::parse_tree::Number> node){
  (*output_stream_) << node->get_number();
}

// ‹L†’è”
void TreeInfixPrinter::visit(boost::shared_ptr<hydla::parse_tree::Parameter> node){
  (*output_stream_) << "p" << node->get_name();
}

// t
void TreeInfixPrinter::visit(boost::shared_ptr<hydla::parse_tree::SymbolicT> node){
  (*output_stream_) << "t";
}

} // namespace symbolic_simulator
} // namespace hydla
#include "TreeInfixPrinter.h"

namespace hydla {
namespace parse_tree{


//value‚Æ‚Á‚Ä•¶š—ñ‚É•ÏŠ·‚·‚é
std::ostream& TreeInfixPrinter::print_infix(const node_sptr& node, std::ostream& s){
  need_par_ = PAR_NONE;
  output_stream_ = &s;
  //s << "print_infix\n";
  accept(node);
  return s;
}


void TreeInfixPrinter::print_binary_node(const BinaryNode &node, const std::string &symbol, const needParenthesis &pre_par, const needParenthesis &post_par){
  //(*output_stream_) << "binary_node\n";
  need_par_ = pre_par;
  accept(node.get_lhs());
  (*output_stream_) << symbol;
  need_par_ = post_par;
  accept(node.get_rhs());
}
void TreeInfixPrinter::print_unary_node(const UnaryNode &node, const std::string &pre, const std::string &post){
  //(*output_stream_) << "unary_node\n";
  (*output_stream_) << pre;
  accept(node.get_child());
  (*output_stream_) << post;
}

#define DEFINE_INFIX_VISIT_BINARY(NAME, SYMBOL)  \
void TreeInfixPrinter::visit(boost::shared_ptr<NAME> node){\
  print_binary_node(*node, #SYMBOL);\
}



//ã‡¬
DEFINE_INFIX_VISIT_BINARY(Weaker, <<)

// ”äŠr‰‰Zq
DEFINE_INFIX_VISIT_BINARY(Equal, =)
DEFINE_INFIX_VISIT_BINARY(UnEqual, !=)
DEFINE_INFIX_VISIT_BINARY(Less, <)
DEFINE_INFIX_VISIT_BINARY(LessEqual, <=)
DEFINE_INFIX_VISIT_BINARY(Greater, >)
DEFINE_INFIX_VISIT_BINARY(GreaterEqual, >=)
// ˜_—‰‰Zq
DEFINE_INFIX_VISIT_BINARY(LogicalAnd, &)

void TreeInfixPrinter::visit(boost::shared_ptr<LogicalOr> node){
  (*output_stream_) << "(";
  print_binary_node(*node, "|");
  (*output_stream_) << ")";
}

  // Ask§–ñ
DEFINE_INFIX_VISIT_BINARY(Ask, =>)


#define DEFINE_INFIX_VISIT_UNARY(NAME, PRE, POST)  \
void TreeInfixPrinter::visit(boost::shared_ptr<NAME> node){\
  print_unary_node(*node, PRE, POST);\
}

//’P€‰‰Zq "+"
DEFINE_INFIX_VISIT_UNARY(Positive, "", "'")

//”÷•ª
DEFINE_INFIX_VISIT_UNARY(Differential, "", "'")
//¶‹ÉŒÀ
DEFINE_INFIX_VISIT_UNARY(Previous, "", "-")
//”Û’è
DEFINE_INFIX_VISIT_UNARY(Not, "!(", ")")
//‘Š‰‰Zq
DEFINE_INFIX_VISIT_UNARY(Always, "[](", ")")
// Tell§–ñ
DEFINE_INFIX_VISIT_UNARY(Tell, "", "")
// §–ñ®
DEFINE_INFIX_VISIT_UNARY(Constraint, "", ".\n")
  
  
// ‘Î”
DEFINE_INFIX_VISIT_UNARY(Ln, "Ln(", ")")
void TreeInfixPrinter::visit(boost::shared_ptr<Log> node){
  
  (*output_stream_) << "Log(";
  print_binary_node(*node, ",");
  (*output_stream_) << ")";
}


// OŠpŠÖ”
DEFINE_INFIX_VISIT_UNARY(Sin, "Sin(", ")")
DEFINE_INFIX_VISIT_UNARY(Cos, "Cos(", ")")

//•À—ñ‡¬
void TreeInfixPrinter::visit(boost::shared_ptr<Parallel> node){
  print_binary_node(*node, ",");
}


// Zp“ñ€‰‰Zq
void TreeInfixPrinter::visit(boost::shared_ptr<Plus> node){
  if(need_par_>=PAR_N_P_S){
    (*output_stream_) << "(";
     print_binary_node(*node, "+", PAR_NONE, PAR_N);
    (*output_stream_) << ")";
  }else{
     print_binary_node(*node, "+", PAR_NONE, PAR_N);
  }
}
void TreeInfixPrinter::visit(boost::shared_ptr<Subtract> node){
  if(need_par_>=PAR_N_P_S){
    (*output_stream_) << "(";
     print_binary_node(*node, "-", PAR_NONE, PAR_N_P_S);
    (*output_stream_) << ")";
  }else{
     print_binary_node(*node, "-", PAR_NONE, PAR_N_P_S);
  }
}
void TreeInfixPrinter::visit(boost::shared_ptr<Times> node){
  if(need_par_>=PAR_N_P_S_T_D_P){
    (*output_stream_) << "(";
     print_binary_node(*node, "*", PAR_N_P_S, PAR_N_P_S);
    (*output_stream_) << ")";
  }else{
     print_binary_node(*node, "*", PAR_N_P_S, PAR_N_P_S);
  }
}
void TreeInfixPrinter::visit(boost::shared_ptr<Divide> node){
  if(need_par_>=PAR_N_P_S_T_D_P){
    (*output_stream_) << "(";
     print_binary_node(*node, "/", PAR_N_P_S, PAR_N_P_S);
    (*output_stream_) << ")";
  }else{
     print_binary_node(*node, "/", PAR_N_P_S, PAR_N_P_S);
  }
}
void TreeInfixPrinter::visit(boost::shared_ptr<Power> node){
  if(need_par_>=PAR_N_P_S_T_D_P){
    (*output_stream_) << "(";
     print_binary_node(*node, "^", PAR_N_P_S_T_D_P, PAR_N_P_S_T_D_P);
    (*output_stream_) << ")";
  }else{
     print_binary_node(*node, "^", PAR_N_P_S_T_D_P, PAR_N_P_S_T_D_P);
  }
}

// Zp’P€‰‰Zq"-"
void TreeInfixPrinter::visit(boost::shared_ptr<Negative> node){
  if(need_par_>=PAR_N){
    (*output_stream_) << "(";
    need_par_ = PAR_N_P_S_T_D_P;
    print_unary_node(*node, "-", "");
    (*output_stream_) << ")";
  }else{
    print_unary_node(*node, "-", "");
  }
}
// •Ï”
void TreeInfixPrinter::visit(boost::shared_ptr<Variable> node){
  (*output_stream_) << node->get_name();
}

// ”š
void TreeInfixPrinter::visit(boost::shared_ptr<Number> node){
  (*output_stream_) << node->get_number();
}

// ‹L†’è”
void TreeInfixPrinter::visit(boost::shared_ptr<Parameter> node){
  (*output_stream_) << "p" << node->get_name();
}

// t
void TreeInfixPrinter::visit(boost::shared_ptr<SymbolicT> node){
  (*output_stream_) << "t";
}


// ”CˆÓ
void TreeInfixPrinter::visit(boost::shared_ptr<ArbitraryFactor> node){
  (*output_stream_) << node->get_string();
}

void TreeInfixPrinter::visit(boost::shared_ptr<ArbitraryUnary> node){
  (*output_stream_) << node->get_string() << "(";
  accept(node->get_child());
  (*output_stream_) << ")";
}

void TreeInfixPrinter::visit(boost::shared_ptr<ArbitraryBinary> node){
  (*output_stream_) << node->get_string() << "(";
  accept(node->get_lhs());
  (*output_stream_) << ",";
  accept(node->get_rhs());
  (*output_stream_) << ")";
}


void TreeInfixPrinter::visit(boost::shared_ptr<ArbitraryNode> node){
  (*output_stream_) << node->get_string() << "(";
  int i=0;
  while(node->arguments_.size()){
    accept(node->arguments_[i]);
    if(++i >= node->arguments_.size())break;
    (*output_stream_) << ",";
  }
  (*output_stream_) << ")";
}

// ©‘R‘Î”‚Ì’ê
void TreeInfixPrinter::visit(boost::shared_ptr<E> node){
  (*output_stream_) << "E";
}


// ‰~ü—¦
void TreeInfixPrinter::visit(boost::shared_ptr<Pi> node){
  (*output_stream_) << "Pi";
}



// §–ñ’è‹`
void TreeInfixPrinter::visit(boost::shared_ptr<ConstraintDefinition> node){
  (*output_stream_) << node->get_name() << "(";
  Definition::bound_variables_iterator it = node->bound_variable_begin();
  Definition::bound_variables_iterator end = node->bound_variable_end();
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
  
// ƒvƒƒOƒ‰ƒ€’è‹`
void TreeInfixPrinter::visit(boost::shared_ptr<ProgramDefinition> node){
  (*output_stream_) << node->get_name() << "(";
  Definition::bound_variables_iterator it = node->bound_variable_begin();
  Definition::bound_variables_iterator end = node->bound_variable_end();
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


// §–ñŒÄ‚Ño‚µ
void TreeInfixPrinter::visit(boost::shared_ptr<ConstraintCaller> node){
  (*output_stream_) << node->get_name() << "(";
  Caller::actual_args_iterator it = node->actual_arg_begin();
  Caller::actual_args_iterator end = node->actual_arg_end();
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

// ƒvƒƒOƒ‰ƒ€ŒÄ‚Ño‚µ
void TreeInfixPrinter::visit(boost::shared_ptr<ProgramCaller> node){
  (*output_stream_) << node->get_name() << "(";
  Caller::actual_args_iterator it = node->actual_arg_begin();
  Caller::actual_args_iterator end = node->actual_arg_end();
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


} // namespace parse_tree
} // namespace hydla
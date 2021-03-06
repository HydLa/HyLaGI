#include "TreeInfixPrinter.h"
#include <iomanip>

namespace hydla {
namespace symbolic_expression {

using namespace std;

bool TreeInfixPrinter::use_shorthand = false;

// valueとって文字列に変換する
ostream &TreeInfixPrinter::print_infix(const node_sptr &node, ostream &s) {
  need_par_ = PAR_NONE;
  output_stream_ = &s;
  accept(node);
  return s;
}

string TreeInfixPrinter::get_infix_string(
    const hydla::symbolic_expression::node_sptr &node) {
  need_par_ = PAR_NONE;
  stringstream sstr;
  output_stream_ = &sstr;
  accept(node);
  return sstr.str();
}

void TreeInfixPrinter::print_binary_node(const BinaryNode &node,
                                         const string &symbol,
                                         const needParenthesis &pre_par,
                                         const needParenthesis &post_par) {
  need_par_ = pre_par;
  accept(node.get_lhs());
  (*output_stream_) << symbol;
  need_par_ = post_par;
  accept(node.get_rhs());
}
void TreeInfixPrinter::print_unary_node(const UnaryNode &node,
                                        const string &pre, const string &post) {
  (*output_stream_) << pre;
  accept(node.get_child());
  (*output_stream_) << post;
}

void TreeInfixPrinter::print_factor_node(const FactorNode &node,
                                         const string &pre,
                                         const string &post) {
  (*output_stream_) << pre;
  (*output_stream_) << post;
}
#define DEFINE_INFIX_VISIT_FACTOR(NAME, PRE, POST)                             \
  void TreeInfixPrinter::visit(std::shared_ptr<NAME> node) {                   \
    print_factor_node(*node, PRE, POST);                                       \
  }

#define DEFINE_INFIX_VISIT_BINARY(NAME, SYMBOL)                                \
  void TreeInfixPrinter::visit(std::shared_ptr<NAME> node) {                   \
    print_binary_node(*node, #SYMBOL);                                         \
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

void TreeInfixPrinter::visit(std::shared_ptr<LogicalOr> node) {
  (*output_stream_) << "(";
  print_binary_node(*node, "|");
  (*output_stream_) << ")";
}

// Ask制約
DEFINE_INFIX_VISIT_BINARY(Ask, =>)

void TreeInfixPrinter::visit(std::shared_ptr<Exists> node) {
  (*output_stream_) << "\\";
  print_binary_node(*node, ".");
}

#define DEFINE_INFIX_VISIT_UNARY(NAME, PRE, POST)                              \
  void TreeInfixPrinter::visit(std::shared_ptr<NAME> node) {                   \
    print_unary_node(*node, PRE, POST);                                        \
  }

//単項演算子 "+"
DEFINE_INFIX_VISIT_UNARY(Positive, "+", "")

//微分
DEFINE_INFIX_VISIT_UNARY(Differential, "", "\'")
//左極限
DEFINE_INFIX_VISIT_UNARY(Previous, "", "-")
//否定
DEFINE_INFIX_VISIT_UNARY(Not, "!(", ")")
//時相演算子
DEFINE_INFIX_VISIT_UNARY(Always, "[](", ")")
// Tell制約
DEFINE_INFIX_VISIT_UNARY(Tell, "", "")
// 制約式
DEFINE_INFIX_VISIT_UNARY(Constraint, "", ".\n")

// Print
DEFINE_INFIX_VISIT_FACTOR(Print, "Print(", ")")
DEFINE_INFIX_VISIT_FACTOR(PrintPP, "PrintPP(", ")")
DEFINE_INFIX_VISIT_FACTOR(PrintIP, "PrintIP(", ")")
DEFINE_INFIX_VISIT_FACTOR(Scan, "Scan(", ")")
DEFINE_INFIX_VISIT_FACTOR(Exit, "Exit(", ")")
DEFINE_INFIX_VISIT_FACTOR(Abort, "Abort(", ")")

// True
DEFINE_INFIX_VISIT_FACTOR(True, "True", "")

// False
DEFINE_INFIX_VISIT_FACTOR(False, "False", "")

void TreeInfixPrinter::visit(std::shared_ptr<SVtimer> node) {
  (*output_stream_) << "$timer";
}
//並列合成
void TreeInfixPrinter::visit(std::shared_ptr<Parallel> node) {
  print_binary_node(*node, ",");
}

//式リスト要素
void TreeInfixPrinter::visit(std::shared_ptr<ExpressionListElement> node) {
  std::shared_ptr<ExpressionList> el =
      std::dynamic_pointer_cast<ExpressionList>(node->get_lhs());
  if (el)
    (*output_stream_) << el->get_name();
  (*output_stream_) << "[";
  accept(node->get_rhs());
  (*output_stream_) << "]";
}

// 算術二項演算子
void TreeInfixPrinter::visit(std::shared_ptr<Plus> node) {
  if (need_par_ >= PAR_N_P_S || need_par_ == PAR_P_S) {
    (*output_stream_) << "(";
    print_binary_node(*node, "+", PAR_NONE, PAR_N);
    (*output_stream_) << ")";
  } else {
    print_binary_node(*node, "+", PAR_NONE, PAR_N);
  }
}
void TreeInfixPrinter::visit(std::shared_ptr<Subtract> node) {
  if (need_par_ >= PAR_N_P_S || need_par_ == PAR_P_S) {
    (*output_stream_) << "(";
    print_binary_node(*node, "-", PAR_NONE, PAR_N_P_S);
    (*output_stream_) << ")";
  } else {
    print_binary_node(*node, "-", PAR_NONE, PAR_N_P_S);
  }
}
void TreeInfixPrinter::visit(std::shared_ptr<Times> node) {
  if (need_par_ >= PAR_N_P_S_T_D_P) {
    (*output_stream_) << "(";
    print_binary_node(*node, "*", PAR_P_S, PAR_N_P_S);
    (*output_stream_) << ")";
  } else {
    needParenthesis pre = need_par_ >= PAR_N ? PAR_N_P_S : PAR_P_S;
    print_binary_node(*node, "*", pre, PAR_N_P_S);
  }
}
void TreeInfixPrinter::visit(std::shared_ptr<Divide> node) {
  if (need_par_ >= PAR_N_P_S_T_D_P) {
    (*output_stream_) << "(";
    print_binary_node(*node, "/", PAR_P_S, PAR_N_P_S);
    (*output_stream_) << ")";
  } else {
    needParenthesis pre = need_par_ >= PAR_N ? PAR_N_P_S : PAR_P_S;
    print_binary_node(*node, "/", pre, PAR_N_P_S);
  }
}
void TreeInfixPrinter::visit(std::shared_ptr<Power> node) {
  if (need_par_ >= PAR_N_P_S_T_D_P) {
    (*output_stream_) << "(";
    print_binary_node(*node, "^", PAR_N_P_S_T_D_P, PAR_N_P_S_T_D_P);
    (*output_stream_) << ")";
  } else {
    print_binary_node(*node, "^", PAR_N_P_S_T_D_P, PAR_N_P_S_T_D_P);
  }
}

// 算術単項演算子"-"
void TreeInfixPrinter::visit(std::shared_ptr<Negative> node) {
  if (need_par_ >= PAR_N) {
    (*output_stream_) << "(";
    need_par_ = PAR_N_P_S_T_D_P;
    print_unary_node(*node, "-", "");
    (*output_stream_) << ")";
  } else {
    need_par_ = PAR_N_P_S_T_D_P;
    print_unary_node(*node, "-", "");
  }
}
// 変数
void TreeInfixPrinter::visit(std::shared_ptr<Variable> node) {
  (*output_stream_) << node->get_name();
}

// 数字
void TreeInfixPrinter::visit(std::shared_ptr<Number> node) {
  (*output_stream_) << node->get_number();
}

void TreeInfixPrinter::visit(std::shared_ptr<Float> node) {
  double number = node->get_number();
  if (number >= 0 || need_par_ < PAR_N) {
    (*output_stream_) << setprecision(16) << number;
  } else {
    (*output_stream_) << "(";
    (*output_stream_) << setprecision(16) << number;
    (*output_stream_) << ")";
  }
}

// 記号定数
void TreeInfixPrinter::visit(std::shared_ptr<Parameter> node) {
  if (use_shorthand) {
    (*output_stream_) << node->get_name() << node->get_differential_count()
                      << node->get_phase_id();
  } else {
    (*output_stream_) << "p[" << node->get_name() << ", "
                      << node->get_differential_count() << ", "
                      << node->get_phase_id() << "]";
  }
}

// t
void TreeInfixPrinter::visit(std::shared_ptr<SymbolicT> node) {
  (*output_stream_) << "t";
}

void TreeInfixPrinter::visit(std::shared_ptr<ImaginaryUnit> node) {
  (*output_stream_) << "I";
}

//関数
void TreeInfixPrinter::visit(std::shared_ptr<Function> node) {
  (*output_stream_) << node->get_name() << "[";
  int i = 0;
  while (true) {
    accept(node->get_argument(i));
    if (++i >= node->get_arguments_size())
      break;
    (*output_stream_) << ",";
  }
  (*output_stream_) << "]";
}

void TreeInfixPrinter::visit(std::shared_ptr<UnsupportedFunction> node) {
  (*output_stream_) << node->get_name() << "[";
  int i = 0;
  while (true) {

    accept(node->get_argument(i));
    if (++i >= node->get_arguments_size())
      break;
    (*output_stream_) << ",";
  }
  (*output_stream_) << "]";
}

void TreeInfixPrinter::visit(std::shared_ptr<E> node) {
  (*output_stream_) << "E";
}

void TreeInfixPrinter::visit(std::shared_ptr<Pi> node) {
  (*output_stream_) << "Pi";
}

void TreeInfixPrinter::visit(std::shared_ptr<Infinity> node) {
  (*output_stream_) << "Infinity";
}

void TreeInfixPrinter::visit(std::shared_ptr<ConstraintDefinition> node) {
  (*output_stream_) << node->get_name() << "(";
  Definition::bound_variables_iterator it = node->bound_variable_begin();
  Definition::bound_variables_iterator end = node->bound_variable_end();
  if (it != end) {
    while (1) {
      (*output_stream_) << *it;
      if (++it == end)
        break;
      (*output_stream_) << ", ";
    }
  }
  (*output_stream_) << ") ";
  if (node->get_child())
    print_unary_node(*node, "<=>", "");
}

// プログラム定義
void TreeInfixPrinter::visit(std::shared_ptr<ProgramDefinition> node) {
  (*output_stream_) << node->get_name() << "(";
  Definition::bound_variables_iterator it = node->bound_variable_begin();
  Definition::bound_variables_iterator end = node->bound_variable_end();
  if (it != end) {
    while (1) {
      (*output_stream_) << *it;
      if (++it == end)
        break;
      (*output_stream_) << ", ";
    }
  }
  (*output_stream_) << ") ";
  if (node->get_child())
    print_unary_node(*node, "<=>", "");
}

// 制約呼び出し
void TreeInfixPrinter::visit(std::shared_ptr<ConstraintCaller> node) {
  (*output_stream_) << node->get_name() << "(";
  Caller::actual_args_iterator it = node->actual_arg_begin();
  Caller::actual_args_iterator end = node->actual_arg_end();
  if (it != end) {
    while (1) {
      accept(*it);
      if (++it == end)
        break;
      (*output_stream_) << ", ";
    }
  }
  (*output_stream_) << ")";
  // if (node->get_child()) print_unary_node(*node, "{", "}");
}

// プログラム呼び出し
void TreeInfixPrinter::visit(std::shared_ptr<ProgramCaller> node) {
  (*output_stream_) << node->get_name() << "(";
  Caller::actual_args_iterator it = node->actual_arg_begin();
  Caller::actual_args_iterator end = node->actual_arg_end();
  if (it != end) {
    while (1) {
      accept(*it);
      if (++it == end)
        break;
      (*output_stream_) << ", ";
    }
  }
  (*output_stream_) << ")";
  // if (node->get_child())print_unary_node(*node, "{", "}");
}

void TreeInfixPrinter::set_use_shorthand(bool u) { use_shorthand = u; }

} // namespace symbolic_expression
} // namespace hydla

#include "ParseTreeGraphvizDumper.h"

using namespace hydla::symbolic_expression;
using namespace hydla::parser::error;
using namespace std;

using std::make_pair;

namespace hydla {
namespace parser {

ParseTreeGraphvizDumper::ParseTreeGraphvizDumper() {}

ParseTreeGraphvizDumper::~ParseTreeGraphvizDumper() {}

namespace {

struct NodeDumper {
  NodeDumper(std::ostream &s) : s_(s) {}

  template <typename T> void operator()(const T &n) {
    s_ << "  "
       << "n" << n.first << " [label = \"" << n.second << "\"];\n";
  }

  std::ostream &s_;
};

struct EdgeDumper {
  EdgeDumper(std::ostream &s) : s_(s) {}

  template <typename T> void operator()(const T &n) {
    s_ << "  "
       << "n" << n.first << " -> "
       << "n" << n.second << ";\n";
  }

  std::ostream &s_;
};
} // namespace

std::ostream &
ParseTreeGraphvizDumper::dump(std::ostream &s,
                              const symbolic_expression::node_sptr &node) {
  node_id_ = 0;
  nodes_.clear();
  edges_.clear();

  accept(node);

  s << "digraph g {\n";
  s << "  node [shape = record];\n";
  s << "\n";

  std::for_each(nodes_.begin(), nodes_.end(), NodeDumper(s));
  s << "\n";

  std::for_each(edges_.begin(), edges_.end(), EdgeDumper(s));
  s << "}" << std::endl;

  return s;
}

void ParseTreeGraphvizDumper::dump_node(std::shared_ptr<symbolic_expression::FactorNode> node) {
  node_id_t own_id = node_id_;
  nodes_.insert(make_pair(own_id, node->get_node_type_name()));
}

void ParseTreeGraphvizDumper::dump_node(std::shared_ptr<symbolic_expression::UnaryNode> node) {
  node_id_t own_id = node_id_;
  nodes_.insert(make_pair(own_id, node->get_node_type_name()));

  edges_.insert(make_pair(own_id, ++node_id_));
  accept(node->get_child());
}

void ParseTreeGraphvizDumper::dump_node(std::shared_ptr<symbolic_expression::BinaryNode> node) {
  node_id_t own_id = node_id_;
  nodes_.insert(make_pair(own_id, node->get_node_type_name()));

  edges_.insert(make_pair(own_id, ++node_id_));
  accept(node->get_lhs());

  edges_.insert(make_pair(own_id, ++node_id_));
  accept(node->get_rhs());
}

void ParseTreeGraphvizDumper::dump_node(std::shared_ptr<symbolic_expression::VariadicNode> node) {
  node_id_t own_id = node_id_;
  nodes_.insert(make_pair(own_id, node->get_string()));

  for (int i = 0; i < node->get_arguments_size(); i++) {
    edges_.insert(make_pair(own_id, ++node_id_));
    accept(node->get_argument(i));
  }
}

// 制約定義
void ParseTreeGraphvizDumper::visit(
    std::shared_ptr<symbolic_expression::ConstraintDefinition> node) {
  dump_node(node);
}

// プログラム定義
void ParseTreeGraphvizDumper::visit(std::shared_ptr<symbolic_expression::ProgramDefinition> node) {
  dump_node(node);
}

// 制約呼び出し
void ParseTreeGraphvizDumper::visit(std::shared_ptr<symbolic_expression::ConstraintCaller> node) {
  dump_node(node);
}

// プログラム呼び出し
void ParseTreeGraphvizDumper::visit(std::shared_ptr<symbolic_expression::ProgramCaller> node) {
  dump_node(node);
}

// 制約式
void ParseTreeGraphvizDumper::visit(std::shared_ptr<symbolic_expression::Constraint> node) {
  dump_node(node);
}

// Ask制約
void ParseTreeGraphvizDumper::visit(std::shared_ptr<symbolic_expression::Ask> node) {
  dump_node(node);
}

// Ask制約
void ParseTreeGraphvizDumper::visit(std::shared_ptr<symbolic_expression::Exists> node) {
  dump_node(node);
}

// Tell制約
void ParseTreeGraphvizDumper::visit(std::shared_ptr<symbolic_expression::Tell> node) {
  dump_node(node);
}

// 算術単項演算子
void ParseTreeGraphvizDumper::visit(std::shared_ptr<symbolic_expression::Negative> node) {
  dump_node(node);
}

void ParseTreeGraphvizDumper::visit(std::shared_ptr<symbolic_expression::Positive> node) {
  dump_node(node);
}

void ParseTreeGraphvizDumper::visit(std::shared_ptr<symbolic_expression::Equal> node) {
  dump_node(node);
}

void ParseTreeGraphvizDumper::visit(std::shared_ptr<symbolic_expression::UnEqual> node) {
  dump_node(node);
}

void ParseTreeGraphvizDumper::visit(std::shared_ptr<symbolic_expression::Less> node) {
  dump_node(node);
}

void ParseTreeGraphvizDumper::visit(std::shared_ptr<symbolic_expression::LessEqual> node) {
  dump_node(node);
}

void ParseTreeGraphvizDumper::visit(std::shared_ptr<symbolic_expression::Greater> node) {
  dump_node(node);
}

void ParseTreeGraphvizDumper::visit(std::shared_ptr<symbolic_expression::GreaterEqual> node) {
  dump_node(node);
}

void ParseTreeGraphvizDumper::visit(std::shared_ptr<symbolic_expression::Plus> node) {
  dump_node(node);
}

void ParseTreeGraphvizDumper::visit(std::shared_ptr<symbolic_expression::Subtract> node) {
  dump_node(node);
}

void ParseTreeGraphvizDumper::visit(std::shared_ptr<symbolic_expression::Times> node) {
  dump_node(node);
}

void ParseTreeGraphvizDumper::visit(std::shared_ptr<symbolic_expression::Divide> node) {
  dump_node(node);
}

void ParseTreeGraphvizDumper::visit(std::shared_ptr<symbolic_expression::Power> node) {
  dump_node(node);
}

void ParseTreeGraphvizDumper::visit(std::shared_ptr<symbolic_expression::LogicalAnd> node) {
  dump_node(node);
}

void ParseTreeGraphvizDumper::visit(std::shared_ptr<symbolic_expression::LogicalOr> node) {
  dump_node(node);
}

void ParseTreeGraphvizDumper::visit(std::shared_ptr<symbolic_expression::Not> node) {
  dump_node(node);
}

void ParseTreeGraphvizDumper::visit(std::shared_ptr<symbolic_expression::Weaker> node) {
  dump_node(node);
}

void ParseTreeGraphvizDumper::visit(std::shared_ptr<symbolic_expression::Parallel> node) {
  dump_node(node);
}

// 時相演算子
void ParseTreeGraphvizDumper::visit(std::shared_ptr<symbolic_expression::Always> node) {
  dump_node(node);
}

// 微分
void ParseTreeGraphvizDumper::visit(std::shared_ptr<symbolic_expression::Differential> node) {
  dump_node(node);
}

// 左極限
void ParseTreeGraphvizDumper::visit(std::shared_ptr<symbolic_expression::Previous> node) {
  dump_node(node);
}

// Print
void ParseTreeGraphvizDumper::visit(std::shared_ptr<symbolic_expression::Print> node) {
  dump_node(node);
}
void ParseTreeGraphvizDumper::visit(std::shared_ptr<symbolic_expression::PrintPP> node) {
  dump_node(node);
}
void ParseTreeGraphvizDumper::visit(std::shared_ptr<symbolic_expression::PrintIP> node) {
  dump_node(node);
}
void ParseTreeGraphvizDumper::visit(std::shared_ptr<symbolic_expression::Scan> node) {
  dump_node(node);
}
void ParseTreeGraphvizDumper::visit(std::shared_ptr<symbolic_expression::Exit> node) {
  dump_node(node);
}
void ParseTreeGraphvizDumper::visit(std::shared_ptr<symbolic_expression::Abort> node) {
  dump_node(node);
}
// SystemVariable
void ParseTreeGraphvizDumper::visit(std::shared_ptr<symbolic_expression::SVtimer> node) {
  dump_node(node);
}
// 関数
void ParseTreeGraphvizDumper::visit(std::shared_ptr<symbolic_expression::Function> node) {
  dump_node(node);
}
void ParseTreeGraphvizDumper::visit(
    std::shared_ptr<symbolic_expression::UnsupportedFunction> node) {
  dump_node(node);
}

// 円周率
void ParseTreeGraphvizDumper::visit(std::shared_ptr<symbolic_expression::Pi> node) {
  dump_node(node);
}

// 自然対数の底
void ParseTreeGraphvizDumper::visit(std::shared_ptr<symbolic_expression::E> node) {
  dump_node(node);
}

// True
void ParseTreeGraphvizDumper::visit(std::shared_ptr<symbolic_expression::True> node) {
  dump_node(node);
}

// False
void ParseTreeGraphvizDumper::visit(std::shared_ptr<symbolic_expression::False> node) {
  dump_node(node);
}

// 変数
void ParseTreeGraphvizDumper::visit(std::shared_ptr<symbolic_expression::Variable> node) {
  node_id_t own_id = node_id_;
  nodes_.insert(make_pair(own_id, "{" + node->get_node_type_name() + " | " +
                                      node->get_name() + "}"));
}

// 数字
void ParseTreeGraphvizDumper::visit(std::shared_ptr<symbolic_expression::Number> node) {
  node_id_t own_id = node_id_;
  nodes_.insert(make_pair(own_id, "{" + node->get_node_type_name() + " | " +
                                      node->get_number() + "}"));
}

void ParseTreeGraphvizDumper::visit(std::shared_ptr<symbolic_expression::Float> node) {
  stringstream sstr;
  sstr << node->get_number();
  node_id_t own_id = node_id_;
  nodes_.insert(make_pair(own_id, "{" + node->get_node_type_name() + " | " +
                                      sstr.str() + "}"));
}

void ParseTreeGraphvizDumper::visit(std::shared_ptr<symbolic_expression::ImaginaryUnit> node) {
  dump_node(node);
}

void ParseTreeGraphvizDumper::visit(std::shared_ptr<symbolic_expression::Infinity> node) {
  dump_node(node);
}

void ParseTreeGraphvizDumper::visit(std::shared_ptr<symbolic_expression::Parameter> node) {

  node_id_t own_id = node_id_;
  stringstream sstr;
  sstr << "{" << node->get_node_type_name() << " | " << node->get_name() << ", "
       << node->get_differential_count() << ", " << node->get_phase_id() << "}";
  nodes_.insert(make_pair(own_id, sstr.str()));
}

void ParseTreeGraphvizDumper::visit(std::shared_ptr<symbolic_expression::SymbolicT> node) {
  dump_node(node);
}

// ExpressionList
void ParseTreeGraphvizDumper::visit(std::shared_ptr<symbolic_expression::ExpressionList> node) {
  dump_node(node);
}

// ConditionalExpressionList
void ParseTreeGraphvizDumper::visit(
    std::shared_ptr<symbolic_expression::ConditionalExpressionList> node) {
  dump_node(node);
}

// ProgramList
void ParseTreeGraphvizDumper::visit(std::shared_ptr<symbolic_expression::ProgramList> node) {
  dump_node(node);
}

// ConditionalProgramList
void ParseTreeGraphvizDumper::visit(
    std::shared_ptr<symbolic_expression::ConditionalProgramList> node) {
  dump_node(node);
}

// EachElement
void ParseTreeGraphvizDumper::visit(std::shared_ptr<symbolic_expression::EachElement> node) {
  dump_node(node);
}

// DifferentVariable
void ParseTreeGraphvizDumper::visit(std::shared_ptr<symbolic_expression::DifferentVariable> node) {
  dump_node(node);
}

// ExpressionListElement
void ParseTreeGraphvizDumper::visit(
    std::shared_ptr<symbolic_expression::ExpressionListElement> node) {
  dump_node(node);
}

// ExpressionListCaller
void ParseTreeGraphvizDumper::visit(
    std::shared_ptr<symbolic_expression::ExpressionListCaller> node) {
  dump_node(node);
}

// ProgramListElement
void ParseTreeGraphvizDumper::visit(
    std::shared_ptr<symbolic_expression::ProgramListElement> node) {
  dump_node(node);
}

// ExpressionListDefinition
void ParseTreeGraphvizDumper::visit(
    std::shared_ptr<symbolic_expression::ExpressionListDefinition> node) {
  dump_node(node);
}

// ProgramListDefinition
void ParseTreeGraphvizDumper::visit(
    std::shared_ptr<symbolic_expression::ProgramListDefinition> node) {
  dump_node(node);
}

// ProgramListCaller
void ParseTreeGraphvizDumper::visit(std::shared_ptr<symbolic_expression::ProgramListCaller> node) {
  dump_node(node);
}

// Union
void ParseTreeGraphvizDumper::visit(std::shared_ptr<symbolic_expression::Union> node) {
  dump_node(node);
}

// Intersection
void ParseTreeGraphvizDumper::visit(std::shared_ptr<symbolic_expression::Intersection> node) {
  dump_node(node);
}

// Range
void ParseTreeGraphvizDumper::visit(std::shared_ptr<symbolic_expression::Range> node) {
  dump_node(node);
}

// SizeOfList
void ParseTreeGraphvizDumper::visit(std::shared_ptr<symbolic_expression::SizeOfList> node) {
  dump_node(node);
}

// SumOfList
void ParseTreeGraphvizDumper::visit(std::shared_ptr<symbolic_expression::SumOfList> node) {
  dump_node(node);
}

// MulOfList
void ParseTreeGraphvizDumper::visit(std::shared_ptr<symbolic_expression::MulOfList> node) {
  dump_node(node);
}

} // namespace parser
} // namespace hydla

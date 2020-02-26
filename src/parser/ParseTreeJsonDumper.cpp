#include "ParseTreeJsonDumper.h"

using namespace hydla::symbolic_expression;
using namespace hydla::parser::error;
using namespace std;

using std::make_pair;
using namespace picojson;

namespace hydla {
namespace parser {

ParseTreeJsonDumper::ParseTreeJsonDumper() {}

ParseTreeJsonDumper::~ParseTreeJsonDumper() {}

std::ostream &
ParseTreeJsonDumper::dump(std::ostream &s,
                          const symbolic_expression::node_sptr &node) {
  // initialize json and current pointer
  json_ = value{object{}};
  current_ = &json_;

  // traverse parse tree
  accept(node);

  // output parse tree in JSON format
  s << json_.serialize() << endl;

  return s;
}

void ParseTreeJsonDumper::dump_node(std::shared_ptr<symbolic_expression::FactorNode> node) {
  // set the type of current node
  current_->get<object>()["type"] = value{node->get_node_type_name()};
}

void ParseTreeJsonDumper::dump_node(std::shared_ptr<symbolic_expression::UnaryNode> node) {
  // set the type of current node
  current_->get<object>()["type"] = value{node->get_node_type_name()};

  // create child node
  current_->get<object>()["child"] = value{object{}};
  current_ = &(current_->get<object>()["child"]);
  ;

  // visit child node
  accept(node->get_child());
}

void ParseTreeJsonDumper::dump_node(std::shared_ptr<symbolic_expression::BinaryNode> node) {
  // set the type of current node
  current_->get<object>()["type"] = value{node->get_node_type_name()};

  // create child nodes
  value *own = current_; // store the pointer to current node
  own->get<object>()["lhs"] = value{object{}};
  own->get<object>()["rhs"] = value{object{}};

  // visit lhs node
  current_ = &(own->get<object>()["lhs"]);
  ;
  accept(node->get_lhs());

  // visit rhs node
  current_ = &(own->get<object>()["rhs"]);
  ;
  accept(node->get_rhs());
}

void ParseTreeJsonDumper::dump_node(std::shared_ptr<symbolic_expression::VariadicNode> node) {
  // set the type of current node
  current_->get<object>()["type"] = value{node->get_node_type_name()};

  // create child nodes array
  current_->get<object>()["children"] = value{picojson::array{}};

  value *own = current_;

  // visit each child nodes
  int length = node->get_arguments_size();
  for (int i = 0; i < length; ++i) {
    own->get<object>()["children"].get(i) = value{object{}};
    current_ = &(own->get<object>()["children"].get(i));
    accept(node->get_argument(i));
  }
}

// 制約定義
void ParseTreeJsonDumper::visit(std::shared_ptr<symbolic_expression::ConstraintDefinition> node) {
  dump_node(node);
}

// プログラム定義
void ParseTreeJsonDumper::visit(std::shared_ptr<symbolic_expression::ProgramDefinition> node) {
  dump_node(node);
}

// 制約呼び出し
void ParseTreeJsonDumper::visit(std::shared_ptr<symbolic_expression::ConstraintCaller> node) {
  current_->get<object>()["name"] = value{node->get_name()};
  dump_node(node);
}

// プログラム呼び出し
void ParseTreeJsonDumper::visit(std::shared_ptr<symbolic_expression::ProgramCaller> node) {
  dump_node(node);
}

// 制約式
void ParseTreeJsonDumper::visit(std::shared_ptr<symbolic_expression::Constraint> node) {
  dump_node(node);
}

// Ask制約
void ParseTreeJsonDumper::visit(std::shared_ptr<symbolic_expression::Ask> node) {
  dump_node(node);
}

// Exists
void ParseTreeJsonDumper::visit(std::shared_ptr<symbolic_expression::Exists> node) {
  dump_node(node);
}

// Tell制約
void ParseTreeJsonDumper::visit(std::shared_ptr<symbolic_expression::Tell> node) {
  dump_node(node);
}

// 算術単項演算子
void ParseTreeJsonDumper::visit(std::shared_ptr<symbolic_expression::Negative> node) {
  dump_node(node);
}

void ParseTreeJsonDumper::visit(std::shared_ptr<symbolic_expression::Positive> node) {
  dump_node(node);
}

void ParseTreeJsonDumper::visit(std::shared_ptr<symbolic_expression::Equal> node) {
  dump_node(node);
}

void ParseTreeJsonDumper::visit(std::shared_ptr<symbolic_expression::UnEqual> node) {
  dump_node(node);
}

void ParseTreeJsonDumper::visit(std::shared_ptr<symbolic_expression::Less> node) {
  dump_node(node);
}

void ParseTreeJsonDumper::visit(std::shared_ptr<symbolic_expression::LessEqual> node) {
  dump_node(node);
}

void ParseTreeJsonDumper::visit(std::shared_ptr<symbolic_expression::Greater> node) {
  dump_node(node);
}

void ParseTreeJsonDumper::visit(std::shared_ptr<symbolic_expression::GreaterEqual> node) {
  dump_node(node);
}

void ParseTreeJsonDumper::visit(std::shared_ptr<symbolic_expression::Plus> node) {
  dump_node(node);
}

void ParseTreeJsonDumper::visit(std::shared_ptr<symbolic_expression::Subtract> node) {
  dump_node(node);
}

void ParseTreeJsonDumper::visit(std::shared_ptr<symbolic_expression::Times> node) {
  dump_node(node);
}

void ParseTreeJsonDumper::visit(std::shared_ptr<symbolic_expression::Divide> node) {
  dump_node(node);
}

void ParseTreeJsonDumper::visit(std::shared_ptr<symbolic_expression::Power> node) {
  dump_node(node);
}

void ParseTreeJsonDumper::visit(std::shared_ptr<symbolic_expression::LogicalAnd> node) {
  dump_node(node);
}

void ParseTreeJsonDumper::visit(std::shared_ptr<symbolic_expression::LogicalOr> node) {
  dump_node(node);
}

void ParseTreeJsonDumper::visit(std::shared_ptr<symbolic_expression::Not> node) {
  dump_node(node);
}

void ParseTreeJsonDumper::visit(std::shared_ptr<symbolic_expression::Weaker> node) {
  dump_node(node);
}

void ParseTreeJsonDumper::visit(std::shared_ptr<symbolic_expression::Parallel> node) {
  dump_node(node);
}

// 時相演算子
void ParseTreeJsonDumper::visit(std::shared_ptr<symbolic_expression::Always> node) {
  dump_node(node);
}

// 微分
void ParseTreeJsonDumper::visit(std::shared_ptr<symbolic_expression::Differential> node) {
  dump_node(node);
}

// 左極限
void ParseTreeJsonDumper::visit(std::shared_ptr<symbolic_expression::Previous> node) {
  dump_node(node);
}

// Print
void ParseTreeJsonDumper::visit(std::shared_ptr<symbolic_expression::Print> node) {
  dump_node(node);
}
void ParseTreeJsonDumper::visit(std::shared_ptr<symbolic_expression::PrintPP> node) {
  dump_node(node);
}
void ParseTreeJsonDumper::visit(std::shared_ptr<symbolic_expression::PrintIP> node) {
  dump_node(node);
}
void ParseTreeJsonDumper::visit(std::shared_ptr<symbolic_expression::Scan> node) {
  dump_node(node);
}
void ParseTreeJsonDumper::visit(std::shared_ptr<symbolic_expression::Exit> node) {
  dump_node(node);
}
void ParseTreeJsonDumper::visit(std::shared_ptr<symbolic_expression::Abort> node) {
  dump_node(node);
}
// SystemVariable
void ParseTreeJsonDumper::visit(std::shared_ptr<symbolic_expression::SVtimer> node) {
  dump_node(node);
}
// 関数
void ParseTreeJsonDumper::visit(std::shared_ptr<symbolic_expression::Function> node) {
  dump_node(node);
}
void ParseTreeJsonDumper::visit(std::shared_ptr<symbolic_expression::UnsupportedFunction> node) {
  dump_node(node);
}

// 円周率
void ParseTreeJsonDumper::visit(std::shared_ptr<symbolic_expression::Pi> node) { dump_node(node); }

// 自然対数の底
void ParseTreeJsonDumper::visit(std::shared_ptr<symbolic_expression::E> node) { dump_node(node); }

// True
void ParseTreeJsonDumper::visit(std::shared_ptr<symbolic_expression::True> node) {
  dump_node(node);
}

// False
void ParseTreeJsonDumper::visit(std::shared_ptr<symbolic_expression::False> node) {
  dump_node(node);
}

// 変数
void ParseTreeJsonDumper::visit(std::shared_ptr<symbolic_expression::Variable> node) {
  current_->get<object>()["type"] = value{node->get_node_type_name()};
  current_->get<object>()["name"] = value{node->get_name()};
}

// 数字
void ParseTreeJsonDumper::visit(std::shared_ptr<symbolic_expression::Number> node) {
  current_->get<object>()["type"] = value{node->get_node_type_name()};
  current_->get<object>()["value"] = value{node->get_number()};
}

void ParseTreeJsonDumper::visit(std::shared_ptr<symbolic_expression::Float> node) {
  current_->get<object>()["type"] = value{node->get_node_type_name()};
  current_->get<object>()["value"] = value{node->get_number()};
}

void ParseTreeJsonDumper::visit(std::shared_ptr<symbolic_expression::ImaginaryUnit> node) {
  dump_node(node);
}

void ParseTreeJsonDumper::visit(std::shared_ptr<symbolic_expression::Infinity> node) {
  dump_node(node);
}

void ParseTreeJsonDumper::visit(std::shared_ptr<symbolic_expression::Parameter> node) {
  dump_node(node);
}

void ParseTreeJsonDumper::visit(std::shared_ptr<symbolic_expression::SymbolicT> node) {
  dump_node(node);
}

// ExpressionList
void ParseTreeJsonDumper::visit(std::shared_ptr<symbolic_expression::ExpressionList> node) {
  dump_node(node);
}

// ConditionalExpressionList
void ParseTreeJsonDumper::visit(
    std::shared_ptr<symbolic_expression::ConditionalExpressionList> node) {
  dump_node(node);
}

// ProgramList
void ParseTreeJsonDumper::visit(std::shared_ptr<symbolic_expression::ProgramList> node) {
  dump_node(node);
}

// ConditionalProgramList
void ParseTreeJsonDumper::visit(
    std::shared_ptr<symbolic_expression::ConditionalProgramList> node) {
  dump_node(node);
}

// EachElement
void ParseTreeJsonDumper::visit(std::shared_ptr<symbolic_expression::EachElement> node) {
  dump_node(node);
}

// DifferentVariable
void ParseTreeJsonDumper::visit(std::shared_ptr<symbolic_expression::DifferentVariable> node) {
  dump_node(node);
}

// ExpressionListElement
void ParseTreeJsonDumper::visit(std::shared_ptr<symbolic_expression::ExpressionListElement> node) {
  dump_node(node);
}

// ExpressionListCaller
void ParseTreeJsonDumper::visit(std::shared_ptr<symbolic_expression::ExpressionListCaller> node) {
  dump_node(node);
}

// ProgramListElement
void ParseTreeJsonDumper::visit(std::shared_ptr<symbolic_expression::ProgramListElement> node) {
  dump_node(node);
}

// ExpressionListDefinition
void ParseTreeJsonDumper::visit(
    std::shared_ptr<symbolic_expression::ExpressionListDefinition> node) {
  dump_node(node);
}

// ProgramListDefinition
void ParseTreeJsonDumper::visit(std::shared_ptr<symbolic_expression::ProgramListDefinition> node) {
  dump_node(node);
}

// ProgramListCaller
void ParseTreeJsonDumper::visit(std::shared_ptr<symbolic_expression::ProgramListCaller> node) {
  dump_node(node);
}

// Union
void ParseTreeJsonDumper::visit(std::shared_ptr<symbolic_expression::Union> node) {
  dump_node(node);
}

// Intersection
void ParseTreeJsonDumper::visit(std::shared_ptr<symbolic_expression::Intersection> node) {
  dump_node(node);
}

// Range
void ParseTreeJsonDumper::visit(std::shared_ptr<symbolic_expression::Range> node) {
  dump_node(node);
}

// SizeOfList
void ParseTreeJsonDumper::visit(std::shared_ptr<symbolic_expression::SizeOfList> node) {
  dump_node(node);
}

// SumOfList
void ParseTreeJsonDumper::visit(std::shared_ptr<symbolic_expression::SumOfList> node) {
  dump_node(node);
}

// MulOfList
void ParseTreeJsonDumper::visit(std::shared_ptr<symbolic_expression::MulOfList> node) {
  dump_node(node);
}
} // namespace parser
} // namespace hydla

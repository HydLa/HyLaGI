#include "ParseTreeJsonDumper.h"

using namespace hydla::symbolic_expression;
using namespace hydla::parser::error;
using namespace std;

using std::make_pair;
using namespace picojson;

namespace hydla { 
namespace parser {

ParseTreeJsonDumper::ParseTreeJsonDumper()
{}

ParseTreeJsonDumper::~ParseTreeJsonDumper()
{}

std::ostream& ParseTreeJsonDumper::dump(std::ostream& s, const symbolic_expression::node_sptr& node)
{
  // initialize json and current pointer
  json_ = value{object{}};
  current_ = &json_;

  // traverse parse tree
  accept(node);

  // output parse tree in JSON format
  s << json_.serialize() << endl;

  return s;
}

void ParseTreeJsonDumper::dump_node(boost::shared_ptr<FactorNode> node)
{
  // set the type of current node
  current_->get<object>()["type"] = value{ node->get_node_type_name() };
}

void ParseTreeJsonDumper::dump_node(boost::shared_ptr<UnaryNode> node)
{
  // set the type of current node
  current_->get<object>()["type"] = value{ node->get_node_type_name() };

  // create child node
  current_->get<object>()["child"] = value{ object{} };
  current_ = &(current_->get<object>()["child"]);;

  // visit child node
  accept(node->get_child());
}

void ParseTreeJsonDumper::dump_node(boost::shared_ptr<BinaryNode> node)
{
  // set the type of current node
  current_->get<object>()["type"] = value{ node->get_node_type_name() };

  // create child nodes
  value * own = current_; // store the pointer to current node
  own->get<object>()["lhs"] = value{ object{} };
  own->get<object>()["rhs"] = value{ object{} };

  // visit lhs node
  current_ = &(own->get<object>()["lhs"]);;
  accept(node->get_lhs());

  // visit rhs node
  current_ = &(own->get<object>()["rhs"]);;
  accept(node->get_rhs());
}

void ParseTreeJsonDumper::dump_node(boost::shared_ptr<VariadicNode> node)
{
  // set the type of current node
  current_->get<object>()["type"] = value{ node->get_node_type_name() };

  // create child nodes array
  current_->get<object>()["children"] = value { picojson::array{} };
  
  value * own = current_;

  // visit each child nodes
  int length = node->get_arguments_size();
  for (int i = 0; i < length; ++i)
  {
    own->get<object>()["children"].get(i) = value{ object{} };
    current_ = &(own->get<object>()["children"].get(i));
    accept(node->get_argument(i));
  }
}

// 制約定義
void ParseTreeJsonDumper::visit(boost::shared_ptr<ConstraintDefinition> node)  
{
  dump_node(node);
}

// プログラム定義
void ParseTreeJsonDumper::visit(boost::shared_ptr<ProgramDefinition> node)     
{
  dump_node(node);
}

// 制約呼び出し
void ParseTreeJsonDumper::visit(boost::shared_ptr<ConstraintCaller> node)      
{
  current_->get<object>()["name"] = value{ node->get_name() };
  dump_node(node);
}

// プログラム呼び出し
void ParseTreeJsonDumper::visit(boost::shared_ptr<ProgramCaller> node)         
{
  dump_node(node);
}

// 制約式
void ParseTreeJsonDumper::visit(boost::shared_ptr<Constraint> node)            
{
  dump_node(node);
}

// Ask制約
void ParseTreeJsonDumper::visit(boost::shared_ptr<Ask> node)                   
{
  dump_node(node);
}

// Tell制約
void ParseTreeJsonDumper::visit(boost::shared_ptr<Tell> node)                  
{
  dump_node(node);
}

// 算術単項演算子
void ParseTreeJsonDumper::visit(boost::shared_ptr<Negative> node)
{
  dump_node(node);
}

void ParseTreeJsonDumper::visit(boost::shared_ptr<Positive> node)
{
  dump_node(node);
}

void ParseTreeJsonDumper::visit(boost::shared_ptr<Equal> node)                 
{
  dump_node(node);
}

void ParseTreeJsonDumper::visit(boost::shared_ptr<UnEqual> node)               
{
  dump_node(node);
}

void ParseTreeJsonDumper::visit(boost::shared_ptr<Less> node)
{
  dump_node(node);
}

void ParseTreeJsonDumper::visit(boost::shared_ptr<LessEqual> node)
{
  dump_node(node);
}

void ParseTreeJsonDumper::visit(boost::shared_ptr<Greater> node)
{
  dump_node(node);
}

void ParseTreeJsonDumper::visit(boost::shared_ptr<GreaterEqual> node)
{
  dump_node(node);
}

void ParseTreeJsonDumper::visit(boost::shared_ptr<Plus> node)
{
  dump_node(node);
}

void ParseTreeJsonDumper::visit(boost::shared_ptr<Subtract> node)
{
  dump_node(node);
}

void ParseTreeJsonDumper::visit(boost::shared_ptr<Times> node)
{
  dump_node(node);
}

void ParseTreeJsonDumper::visit(boost::shared_ptr<Divide> node)
{
  dump_node(node);
}


void ParseTreeJsonDumper::visit(boost::shared_ptr<Power> node)
{
  dump_node(node);
}

void ParseTreeJsonDumper::visit(boost::shared_ptr<LogicalAnd> node)
{
  dump_node(node);
}

void ParseTreeJsonDumper::visit(boost::shared_ptr<LogicalOr> node)
{
  dump_node(node);
}  


void ParseTreeJsonDumper::visit(boost::shared_ptr<Not> node)
{
  dump_node(node);
}  

void ParseTreeJsonDumper::visit(boost::shared_ptr<Weaker> node)
{
  dump_node(node);
}

void ParseTreeJsonDumper::visit(boost::shared_ptr<Parallel> node)
{
  dump_node(node);
}
  
// 時相演算子
void ParseTreeJsonDumper::visit(boost::shared_ptr<Always> node)
{
  dump_node(node);
}
  
// 微分
void ParseTreeJsonDumper::visit(boost::shared_ptr<Differential> node)
{
  dump_node(node);
}

// 左極限
void ParseTreeJsonDumper::visit(boost::shared_ptr<Previous> node)
{
  dump_node(node);
}

//Print 
void ParseTreeJsonDumper::visit(boost::shared_ptr<Print> node)
{
  dump_node(node);
}
void ParseTreeJsonDumper::visit(boost::shared_ptr<PrintPP> node)
{
  dump_node(node);
}
void ParseTreeJsonDumper::visit(boost::shared_ptr<PrintIP> node)
{
  dump_node(node);
}
void ParseTreeJsonDumper::visit(boost::shared_ptr<Scan> node)
{
  dump_node(node);
}
void ParseTreeJsonDumper::visit(boost::shared_ptr<Exit> node)
{
  dump_node(node);
}
void ParseTreeJsonDumper::visit(boost::shared_ptr<Abort> node)
{
  dump_node(node);
}
// SystemVariable
void ParseTreeJsonDumper::visit(boost::shared_ptr<SVtimer> node)
{
  dump_node(node);
}
// 関数
void ParseTreeJsonDumper::visit(boost::shared_ptr<Function> node)
{
  dump_node(node);
}
void ParseTreeJsonDumper::visit(boost::shared_ptr<UnsupportedFunction> node)
{
  dump_node(node);
}

// 円周率
void ParseTreeJsonDumper::visit(boost::shared_ptr<Pi> node)
{
  dump_node(node);
}

// 自然対数の底
void ParseTreeJsonDumper::visit(boost::shared_ptr<E> node)
{
  dump_node(node);
}

// True
void ParseTreeJsonDumper::visit(boost::shared_ptr<True> node)
{
  dump_node(node);
}

// False
void ParseTreeJsonDumper::visit(boost::shared_ptr<False> node)
{
  dump_node(node);
}

// 変数
void ParseTreeJsonDumper::visit(boost::shared_ptr<Variable> node)
{
  current_->get<object>()["type"] = value{ node->get_node_type_name() };
  current_->get<object>()["name"] = value{ node->get_name() };
}

// 数字
void ParseTreeJsonDumper::visit(boost::shared_ptr<Number> node)
{
  current_->get<object>()["type"] = value{ node->get_node_type_name() };
  current_->get<object>()["value"] = value{ node->get_number() };
}

void ParseTreeJsonDumper::visit(boost::shared_ptr<Float> node)
{
  current_->get<object>()["type"] = value{ node->get_node_type_name() };
  current_->get<object>()["value"] = value{ node->get_number() };
}

void ParseTreeJsonDumper::visit(boost::shared_ptr<ImaginaryUnit> node)
{
  dump_node(node);
}

void ParseTreeJsonDumper::visit(boost::shared_ptr<Infinity> node)
{
  dump_node(node);
}

void ParseTreeJsonDumper::visit(boost::shared_ptr<Parameter> node)
{
  dump_node(node);
}

void ParseTreeJsonDumper::visit(boost::shared_ptr<SymbolicT> node)
{
  dump_node(node);
}

// ExpressionList
void ParseTreeJsonDumper::visit(boost::shared_ptr<ExpressionList> node)
{
  dump_node(node);
}

// ConditionalExpressionList
void ParseTreeJsonDumper::visit(boost::shared_ptr<ConditionalExpressionList> node)
{
  dump_node(node);
}

// ProgramList
void ParseTreeJsonDumper::visit(boost::shared_ptr<ProgramList> node)
{
  dump_node(node);
}

// ConditionalProgramList
void ParseTreeJsonDumper::visit(boost::shared_ptr<ConditionalProgramList> node)
{
  dump_node(node);
}

// EachElement
void ParseTreeJsonDumper::visit(boost::shared_ptr<EachElement> node)
{
  dump_node(node);
}

// DifferentVariable
void ParseTreeJsonDumper::visit(boost::shared_ptr<DifferentVariable> node)
{
  dump_node(node);
}

// ExpressionListElement 
void ParseTreeJsonDumper::visit(boost::shared_ptr<ExpressionListElement> node)
{
  dump_node(node);
}

// ExpressionListCaller
void ParseTreeJsonDumper::visit(boost::shared_ptr<ExpressionListCaller> node)
{
  dump_node(node);
}

// ProgramListElement 
void ParseTreeJsonDumper::visit(boost::shared_ptr<ProgramListElement> node)
{
  dump_node(node);
}

// ExpressionListDefinition
void ParseTreeJsonDumper::visit(boost::shared_ptr<ExpressionListDefinition> node)
{
  dump_node(node);
}

// ProgramListDefinition
void ParseTreeJsonDumper::visit(boost::shared_ptr<ProgramListDefinition> node)
{
  dump_node(node);
}

// ProgramListCaller 
void ParseTreeJsonDumper::visit(boost::shared_ptr<ProgramListCaller> node)
{
  dump_node(node);
}

// Union 
void ParseTreeJsonDumper::visit(boost::shared_ptr<Union> node)
{
  dump_node(node);
}

// Intersection
void ParseTreeJsonDumper::visit(boost::shared_ptr<Intersection> node)
{
  dump_node(node);
}

// Range
void ParseTreeJsonDumper::visit(boost::shared_ptr<Range> node)
{
  dump_node(node);
}

// SizeOfList
void ParseTreeJsonDumper::visit(boost::shared_ptr<SizeOfList> node)
{
  dump_node(node);
}

// SumOfList
void ParseTreeJsonDumper::visit(boost::shared_ptr<SumOfList> node)
{
  dump_node(node);
}

} //namespace parser
} //namespace hydla

#include <algorithm>

#include "ParseTreeGraphvizDumper.h"

using namespace hydla::symbolic_expression;
using namespace hydla::parser::error;
using namespace std;

using std::make_pair;

namespace hydla { 
namespace parser {

ParseTreeGraphvizDumper::ParseTreeGraphvizDumper()
{}

ParseTreeGraphvizDumper::~ParseTreeGraphvizDumper()
{}

namespace {

  struct NodeDumper {
    NodeDumper(std::ostream& s) :
      s_(s)
    {}

    template<typename T>
    void operator()(const T& n) {
      s_ << "  "
         << "n" << n.first 
         << " [label = \"" << n.second << "\"];\n";      
    }

    std::ostream& s_;
  };
  
  struct EdgeDumper {
    EdgeDumper(std::ostream& s) :
      s_(s)
    {}

    template<typename T>
    void operator()(const T& n) {
      s_ << "  "
         << "n" << n.first << " -> "
         << "n" << n.second << ";\n";      
    }

    std::ostream& s_;
  };
}

std::ostream& ParseTreeGraphvizDumper::dump(std::ostream& s, const symbolic_expression::node_sptr& node)
{
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



void ParseTreeGraphvizDumper::dump_node(std::shared_ptr<FactorNode> node)
{
  node_id_t own_id = node_id_;
  nodes_.insert(make_pair(own_id, node->get_node_type_name()));
}

void ParseTreeGraphvizDumper::dump_node(std::shared_ptr<UnaryNode> node)
{
  node_id_t own_id = node_id_;
  nodes_.insert(make_pair(own_id, node->get_node_type_name()));

  edges_.insert(make_pair(own_id, ++node_id_));
  accept(node->get_child());
}

void ParseTreeGraphvizDumper::dump_node(std::shared_ptr<BinaryNode> node)
{
  node_id_t own_id = node_id_;
  nodes_.insert(make_pair(own_id, node->get_node_type_name()));

  edges_.insert(make_pair(own_id, ++node_id_));
  accept(node->get_lhs());

  edges_.insert(make_pair(own_id, ++node_id_));
  accept(node->get_rhs());
}


void ParseTreeGraphvizDumper::dump_node(std::shared_ptr<VariadicNode> node)
{
  node_id_t own_id = node_id_;
  nodes_.insert(make_pair(own_id, node->get_string()));

  for(int i=0; i< node->get_arguments_size();i++){
    edges_.insert(make_pair(own_id, ++node_id_));
    accept(node->get_argument(i));
  }
}


// 制約定義
void ParseTreeGraphvizDumper::visit(std::shared_ptr<ConstraintDefinition> node)  
{
  dump_node(node);
}

// プログラム定義
void ParseTreeGraphvizDumper::visit(std::shared_ptr<ProgramDefinition> node)     
{
  dump_node(node);
}

// 制約呼び出し
void ParseTreeGraphvizDumper::visit(std::shared_ptr<ConstraintCaller> node)      
{
  dump_node(node);
}

// プログラム呼び出し
void ParseTreeGraphvizDumper::visit(std::shared_ptr<ProgramCaller> node)         
{
  dump_node(node);
}

// 制約式
void ParseTreeGraphvizDumper::visit(std::shared_ptr<Constraint> node)            
{
  dump_node(node);
}

// Ask制約
void ParseTreeGraphvizDumper::visit(std::shared_ptr<Ask> node)                   
{
  dump_node(node);
}

// Tell制約
void ParseTreeGraphvizDumper::visit(std::shared_ptr<Tell> node)                  
{
  dump_node(node);
}

// 算術単項演算子
void ParseTreeGraphvizDumper::visit(std::shared_ptr<Negative> node)
{
  dump_node(node);
}

void ParseTreeGraphvizDumper::visit(std::shared_ptr<Positive> node)
{
  dump_node(node);
}

void ParseTreeGraphvizDumper::visit(std::shared_ptr<Equal> node)                 
{
  dump_node(node);
}

void ParseTreeGraphvizDumper::visit(std::shared_ptr<UnEqual> node)               
{
  dump_node(node);
}

void ParseTreeGraphvizDumper::visit(std::shared_ptr<Less> node)
{
  dump_node(node);
}

void ParseTreeGraphvizDumper::visit(std::shared_ptr<LessEqual> node)
{
  dump_node(node);
}

void ParseTreeGraphvizDumper::visit(std::shared_ptr<Greater> node)
{
  dump_node(node);
}

void ParseTreeGraphvizDumper::visit(std::shared_ptr<GreaterEqual> node)
{
  dump_node(node);
}

void ParseTreeGraphvizDumper::visit(std::shared_ptr<Plus> node)
{
  dump_node(node);
}

void ParseTreeGraphvizDumper::visit(std::shared_ptr<Subtract> node)
{
  dump_node(node);
}

void ParseTreeGraphvizDumper::visit(std::shared_ptr<Times> node)
{
  dump_node(node);
}

void ParseTreeGraphvizDumper::visit(std::shared_ptr<Divide> node)
{
  dump_node(node);
}


void ParseTreeGraphvizDumper::visit(std::shared_ptr<Power> node)
{
  dump_node(node);
}

void ParseTreeGraphvizDumper::visit(std::shared_ptr<LogicalAnd> node)
{
  dump_node(node);
}

void ParseTreeGraphvizDumper::visit(std::shared_ptr<LogicalOr> node)
{
  dump_node(node);
}  


void ParseTreeGraphvizDumper::visit(std::shared_ptr<Not> node)
{
  dump_node(node);
}  

void ParseTreeGraphvizDumper::visit(std::shared_ptr<Weaker> node)
{
  dump_node(node);
}

void ParseTreeGraphvizDumper::visit(std::shared_ptr<Parallel> node)
{
  dump_node(node);
}
  
// 時相演算子
void ParseTreeGraphvizDumper::visit(std::shared_ptr<Always> node)
{
  dump_node(node);
}
  
// 微分
void ParseTreeGraphvizDumper::visit(std::shared_ptr<Differential> node)
{
  dump_node(node);
}

// 左極限
void ParseTreeGraphvizDumper::visit(std::shared_ptr<Previous> node)
{
  dump_node(node);
}

//Print 
void ParseTreeGraphvizDumper::visit(std::shared_ptr<Print> node)
{
  dump_node(node);
}
void ParseTreeGraphvizDumper::visit(std::shared_ptr<PrintPP> node)
{
  dump_node(node);
}
void ParseTreeGraphvizDumper::visit(std::shared_ptr<PrintIP> node)
{
  dump_node(node);
}
void ParseTreeGraphvizDumper::visit(std::shared_ptr<Scan> node)
{
  dump_node(node);
}
void ParseTreeGraphvizDumper::visit(std::shared_ptr<Exit> node)
{
  dump_node(node);
}
void ParseTreeGraphvizDumper::visit(std::shared_ptr<Abort> node)
{
  dump_node(node);
}
// SystemVariable
void ParseTreeGraphvizDumper::visit(std::shared_ptr<SVtimer> node)
{
  dump_node(node);
}
// 関数
void ParseTreeGraphvizDumper::visit(std::shared_ptr<Function> node)
{
  dump_node(node);
}
void ParseTreeGraphvizDumper::visit(std::shared_ptr<UnsupportedFunction> node)
{
  dump_node(node);
}

// 円周率
void ParseTreeGraphvizDumper::visit(std::shared_ptr<Pi> node)
{
  dump_node(node);
}

// 自然対数の底
void ParseTreeGraphvizDumper::visit(std::shared_ptr<E> node)
{
  dump_node(node);
}

// True
void ParseTreeGraphvizDumper::visit(std::shared_ptr<True> node)
{
  dump_node(node);
}

// False
void ParseTreeGraphvizDumper::visit(std::shared_ptr<False> node)
{
  dump_node(node);
}

// 変数
void ParseTreeGraphvizDumper::visit(std::shared_ptr<Variable> node)
{
  node_id_t own_id = node_id_;
  nodes_.insert(make_pair(own_id, 
    "{" + node->get_node_type_name() + " | " + node->get_name() + "}"));
}

// 数字
void ParseTreeGraphvizDumper::visit(std::shared_ptr<Number> node)
{
  node_id_t own_id = node_id_;
  nodes_.insert(make_pair(own_id, 
    "{" + node->get_node_type_name() + " | " + node->get_number() + "}"));
}

void ParseTreeGraphvizDumper::visit(std::shared_ptr<Float> node)
{
  stringstream sstr;
  sstr << node->get_number();
  node_id_t own_id = node_id_;
  nodes_.insert(make_pair(own_id, 
               "{" + node->get_node_type_name()
               + " | " + sstr.str() + "}"));
}


void ParseTreeGraphvizDumper::visit(std::shared_ptr<ImaginaryUnit> node)
{
  dump_node(node);
}


void ParseTreeGraphvizDumper::visit(std::shared_ptr<Infinity> node)
{
  dump_node(node);
}


void ParseTreeGraphvizDumper::visit(std::shared_ptr<Parameter> node)
{
  
  node_id_t own_id = node_id_;
  stringstream sstr;
  sstr << "{" << node->get_node_type_name() << " | " << node->get_name() << ", " << node->get_differential_count() << ", " << node->get_phase_id() << "}";
  nodes_.insert(make_pair(own_id, sstr.str()));
}

void ParseTreeGraphvizDumper::visit(std::shared_ptr<SymbolicT> node)
{
  dump_node(node);
}

// ExpressionList
void ParseTreeGraphvizDumper::visit(std::shared_ptr<ExpressionList> node)
{
  dump_node(node);
}

// ConditionalExpressionList
void ParseTreeGraphvizDumper::visit(std::shared_ptr<ConditionalExpressionList> node)
{
  dump_node(node);
}

// ProgramList
void ParseTreeGraphvizDumper::visit(std::shared_ptr<ProgramList> node)
{
  dump_node(node);
}

// ConditionalProgramList
void ParseTreeGraphvizDumper::visit(std::shared_ptr<ConditionalProgramList> node)
{
  dump_node(node);
}

// EachElement
void ParseTreeGraphvizDumper::visit(std::shared_ptr<EachElement> node)
{
  dump_node(node);
}

// DifferentVariable
void ParseTreeGraphvizDumper::visit(std::shared_ptr<DifferentVariable> node)
{
  dump_node(node);
}

// ExpressionListElement 
void ParseTreeGraphvizDumper::visit(std::shared_ptr<ExpressionListElement> node)
{
  dump_node(node);
}

// ExpressionListCaller
void ParseTreeGraphvizDumper::visit(std::shared_ptr<ExpressionListCaller> node)
{
  dump_node(node);
}

// ProgramListElement 
void ParseTreeGraphvizDumper::visit(std::shared_ptr<ProgramListElement> node)
{
  dump_node(node);
}

// ExpressionListDefinition
void ParseTreeGraphvizDumper::visit(std::shared_ptr<ExpressionListDefinition> node)
{
  dump_node(node);
}

// ProgramListDefinition
void ParseTreeGraphvizDumper::visit(std::shared_ptr<ProgramListDefinition> node)
{
  dump_node(node);
}

// ProgramListCaller 
void ParseTreeGraphvizDumper::visit(std::shared_ptr<ProgramListCaller> node)
{
  dump_node(node);
}

// Union 
void ParseTreeGraphvizDumper::visit(std::shared_ptr<Union> node)
{
  dump_node(node);
}

// Intersection
void ParseTreeGraphvizDumper::visit(std::shared_ptr<Intersection> node)
{
  dump_node(node);
}

// Range
void ParseTreeGraphvizDumper::visit(std::shared_ptr<Range> node)
{
  dump_node(node);
}

// SizeOfList
void ParseTreeGraphvizDumper::visit(std::shared_ptr<SizeOfList> node)
{
  dump_node(node);
}

// SumOfList
void ParseTreeGraphvizDumper::visit(std::shared_ptr<SumOfList> node)
{
  dump_node(node);
}

// MulOfList
void ParseTreeGraphvizDumper::visit(std::shared_ptr<MulOfList> node)
{
  dump_node(node);
}



} //namespace parser
} //namespace hydla

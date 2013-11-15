#include "DefaultTreeVisitor.h"


namespace hydla { 
namespace parse_tree {

DefaultTreeVisitor::DefaultTreeVisitor()
{}

DefaultTreeVisitor::~DefaultTreeVisitor()
{}

// 定義
void DefaultTreeVisitor::visit(boost::shared_ptr<ConstraintDefinition> node)  {accept(node->get_child());}
void DefaultTreeVisitor::visit(boost::shared_ptr<ProgramDefinition> node)     {accept(node->get_child());}

// 呼び出し
void DefaultTreeVisitor::visit(boost::shared_ptr<ConstraintCaller> node)      {accept(node->get_child());}
void DefaultTreeVisitor::visit(boost::shared_ptr<ProgramCaller> node)         {accept(node->get_child());}

// 制約式
void DefaultTreeVisitor::visit(boost::shared_ptr<Constraint> node)            {accept(node->get_child());}

// Ask制約
void DefaultTreeVisitor::visit(boost::shared_ptr<Ask> node)                   {accept(node->get_lhs());accept(node->get_rhs());}

// Tell制約
void DefaultTreeVisitor::visit(boost::shared_ptr<Tell> node)                  {accept(node->get_child());}

// 比較演算子
void DefaultTreeVisitor::visit(boost::shared_ptr<Equal> node)                 {accept(node->get_lhs());accept(node->get_rhs());}
void DefaultTreeVisitor::visit(boost::shared_ptr<UnEqual> node)               {accept(node->get_lhs());accept(node->get_rhs());}
void DefaultTreeVisitor::visit(boost::shared_ptr<Less> node)                  {accept(node->get_lhs());accept(node->get_rhs());}
void DefaultTreeVisitor::visit(boost::shared_ptr<LessEqual> node)             {accept(node->get_lhs());accept(node->get_rhs());}
void DefaultTreeVisitor::visit(boost::shared_ptr<Greater> node)               {accept(node->get_lhs());accept(node->get_rhs());}
void DefaultTreeVisitor::visit(boost::shared_ptr<GreaterEqual> node)          {accept(node->get_lhs());accept(node->get_rhs());}

// 論理演算子
void DefaultTreeVisitor::visit(boost::shared_ptr<LogicalAnd> node)            {accept(node->get_lhs());accept(node->get_rhs());}
void DefaultTreeVisitor::visit(boost::shared_ptr<LogicalOr> node)             {accept(node->get_lhs());accept(node->get_rhs());}
  
// 算術二項演算子
void DefaultTreeVisitor::visit(boost::shared_ptr<Plus> node)                  {accept(node->get_lhs());accept(node->get_rhs());}
void DefaultTreeVisitor::visit(boost::shared_ptr<Subtract> node)              {accept(node->get_lhs());accept(node->get_rhs());}
void DefaultTreeVisitor::visit(boost::shared_ptr<Times> node)                 {accept(node->get_lhs());accept(node->get_rhs());}
void DefaultTreeVisitor::visit(boost::shared_ptr<Divide> node)                {accept(node->get_lhs());accept(node->get_rhs());}
void DefaultTreeVisitor::visit(boost::shared_ptr<Power> node)                 {accept(node->get_lhs());accept(node->get_rhs());}

// 算術単項演算子
void DefaultTreeVisitor::visit(boost::shared_ptr<Negative> node)              {accept(node->get_child());}
void DefaultTreeVisitor::visit(boost::shared_ptr<Positive> node)              {accept(node->get_child());}
  
// 制約階層定義演算子
void DefaultTreeVisitor::visit(boost::shared_ptr<Weaker> node)                {accept(node->get_lhs());accept(node->get_rhs());}
void DefaultTreeVisitor::visit(boost::shared_ptr<Parallel> node)              {accept(node->get_lhs());accept(node->get_rhs());}

// 時相演算子
void DefaultTreeVisitor::visit(boost::shared_ptr<Always> node)                {accept(node->get_child());}

// 円周率
void DefaultTreeVisitor::visit(boost::shared_ptr<Pi> node)                {}
// 自然対数の底
void DefaultTreeVisitor::visit(boost::shared_ptr<E> node)                {}
// True
void DefaultTreeVisitor::visit(boost::shared_ptr<True> node)                {}

// 関数
void DefaultTreeVisitor::visit(boost::shared_ptr<Function> node)                {for(int i=0;i<node->get_arguments_size();i++){accept(node->get_argument(i));}}
void DefaultTreeVisitor::visit(boost::shared_ptr<UnsupportedFunction> node)     {for(int i=0;i<node->get_arguments_size();i++){accept(node->get_argument(i));}}

  
// 微分
void DefaultTreeVisitor::visit(boost::shared_ptr<Differential> node)          {accept(node->get_child());}

// 左極限
void DefaultTreeVisitor::visit(boost::shared_ptr<Previous> node)              {accept(node->get_child());}

// 否定
void DefaultTreeVisitor::visit(boost::shared_ptr<Not> node)              {accept(node->get_child());}
  
// 変数
void DefaultTreeVisitor::visit(boost::shared_ptr<Variable> node)              {}

// 数字
void DefaultTreeVisitor::visit(boost::shared_ptr<Number> node)                {}

// 記号定数
void DefaultTreeVisitor::visit(boost::shared_ptr<Parameter> node)                {}

// t
void DefaultTreeVisitor::visit(boost::shared_ptr<SymbolicT> node)                {}

// Print
void DefaultTreeVisitor::visit(boost::shared_ptr<Print> node)              {}
void DefaultTreeVisitor::visit(boost::shared_ptr<PrintPP> node)              {}
void DefaultTreeVisitor::visit(boost::shared_ptr<PrintIP> node)              {}

void DefaultTreeVisitor::visit(boost::shared_ptr<Scan> node)              {}
void DefaultTreeVisitor::visit(boost::shared_ptr<Exit> node)              {}
void DefaultTreeVisitor::visit(boost::shared_ptr<Abort> node)              {}

void DefaultTreeVisitor::visit(boost::shared_ptr<SVtimer> node)              {}

void DefaultTreeVisitor::visit(boost::shared_ptr<Infinity> node)              {}


} //namespace parse_tree
} //namespace hydla

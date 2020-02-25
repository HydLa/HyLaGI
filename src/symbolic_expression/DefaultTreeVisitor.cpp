#include "DefaultTreeVisitor.h"


namespace hydla { 
namespace symbolic_expression {

DefaultTreeVisitor::DefaultTreeVisitor()
{}

DefaultTreeVisitor::~DefaultTreeVisitor()
{}

// 定義
void DefaultTreeVisitor::visit(std::shared_ptr<ConstraintDefinition> node)  {accept(node->get_child());}
void DefaultTreeVisitor::visit(std::shared_ptr<ProgramDefinition> node)     {accept(node->get_child());}

// 呼び出し
void DefaultTreeVisitor::visit(std::shared_ptr<ConstraintCaller> node)      {accept(node->get_child());}
void DefaultTreeVisitor::visit(std::shared_ptr<ProgramCaller> node)         {accept(node->get_child());}

// 制約式
void DefaultTreeVisitor::visit(std::shared_ptr<Constraint> node)            {accept(node->get_child());}

// Ask制約
void DefaultTreeVisitor::visit(std::shared_ptr<Ask> node)                   {accept(node->get_lhs());accept(node->get_rhs());}

// Tell制約
void DefaultTreeVisitor::visit(std::shared_ptr<Tell> node)                  {accept(node->get_child());}

// 比較演算子
void DefaultTreeVisitor::visit(std::shared_ptr<Equal> node)                 {accept(node->get_lhs());accept(node->get_rhs());}
void DefaultTreeVisitor::visit(std::shared_ptr<UnEqual> node)               {accept(node->get_lhs());accept(node->get_rhs());}
void DefaultTreeVisitor::visit(std::shared_ptr<Less> node)                  {accept(node->get_lhs());accept(node->get_rhs());}
void DefaultTreeVisitor::visit(std::shared_ptr<LessEqual> node)             {accept(node->get_lhs());accept(node->get_rhs());}
void DefaultTreeVisitor::visit(std::shared_ptr<Greater> node)               {accept(node->get_lhs());accept(node->get_rhs());}
void DefaultTreeVisitor::visit(std::shared_ptr<GreaterEqual> node)          {accept(node->get_lhs());accept(node->get_rhs());}

// 論理演算子
void DefaultTreeVisitor::visit(std::shared_ptr<LogicalAnd> node)            {accept(node->get_lhs());accept(node->get_rhs());}
void DefaultTreeVisitor::visit(std::shared_ptr<LogicalOr> node)             {accept(node->get_lhs());accept(node->get_rhs());}
  
// 算術二項演算子
void DefaultTreeVisitor::visit(std::shared_ptr<Plus> node)                  {accept(node->get_lhs());accept(node->get_rhs());}
void DefaultTreeVisitor::visit(std::shared_ptr<Subtract> node)              {accept(node->get_lhs());accept(node->get_rhs());}
void DefaultTreeVisitor::visit(std::shared_ptr<Times> node)                 {accept(node->get_lhs());accept(node->get_rhs());}
void DefaultTreeVisitor::visit(std::shared_ptr<Divide> node)                {accept(node->get_lhs());accept(node->get_rhs());}
void DefaultTreeVisitor::visit(std::shared_ptr<Power> node)                 {accept(node->get_lhs());accept(node->get_rhs());}

// 算術単項演算子
void DefaultTreeVisitor::visit(std::shared_ptr<Negative> node)              {accept(node->get_child());}
void DefaultTreeVisitor::visit(std::shared_ptr<Positive> node)              {accept(node->get_child());}
  
// 制約階層定義演算子
void DefaultTreeVisitor::visit(std::shared_ptr<Weaker> node)                {accept(node->get_lhs());accept(node->get_rhs());}
void DefaultTreeVisitor::visit(std::shared_ptr<Parallel> node)              {accept(node->get_lhs());accept(node->get_rhs());}

// 時相演算子
void DefaultTreeVisitor::visit(std::shared_ptr<Always> node)                {accept(node->get_child());}

// 円周率
void DefaultTreeVisitor::visit(std::shared_ptr<Pi> node)                {}
// 自然対数の底
void DefaultTreeVisitor::visit(std::shared_ptr<E> node)                {}
// True
void DefaultTreeVisitor::visit(std::shared_ptr<True> node)                {}
// False
void DefaultTreeVisitor::visit(std::shared_ptr<False> node)                {}

// 関数
void DefaultTreeVisitor::visit(std::shared_ptr<Function> node)                {for(int i=0;i<node->get_arguments_size();i++){accept(node->get_argument(i));}}
void DefaultTreeVisitor::visit(std::shared_ptr<UnsupportedFunction> node)     {for(int i=0;i<node->get_arguments_size();i++){accept(node->get_argument(i));}}

  
// 微分
void DefaultTreeVisitor::visit(std::shared_ptr<Differential> node)          {accept(node->get_child());}

// 左極限
void DefaultTreeVisitor::visit(std::shared_ptr<Previous> node)              {accept(node->get_child());}

// 否定
void DefaultTreeVisitor::visit(std::shared_ptr<Not> node)              {accept(node->get_child());}
  
// 変数
void DefaultTreeVisitor::visit(std::shared_ptr<Variable> node)              {}

// 数字
void DefaultTreeVisitor::visit(std::shared_ptr<Number> node)                {}
void DefaultTreeVisitor::visit(std::shared_ptr<Float> node)                {}

// 記号定数
void DefaultTreeVisitor::visit(std::shared_ptr<Parameter> node)                {}

// t
void DefaultTreeVisitor::visit(std::shared_ptr<SymbolicT> node)                {}

void DefaultTreeVisitor::visit(std::shared_ptr<ImaginaryUnit> node)                {}

// Print
void DefaultTreeVisitor::visit(std::shared_ptr<Print> node)              {}
void DefaultTreeVisitor::visit(std::shared_ptr<PrintPP> node)              {}
void DefaultTreeVisitor::visit(std::shared_ptr<PrintIP> node)              {}

void DefaultTreeVisitor::visit(std::shared_ptr<Scan> node)              {}
void DefaultTreeVisitor::visit(std::shared_ptr<Exit> node)              {}
void DefaultTreeVisitor::visit(std::shared_ptr<Abort> node)              {}

void DefaultTreeVisitor::visit(std::shared_ptr<SVtimer> node)              {}

void DefaultTreeVisitor::visit(std::shared_ptr<Infinity> node)              {}

// ExpressionList
void DefaultTreeVisitor::visit(std::shared_ptr<ExpressionList> node)  {for(int i=0;i<node->get_arguments_size();i++){accept(node->get_argument(i));}}
// ConditionalExpressionList
void DefaultTreeVisitor::visit(std::shared_ptr<ConditionalExpressionList> node)   {for(int i=0;i<node->get_arguments_size();i++){accept(node->get_argument(i)); accept(node->get_expression());}}
// ProgramList
void DefaultTreeVisitor::visit(std::shared_ptr<ProgramList> node)  {for(int i=0;i<node->get_arguments_size();i++){accept(node->get_argument(i));}}
// ConditionalProgramList
void DefaultTreeVisitor::visit(std::shared_ptr<ConditionalProgramList> node)    {for(int i=0;i<node->get_arguments_size();i++){accept(node->get_argument(i));} accept(node->get_program());}
// EachElement
void DefaultTreeVisitor::visit(std::shared_ptr<EachElement> node)    {accept(node->get_lhs()); accept(node->get_rhs());}
// DifferentVariable
void DefaultTreeVisitor::visit(std::shared_ptr<DifferentVariable> node)    {accept(node->get_lhs()); accept(node->get_rhs());}
// ExpressionListElement
void DefaultTreeVisitor::visit(std::shared_ptr<ExpressionListElement> node)    {accept(node->get_lhs()); accept(node->get_rhs());}
// ExpressionListCaller
void DefaultTreeVisitor::visit(std::shared_ptr<ExpressionListCaller> node)    {accept(node->get_child());}
// ExpressionListDefinition
void DefaultTreeVisitor::visit(std::shared_ptr<ExpressionListDefinition> node)    {accept(node->get_child());}
// ProgramListElement
void DefaultTreeVisitor::visit(std::shared_ptr<ProgramListElement> node)    {accept(node->get_lhs()); accept(node->get_rhs());}
// ProgramListCaller
void DefaultTreeVisitor::visit(std::shared_ptr<ProgramListCaller> node)    {accept(node->get_child());}
// ProgramListDefinition
void DefaultTreeVisitor::visit(std::shared_ptr<ProgramListDefinition> node)    {accept(node->get_child());}
// Range
void DefaultTreeVisitor::visit(std::shared_ptr<Range> node)    {accept(node->get_lhs()); accept(node->get_rhs());}
// Union 
void DefaultTreeVisitor::visit(std::shared_ptr<Union> node)    {accept(node->get_lhs()); accept(node->get_rhs());}
// Intersection 
void DefaultTreeVisitor::visit(std::shared_ptr<Intersection> node)    {accept(node->get_lhs()); accept(node->get_rhs());}
// SizeOfList 
void DefaultTreeVisitor::visit(std::shared_ptr<SizeOfList> node)    {accept(node->get_child());}
// SumOfList
void DefaultTreeVisitor::visit(std::shared_ptr<SumOfList> node)    {accept(node->get_child());}
// MulOfList
void DefaultTreeVisitor::visit(std::shared_ptr<MulOfList> node)    {accept(node->get_child());}

} //namespace symbolic_expression
} //namespace hydla

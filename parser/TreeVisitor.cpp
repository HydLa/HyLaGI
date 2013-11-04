#include "TreeVisitor.h"

#include <assert.h>

namespace hydla { 
namespace parse_tree {

TreeVisitor::TreeVisitor()
{}

TreeVisitor::~TreeVisitor()
{}

// 定義
void TreeVisitor::visit(boost::shared_ptr<ConstraintDefinition> node)  {assert(0);}
void TreeVisitor::visit(boost::shared_ptr<ProgramDefinition> node)     {assert(0);}

// 呼び出し
void TreeVisitor::visit(boost::shared_ptr<ConstraintCaller> node)      {assert(0);}
void TreeVisitor::visit(boost::shared_ptr<ProgramCaller> node)         {assert(0);}

// 制約式
void TreeVisitor::visit(boost::shared_ptr<Constraint> node)            {assert(0);}

// Ask制約
void TreeVisitor::visit(boost::shared_ptr<Ask> node)                   {assert(0);}

// Tell制約
void TreeVisitor::visit(boost::shared_ptr<Tell> node)                  {assert(0);}

// 比較演算子
void TreeVisitor::visit(boost::shared_ptr<Equal> node)                 {assert(0);}
void TreeVisitor::visit(boost::shared_ptr<UnEqual> node)               {assert(0);}
void TreeVisitor::visit(boost::shared_ptr<Less> node)                  {assert(0);}
void TreeVisitor::visit(boost::shared_ptr<LessEqual> node)             {assert(0);}
void TreeVisitor::visit(boost::shared_ptr<Greater> node)               {assert(0);}
void TreeVisitor::visit(boost::shared_ptr<GreaterEqual> node)          {assert(0);}

// 論理演算子
void TreeVisitor::visit(boost::shared_ptr<LogicalAnd> node)            {assert(0);}
void TreeVisitor::visit(boost::shared_ptr<LogicalOr> node)             {assert(0);}
  
// 算術二項演算子
void TreeVisitor::visit(boost::shared_ptr<Plus> node)                  {assert(0);}
void TreeVisitor::visit(boost::shared_ptr<Subtract> node)              {assert(0);}
void TreeVisitor::visit(boost::shared_ptr<Times> node)                 {assert(0);}
void TreeVisitor::visit(boost::shared_ptr<Divide> node)                {assert(0);}
void TreeVisitor::visit(boost::shared_ptr<Power> node)                 {assert(0);}
  
// 算術単項演算子
void TreeVisitor::visit(boost::shared_ptr<Negative> node)              {assert(0);}
void TreeVisitor::visit(boost::shared_ptr<Positive> node)              {assert(0);}
  
// 制約階層定義演算子
void TreeVisitor::visit(boost::shared_ptr<Weaker> node)                {assert(0);}
void TreeVisitor::visit(boost::shared_ptr<Parallel> node)              {assert(0);}

// 時相演算子
void TreeVisitor::visit(boost::shared_ptr<Always> node)                {assert(0);}

// 円周率
void TreeVisitor::visit(boost::shared_ptr<Pi> node)                {assert(0);}
// 自然対数の底
void TreeVisitor::visit(boost::shared_ptr<E> node)                {assert(0);}
// 任意の文字列
void TreeVisitor::visit(boost::shared_ptr<ArbitraryNode> node)                {assert(0);}
  
// 微分
void TreeVisitor::visit(boost::shared_ptr<Differential> node)          {assert(0);}

// 左極限
void TreeVisitor::visit(boost::shared_ptr<Previous> node)              {assert(0);}

// 否定
void TreeVisitor::visit(boost::shared_ptr<Not> node)              {assert(0);}
  
// 変数
void TreeVisitor::visit(boost::shared_ptr<Variable> node)              {assert(0);}

// 数字
void TreeVisitor::visit(boost::shared_ptr<Number> node)                {assert(0);}

// 記号定数
void TreeVisitor::visit(boost::shared_ptr<Parameter> node)                {assert(0);}

// t
void TreeVisitor::visit(boost::shared_ptr<SymbolicT> node)                {assert(0);}

// 無限大
void TreeVisitor::visit(boost::shared_ptr<Infinity> node)                {assert(0);}
        
// 関数
void TreeVisitor::visit(boost::shared_ptr<Function> node)                {assert(0);}
void TreeVisitor::visit(boost::shared_ptr<UnsupportedFunction> node)                {assert(0);}

// Print
void TreeVisitor::visit(boost::shared_ptr<Print> node)              {assert(0);}
void TreeVisitor::visit(boost::shared_ptr<PrintPP> node)              {assert(0);}
void TreeVisitor::visit(boost::shared_ptr<PrintIP> node)              {assert(0);}
    
void TreeVisitor::visit(boost::shared_ptr<Scan> node)              {assert(0);}
void TreeVisitor::visit(boost::shared_ptr<Exit> node)              {assert(0);}
void TreeVisitor::visit(boost::shared_ptr<Abort> node)              {assert(0);}

//SystemVariable
void TreeVisitor::visit(boost::shared_ptr<SVtimer> node)              {assert(0);}
void TreeVisitor::visit(boost::shared_ptr<True> node)              {assert(0);}


} //namespace parse_tree
} //namespace hydla

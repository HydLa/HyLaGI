#include "PreprocessParseTree.h"

namespace hydla { 
namespace parse_tree {

PreprocessParseTree::PreprocessParseTree()
{}

PreprocessParseTree::~PreprocessParseTree()
{}

// ง๑่`
void PreprocessParseTree::visit(boost::shared_ptr<ConstraintDefinition> node)  
{
  assert(0);
}

// vO่`
void PreprocessParseTree::visit(boost::shared_ptr<ProgramDefinition> node)     
{
  assert(0);
}

// ง๑ฤัoต
void PreprocessParseTree::visit(boost::shared_ptr<ConstraintCaller> node)      
{
  assert(0);
}

// vOฤัoต
void PreprocessParseTree::visit(boost::shared_ptr<ProgramCaller> node)         
{
  assert(0);
}

// ง๑ฎ
void PreprocessParseTree::visit(boost::shared_ptr<Constraint> node)            
{
  assert(0);
}

// Askง๑
void PreprocessParseTree::visit(boost::shared_ptr<Ask> node)                   
{
  assert(0);
}

// Tellง๑
void PreprocessParseTree::visit(boost::shared_ptr<Tell> node)                  
{
  dispatch_child(node);
}

// ไrZq
void PreprocessParseTree::visit(boost::shared_ptr<Equal> node)                 {assert(0);}
void PreprocessParseTree::visit(boost::shared_ptr<UnEqual> node)               {assert(0);}
void PreprocessParseTree::visit(boost::shared_ptr<Less> node)                  {assert(0);}
void PreprocessParseTree::visit(boost::shared_ptr<LessEqual> node)             {assert(0);}
void PreprocessParseTree::visit(boost::shared_ptr<Greater> node)               {assert(0);}
void PreprocessParseTree::visit(boost::shared_ptr<GreaterEqual> node)          {assert(0);}

// _Zq
void PreprocessParseTree::visit(boost::shared_ptr<LogicalAnd> node)            {assert(0);}
void PreprocessParseTree::visit(boost::shared_ptr<LogicalOr> node)             {assert(0);}
  
// Zp๑Zq
void PreprocessParseTree::visit(boost::shared_ptr<Plus> node)                  {assert(0);}
void PreprocessParseTree::visit(boost::shared_ptr<Subtract> node)              {assert(0);}
void PreprocessParseTree::visit(boost::shared_ptr<Times> node)                 {assert(0);}
void PreprocessParseTree::visit(boost::shared_ptr<Divide> node)                {assert(0);}
  
// ZpPZq
void PreprocessParseTree::visit(boost::shared_ptr<Negative> node)              {assert(0);}
void PreprocessParseTree::visit(boost::shared_ptr<Positive> node)              {assert(0);}
  
// ง๑Kw่`Zq
void PreprocessParseTree::visit(boost::shared_ptr<Weaker> node)                {assert(0);}
void PreprocessParseTree::visit(boost::shared_ptr<Parallel> node)              {assert(0);}

// Zq
void PreprocessParseTree::visit(boost::shared_ptr<Always> node)                {assert(0);}
  
// ๗ช
void PreprocessParseTree::visit(boost::shared_ptr<Differential> node)          {assert(0);}

// ถษภ
void PreprocessParseTree::visit(boost::shared_ptr<Previous> node)              {assert(0);}
  
// ฯ
void PreprocessParseTree::visit(boost::shared_ptr<Variable> node)              {assert(0);}

// 
void PreprocessParseTree::visit(boost::shared_ptr<Number> node)                {assert(0);}


} //namespace parse_tree
} //namespace hydla

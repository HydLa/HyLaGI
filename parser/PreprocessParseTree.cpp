#include "PreprocessParseTree.h"

namespace hydla { 
namespace parse_tree {

PreprocessParseTree::PreprocessParseTree()
{}

PreprocessParseTree::~PreprocessParseTree()
{}

// §–ñ’è‹`
void PreprocessParseTree::visit(boost::shared_ptr<ConstraintDefinition> node)  
{
  assert(0);
}

// ƒvƒƒOƒ‰ƒ€’è‹`
void PreprocessParseTree::visit(boost::shared_ptr<ProgramDefinition> node)     
{
  assert(0);
}

// §–ñŒÄ‚Ño‚µ
void PreprocessParseTree::visit(boost::shared_ptr<ConstraintCaller> node)      
{
  assert(0);
}

// ƒvƒƒOƒ‰ƒ€ŒÄ‚Ño‚µ
void PreprocessParseTree::visit(boost::shared_ptr<ProgramCaller> node)         
{
  assert(0);
}

// §–ñ®
void PreprocessParseTree::visit(boost::shared_ptr<Constraint> node)            
{
  assert(0);
}

// Ask§–ñ
void PreprocessParseTree::visit(boost::shared_ptr<Ask> node)                   
{
  assert(0);
}

// Tell§–ñ
void PreprocessParseTree::visit(boost::shared_ptr<Tell> node)                  
{
  dispatch_child(node);
}

// ”äŠr‰‰Zq
void PreprocessParseTree::visit(boost::shared_ptr<Equal> node)                 {assert(0);}
void PreprocessParseTree::visit(boost::shared_ptr<UnEqual> node)               {assert(0);}
void PreprocessParseTree::visit(boost::shared_ptr<Less> node)                  {assert(0);}
void PreprocessParseTree::visit(boost::shared_ptr<LessEqual> node)             {assert(0);}
void PreprocessParseTree::visit(boost::shared_ptr<Greater> node)               {assert(0);}
void PreprocessParseTree::visit(boost::shared_ptr<GreaterEqual> node)          {assert(0);}

// ˜_—‰‰Zq
void PreprocessParseTree::visit(boost::shared_ptr<LogicalAnd> node)            {assert(0);}
void PreprocessParseTree::visit(boost::shared_ptr<LogicalOr> node)             {assert(0);}
  
// Zp“ñ€‰‰Zq
void PreprocessParseTree::visit(boost::shared_ptr<Plus> node)                  {assert(0);}
void PreprocessParseTree::visit(boost::shared_ptr<Subtract> node)              {assert(0);}
void PreprocessParseTree::visit(boost::shared_ptr<Times> node)                 {assert(0);}
void PreprocessParseTree::visit(boost::shared_ptr<Divide> node)                {assert(0);}
  
// Zp’P€‰‰Zq
void PreprocessParseTree::visit(boost::shared_ptr<Negative> node)              {assert(0);}
void PreprocessParseTree::visit(boost::shared_ptr<Positive> node)              {assert(0);}
  
// §–ñŠK‘w’è‹`‰‰Zq
void PreprocessParseTree::visit(boost::shared_ptr<Weaker> node)                {assert(0);}
void PreprocessParseTree::visit(boost::shared_ptr<Parallel> node)              {assert(0);}

// ‘Š‰‰Zq
void PreprocessParseTree::visit(boost::shared_ptr<Always> node)                {assert(0);}
  
// ”÷•ª
void PreprocessParseTree::visit(boost::shared_ptr<Differential> node)          {assert(0);}

// ¶‹ÉŒÀ
void PreprocessParseTree::visit(boost::shared_ptr<Previous> node)              {assert(0);}
  
// •Ï”
void PreprocessParseTree::visit(boost::shared_ptr<Variable> node)              {assert(0);}

// ”š
void PreprocessParseTree::visit(boost::shared_ptr<Number> node)                {assert(0);}


} //namespace parse_tree
} //namespace hydla

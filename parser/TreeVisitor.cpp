#include "TreeVisitor.h"

#include <assert.h>

namespace hydla { 
namespace parse_tree {

TreeVisitor::TreeVisitor()
{}

TreeVisitor::~TreeVisitor()
{}

// ÄêµÁ
void TreeVisitor::visit(boost::shared_ptr<ConstraintDefinition> node)  {assert(0);}
void TreeVisitor::visit(boost::shared_ptr<ProgramDefinition> node)     {assert(0);}

// ¸Æ¤Ó½Ð¤·
void TreeVisitor::visit(boost::shared_ptr<ConstraintCaller> node)      {assert(0);}
void TreeVisitor::visit(boost::shared_ptr<ProgramCaller> node)         {assert(0);}

// À©Ìó¼°
void TreeVisitor::visit(boost::shared_ptr<Constraint> node)            {assert(0);}

// AskÀ©Ìó
void TreeVisitor::visit(boost::shared_ptr<Ask> node)                   {assert(0);}

// TellÀ©Ìó
void TreeVisitor::visit(boost::shared_ptr<Tell> node)                  {assert(0);}

// Èæ³Ó±é»»»Ò
void TreeVisitor::visit(boost::shared_ptr<Equal> node)                 {assert(0);}
void TreeVisitor::visit(boost::shared_ptr<UnEqual> node)               {assert(0);}
void TreeVisitor::visit(boost::shared_ptr<Less> node)                  {assert(0);}
void TreeVisitor::visit(boost::shared_ptr<LessEqual> node)             {assert(0);}
void TreeVisitor::visit(boost::shared_ptr<Greater> node)               {assert(0);}
void TreeVisitor::visit(boost::shared_ptr<GreaterEqual> node)          {assert(0);}

// ÏÀÍý±é»»»Ò
void TreeVisitor::visit(boost::shared_ptr<LogicalAnd> node)            {assert(0);}
void TreeVisitor::visit(boost::shared_ptr<LogicalOr> node)             {assert(0);}
  
// »»½ÑÆó¹à±é»»»Ò
void TreeVisitor::visit(boost::shared_ptr<Plus> node)                  {assert(0);}
void TreeVisitor::visit(boost::shared_ptr<Subtract> node)              {assert(0);}
void TreeVisitor::visit(boost::shared_ptr<Times> node)                 {assert(0);}
void TreeVisitor::visit(boost::shared_ptr<Divide> node)                {assert(0);}
  
// »»½ÑÃ±¹à±é»»»Ò
void TreeVisitor::visit(boost::shared_ptr<Negative> node)              {assert(0);}
void TreeVisitor::visit(boost::shared_ptr<Positive> node)              {assert(0);}
  
// À©Ìó³¬ÁØÄêµÁ±é»»»Ò
void TreeVisitor::visit(boost::shared_ptr<Weaker> node)                {assert(0);}
void TreeVisitor::visit(boost::shared_ptr<Parallel> node)              {assert(0);}

// »þÁê±é»»»Ò
void TreeVisitor::visit(boost::shared_ptr<Always> node)                {assert(0);}
  
// ÈùÊ¬
void TreeVisitor::visit(boost::shared_ptr<Differential> node)          {assert(0);}

// º¸¶Ë¸Â
void TreeVisitor::visit(boost::shared_ptr<Previous> node)              {assert(0);}
  
// ÊÑ¿ô
void TreeVisitor::visit(boost::shared_ptr<Variable> node)              {assert(0);}

// ¿ô»ú
void TreeVisitor::visit(boost::shared_ptr<Number> node)                {assert(0);}

} //namespace parse_tree
} //namespace hydla

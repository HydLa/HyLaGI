#include "TreeVisitor.h"

#include <assert.h>

namespace hydla { 
namespace parse_tree {

TreeVisitor::TreeVisitor()
{}

TreeVisitor::~TreeVisitor()
{}

// ��`
void TreeVisitor::visit(boost::shared_ptr<ConstraintDefinition> node)  {assert(0);}
void TreeVisitor::visit(boost::shared_ptr<ProgramDefinition> node)     {assert(0);}

// �Ăяo��
void TreeVisitor::visit(boost::shared_ptr<ConstraintCaller> node)      {assert(0);}
void TreeVisitor::visit(boost::shared_ptr<ProgramCaller> node)         {assert(0);}

// ����
void TreeVisitor::visit(boost::shared_ptr<Constraint> node)            {assert(0);}

// Ask����
void TreeVisitor::visit(boost::shared_ptr<Ask> node)                   {assert(0);}

// Tell����
void TreeVisitor::visit(boost::shared_ptr<Tell> node)                  {assert(0);}

// ��r���Z�q
void TreeVisitor::visit(boost::shared_ptr<Equal> node)                 {assert(0);}
void TreeVisitor::visit(boost::shared_ptr<UnEqual> node)               {assert(0);}
void TreeVisitor::visit(boost::shared_ptr<Less> node)                  {assert(0);}
void TreeVisitor::visit(boost::shared_ptr<LessEqual> node)             {assert(0);}
void TreeVisitor::visit(boost::shared_ptr<Greater> node)               {assert(0);}
void TreeVisitor::visit(boost::shared_ptr<GreaterEqual> node)          {assert(0);}

// �_�����Z�q
void TreeVisitor::visit(boost::shared_ptr<LogicalAnd> node)            {assert(0);}
void TreeVisitor::visit(boost::shared_ptr<LogicalOr> node)             {assert(0);}
  
// �Z�p�񍀉��Z�q
void TreeVisitor::visit(boost::shared_ptr<Plus> node)                  {assert(0);}
void TreeVisitor::visit(boost::shared_ptr<Subtract> node)              {assert(0);}
void TreeVisitor::visit(boost::shared_ptr<Times> node)                 {assert(0);}
void TreeVisitor::visit(boost::shared_ptr<Divide> node)                {assert(0);}
void TreeVisitor::visit(boost::shared_ptr<Power> node)                 {assert(0);}
  
// �Z�p�P�����Z�q
void TreeVisitor::visit(boost::shared_ptr<Negative> node)              {assert(0);}
void TreeVisitor::visit(boost::shared_ptr<Positive> node)              {assert(0);}
  
// ����K�w��`���Z�q
void TreeVisitor::visit(boost::shared_ptr<Weaker> node)                {assert(0);}
void TreeVisitor::visit(boost::shared_ptr<Parallel> node)              {assert(0);}

// �������Z�q
void TreeVisitor::visit(boost::shared_ptr<Always> node)                {assert(0);}
  
// ����
void TreeVisitor::visit(boost::shared_ptr<Differential> node)          {assert(0);}

// ���Ɍ�
void TreeVisitor::visit(boost::shared_ptr<Previous> node)              {assert(0);}

// �ے�
void TreeVisitor::visit(boost::shared_ptr<Not> node)              {assert(0);}
  
// �ϐ�
void TreeVisitor::visit(boost::shared_ptr<Variable> node)              {assert(0);}

// ����
void TreeVisitor::visit(boost::shared_ptr<Number> node)                {assert(0);}

// �L���萔
void TreeVisitor::visit(boost::shared_ptr<Parameter> node)                {assert(0);}

// t
void TreeVisitor::visit(boost::shared_ptr<SymbolicT> node)                {assert(0);}

} //namespace parse_tree
} //namespace hydla

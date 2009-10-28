#ifndef _INCLUDED_HYDLA_PARSE_TREE_TREE_VISITOR_H_
#define _INCLUDED_HYDLA_PARSE_TREE_TREE_VISITOR_H_

#include <boost/shared_ptr.hpp>

#include "Node.h"

namespace hydla { 
namespace parse_tree {
  
/**
 * ParseTree�̃m�[�h�W���ɑ΂���Visitor�N���X
 */
class TreeVisitor {
public:
  TreeVisitor()
  {}

  virtual ~TreeVisitor()
  {}

  // ��`
  virtual void visit(ConstraintDefinition* node)  {}
  virtual void visit(ProgramDefinition* node)     {}

  // �Ăяo��
  virtual void visit(ConstraintCaller* node)      {}
  virtual void visit(ProgramCaller* node)         {}

  // ����
  virtual void visit(Constraint* node)            {}

  // Ask����
  virtual void visit(Ask* node)                   {}

  // Tell����
  virtual void visit(Tell* node)                  {}

  // ��r���Z�q
  virtual void visit(Equal* node)                 {}
  virtual void visit(UnEqual* node)               {}
  virtual void visit(Less* node)                  {}
  virtual void visit(LessEqual* node)             {}
  virtual void visit(Greater* node)               {}
  virtual void visit(GreaterEqual* node)          {}

  // �_�����Z�q
  virtual void visit(LogicalAnd* node)            {}
  virtual void visit(LogicalOr* node)             {}
  
  // �Z�p�񍀉��Z�q
  virtual void visit(Plus* node)                  {}
  virtual void visit(Subtract* node)              {}
  virtual void visit(Times* node)                 {}
  virtual void visit(Divide* node)                {}
  
  // �Z�p�P�����Z�q
  virtual void visit(Negative* node)              {}
  virtual void visit(Positive* node)              {}
  
  // ����K�w��`���Z�q
  virtual void visit(Weaker* node)                {}
  virtual void visit(Parallel* node)              {}

  // �������Z�q
  virtual void visit(Always* node)                {}
  
  // ����
  virtual void visit(Differential* node)          {}

  // ���Ɍ�
  virtual void visit(Previous* node)              {}
  
  // �ϐ�
  virtual void visit(Variable* node)              {}

  // ����
  virtual void visit(Number* node)                {}

};

} //namespace parse_tree
} //namespace hydla

#endif //_INCLUDED_HYDLA_PARSE_TREE_TREE_VISITOR_H_
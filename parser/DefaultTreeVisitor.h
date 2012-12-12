#ifndef _INCLUDED_HYDLA_PARSE_TREE_DEFAULT_TREE_VISITOR_H_
#define _INCLUDED_HYDLA_PARSE_TREE_DEFAULT_TREE_VISITOR_H_


#include "TreeVisitor.h"

namespace hydla { 
namespace parse_tree {
  
/**
 * �e�m�[�h�ɑ΂��ăf�t�H���g�̓���i�S�m�[�h�������邪�C�����ύX���Ȃ��j���s���w���p�N���X�D
 * ���̃N���X���p�����邱�ƂŁC�u�ꕔ�̃m�[�h�ɑ΂��Ă̂ݏ������s���N���X�v�����₷���Ȃ�͂��D
 * ���������ꂪ�]�܂��������Ȃ̂��͂悭�������Ă��Ȃ� by matsusho
 */
class DefaultTreeVisitor: public TreeVisitor {
public:
  DefaultTreeVisitor();

  virtual ~DefaultTreeVisitor();

  // �����`
  virtual void visit(boost::shared_ptr<ConstraintDefinition> node);
  
  // �v���O������`
  virtual void visit(boost::shared_ptr<ProgramDefinition> node);

  // ����Ăяo��
  virtual void visit(boost::shared_ptr<ConstraintCaller> node);
  
  // �v���O�����Ăяo��
  virtual void visit(boost::shared_ptr<ProgramCaller> node);

  // ����
  virtual void visit(boost::shared_ptr<Constraint> node);

  // Ask����
  virtual void visit(boost::shared_ptr<Ask> node);

  // Tell����
  virtual void visit(boost::shared_ptr<Tell> node);

  // ��r���Z�q
  virtual void visit(boost::shared_ptr<Equal> node);
  virtual void visit(boost::shared_ptr<UnEqual> node);
  virtual void visit(boost::shared_ptr<Less> node);
  virtual void visit(boost::shared_ptr<LessEqual> node);
  virtual void visit(boost::shared_ptr<Greater> node);
  virtual void visit(boost::shared_ptr<GreaterEqual> node);

  // �_�����Z�q
  virtual void visit(boost::shared_ptr<LogicalAnd> node);
  virtual void visit(boost::shared_ptr<LogicalOr> node);
  
  // �Z�p�񍀉��Z�q
  virtual void visit(boost::shared_ptr<Plus> node);
  virtual void visit(boost::shared_ptr<Subtract> node);
  virtual void visit(boost::shared_ptr<Times> node);
  virtual void visit(boost::shared_ptr<Divide> node);
  virtual void visit(boost::shared_ptr<Power> node);
  
  // �Z�p�P�����Z�q
  virtual void visit(boost::shared_ptr<Negative> node);
  virtual void visit(boost::shared_ptr<Positive> node);
  
  // ����K�w��`���Z�q
  virtual void visit(boost::shared_ptr<Weaker> node);
  virtual void visit(boost::shared_ptr<Parallel> node);

  // �������Z�q
  virtual void visit(boost::shared_ptr<Always> node);
  
  // ����
  virtual void visit(boost::shared_ptr<Differential> node);

  // ���Ɍ�
  virtual void visit(boost::shared_ptr<Previous> node);
  
  //Print
  virtual void visit(boost::shared_ptr<Print> node);
  virtual void visit(boost::shared_ptr<PrintPP> node);
  virtual void visit(boost::shared_ptr<PrintIP> node);
    
  virtual void visit(boost::shared_ptr<Scan> node);
  virtual void visit(boost::shared_ptr<Exit> node);
  virtual void visit(boost::shared_ptr<Abort> node);

  //SystemVariable
  virtual void visit(boost::shared_ptr<SVtimer> node);

  // �ے�
  virtual void visit(boost::shared_ptr<Not> node);
  
  // �~����
  virtual void visit(boost::shared_ptr<Pi> node);
  // ���R�ΐ��̒�
  virtual void visit(boost::shared_ptr<E> node);
  
  //�֐�
  virtual void visit(boost::shared_ptr<Function> node);
  virtual void visit(boost::shared_ptr<UnsupportedFunction> node);


  // �ϐ�
  virtual void visit(boost::shared_ptr<Variable> node);

  // ����
  virtual void visit(boost::shared_ptr<Number> node);
  
  // �L���萔
  virtual void visit(boost::shared_ptr<Parameter> node);
  
  // t�i���ԁj
  virtual void visit(boost::shared_ptr<SymbolicT> node);
};

} //namespace parse_tree
} //namespace hydla

#endif //_INCLUDED_HYDLA_PARSE_TREE_DEFAULT_TREE_VISITOR_H_

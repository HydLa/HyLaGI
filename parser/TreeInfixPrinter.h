#ifndef TREE_INFIX__PRINTER_H_
#define TREE_INFIX__PRINTER_H_


//�c���[�𒆒u�L�@�ŏo�͂���N���X

#include "Node.h"
#include "DefaultTreeVisitor.h"

namespace hydla {
namespace parse_tree {


class TreeInfixPrinter:
  public DefaultTreeVisitor
{
  public:
  typedef enum{
    PAR_NONE,
    PAR_N,
    PAR_N_P_S,
    PAR_N_P_S_T_D_P,
  }needParenthesis;

  /**
   * �m�[�h�������Ɏ��C���l�L�@�`���ŏo�͂���
   */
  std::ostream& print_infix(const node_sptr &, std::ostream&);
  /**
   * �m�[�h�������Ɏ��C���l�L�@�`���ɂ��ĕԂ�
   */
  std::string get_infix_string(const node_sptr &);

  private:
  
  needParenthesis need_par_;
  std::ostream *output_stream_;
  
  void print_binary_node(const BinaryNode &, const std::string &symbol,
                          const needParenthesis &pre_par = PAR_NONE, const needParenthesis &post_par = PAR_NONE);
  void print_unary_node(const UnaryNode &, const std::string &pre, const std::string &post);

  void print_factor_node(const FactorNode &, const std::string &pre, const std::string &post);

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

  
  // ����K�w��`���Z�q
  virtual void visit(boost::shared_ptr<Weaker> node);
  virtual void visit(boost::shared_ptr<Parallel> node);

  // �������Z�q
  virtual void visit(boost::shared_ptr<Always> node);


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

  // �ے�
  virtual void visit(boost::shared_ptr<Not> node);
  
  // �ϐ�
  virtual void visit(boost::shared_ptr<Variable> node);

  // ����
  virtual void visit(boost::shared_ptr<Number> node);

  // �L���萔
  virtual void visit(boost::shared_ptr<Parameter> node);

  // t
  virtual void visit(boost::shared_ptr<SymbolicT> node);

  // ������
  virtual void visit(boost::shared_ptr<Infinity> node);
  
  // ���R�ΐ��̒�
  virtual void visit(boost::shared_ptr<E> node);
  
  // �~����
  virtual void visit(boost::shared_ptr<Pi> node);
  
  
  // �C��
  virtual void visit(boost::shared_ptr<Function> node);
  virtual void visit(boost::shared_ptr<UnsupportedFunction> node);
};

} // namespace parse_tree
} // namespace hydla 

#endif // TREE_INFIX__PRINTER_H_

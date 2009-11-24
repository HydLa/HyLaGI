#ifndef _INCLUDED_HYDLA_PREPROCESS_PARSE_TREE_H_
#define _INCLUDED_HYDLA_PREPROCESS_PARSE_TREE_H_

#include <stack>

#include "Node.h"
#include "TreeVisitor.h"

namespace hydla { 
namespace parse_tree {

class PreprocessParseTree : public TreeVisitor
{
public:
  PreprocessParseTree();
  virtual ~PreprocessParseTree();

  // ��`
  virtual void visit(boost::shared_ptr<ConstraintDefinition> node);
  virtual void visit(boost::shared_ptr<ProgramDefinition> node);

  // �Ăяo��
  virtual void visit(boost::shared_ptr<ConstraintCaller> node);
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
  
  // �ϐ�
  virtual void visit(boost::shared_ptr<Variable> node);

  // ����
  virtual void visit(boost::shared_ptr<Number> node);

private:
  // ��`�̌^
  typedef std::string                             difinition_name_t;
  typedef int                                     bound_variable_count_t;
  typedef std::pair<difinition_name_t, 
                    bound_variable_count_t>       difinition_type_t;

  // �����`
  typedef boost::shared_ptr<ConstraintDefinition> constraint_def_map_value_t;
  typedef std::map<difinition_type_t,
                   constraint_def_map_value_t>    constraint_def_map_t;

  // �v���O������`
  typedef boost::shared_ptr<ProgramDefinition>    program_def_map_value_t;
  typedef std::map<difinition_type_t,
                   program_def_map_value_t>       program_def_map_t;

  typedef std::set<difinition_type_t>             referenced_definition_list_t;

  typedef std::map<std::string, node_sptr>        formal_arg_map_t;

  typedef std::map<std::string, int>              variable_map_t;

  /**
   * �T�����̃m�[�h�c���[�̏�Ԃ�ۑ����邽�߂̍\����
   */
  struct State {
    /// �K�[�h�̒����ǂ���
    bool in_guard;

    /// ���񎮂̒����ǂ���
    bool in_constraint;

    /// always����̗L���͈͓����ǂ���
    bool in_always;

    /// �����L����ʉ߂�����
    /// �ϐ��ɓ��B�����ہA���̒l�����̕ϐ��ɑ΂�������̍ő��
    int differential_count;

    /// �W�J���ꂽ��`�̃��X�g 
    referenced_definition_list_t referenced_definition_list;

    /// �������Ƃ���ɑΉ�����������m�[�h�̑Ή��\  
    formal_arg_map_t formal_arg_map;
  };

  /// State���ނ��߂̃X�^�b�N
  std::stack<State> state_stack_;

  /// �����`�̕\
  constraint_def_map_t constraint_def_map_;

  /// �v���O������`�̕\
  program_def_map_t program_def_map_;
  
  /// �v���O�������Ŏg�p�����ϐ��̕\
  variable_map_t variable_map_;
  
  /**
   * �V�����q�m�[�h
   * accept��A����ɒl�������Ă���ꍇ�̓m�[�h�̒l����������
   */
  node_sptr new_child_;
  
  /**
   * �w�肵���m�[�h���Ăяo���A
   * new_child_�ɒl���ݒ肳��Ă����ꍇ�A�q�m�[�h�����ւ���
   */
  template<class C, 
           const node_sptr& (C::*getter)() const,
           void (C::*setter)(const node_sptr& child)>
  void dispatch(C* n) 
  {
    accept((n->*getter)());
    if(new_child_) {
      (n->*setter)(new_child_);
      new_child_.reset();
    }
  }

  
  template<class NodeType>
  void dispatch_child(NodeType& node)
  {
    dispatch<UnaryNode, &UnaryNode::get_child, &UnaryNode::set_child>(node.get());
  }

  template<class NodeType>
  void dispatch_rhs(NodeType& node)
  {
    dispatch<BinaryNode, &BinaryNode::get_rhs, &BinaryNode::set_rhs>(node.get());
  }

  template<class NodeType>
  void dispatch_lhs(NodeType& node)
  {
    dispatch<BinaryNode, &BinaryNode::get_lhs, &BinaryNode::set_lhs>(node.get());
  }

};

} //namespace parse_tree
} //namespace hydla

#endif //_INCLUDED_HYDLA_PREPROCESS_PARSE_TREE_H_

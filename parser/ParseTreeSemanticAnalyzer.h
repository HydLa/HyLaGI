#ifndef _INCLUDED_HYDLA_PARSE_TREE_SEMANTIC_ANALYZER_H_
#define _INCLUDED_HYDLA_PARSE_TREE_SEMANTIC_ANALYZER_H_

#include <stack>
#include <iostream>

#include "Node.h"
#include "TreeVisitor.h"
#include "DefinitionContainer.h"
#include "ParseTree.h"

namespace hydla { 
namespace parser {

class ParseTreeSemanticAnalyzer : 
  public hydla::parse_tree::TreeVisitor
{
public:
  typedef hydla::parse_tree::node_sptr                 node_sptr;
  typedef hydla::parse_tree::ParseTree::variable_map_t variable_map_t;

  ParseTreeSemanticAnalyzer(
    DefinitionContainer<hydla::parse_tree::ConstraintDefinition>& constraint_definition,
    DefinitionContainer<hydla::parse_tree::ProgramDefinition>&    program_definition,
    hydla::parse_tree::ParseTree* parse_tree);
  
  virtual ~ParseTreeSemanticAnalyzer();

  /**
   * ��͂���ѐ���Ăяo���̓W�J�������Ȃ�
   */
  void analyze(node_sptr& n/*, variable_map_t& variable_map*/);

  // ��`
  virtual void visit(boost::shared_ptr<hydla::parse_tree::ConstraintDefinition> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::ProgramDefinition> node);

  // �Ăяo��
  virtual void visit(boost::shared_ptr<hydla::parse_tree::ConstraintCaller> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::ProgramCaller> node);

  // ����
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Constraint> node);

  // Ask����
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Ask> node);

  // Tell����
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Tell> node);

  // ��r���Z�q
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Equal> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::UnEqual> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Less> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::LessEqual> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Greater> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::GreaterEqual> node);

  // �_�����Z�q
  virtual void visit(boost::shared_ptr<hydla::parse_tree::LogicalAnd> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::LogicalOr> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Not> node);
  
  // �Z�p�񍀉��Z�q
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Plus> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Subtract> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Times> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Divide> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Power> node);
  
  // �Z�p�P�����Z�q
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Negative> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Positive> node);
  
  // ����K�w��`���Z�q
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Weaker> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Parallel> node);

  // �������Z�q
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Always> node);
  
  // ����
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Differential> node);

  // ���Ɍ�
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Previous> node);
  
  // �֐�
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Function> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::UnsupportedFunction> node);

  // �~����
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Pi> node);
  // ���R�ΐ��̒�
  virtual void visit(boost::shared_ptr<hydla::parse_tree::E> node);
  
  
  // �ϐ�
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Variable> node);

  // ����
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Number> node);

  // Print
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Print> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::PrintPP> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::PrintIP> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Scan> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Exit> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Abort> node);

private:
  typedef hydla::parser::DefinitionContainer<
    hydla::parse_tree::Definition>::definition_map_key_t referenced_definition_t;

  typedef std::set<referenced_definition_t>         referenced_definition_list_t;
  
  typedef std::map<std::string, node_sptr>        formal_arg_map_t;

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
  
  /// �v���O�������Ŏg�p�����ϐ��̈ꗗ
  variable_map_t* variable_map_;
  
  /**
   * �V�����q�m�[�h
   * accept��A����ɒl�������Ă���ꍇ�̓m�[�h�̒l����������
   */
  node_sptr new_child_;

  /**
   * �����`�̏��
   */
  DefinitionContainer<hydla::parse_tree::ConstraintDefinition>& 
    constraint_definition_;
    
  /**
   * �v���O������`�̏��
   */
  DefinitionContainer<hydla::parse_tree::ProgramDefinition>&    
    program_definition_;

  hydla::parse_tree::ParseTree* parse_tree_;
  
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
    dispatch<hydla::parse_tree::UnaryNode, 
      &hydla::parse_tree::UnaryNode::get_child, 
      &hydla::parse_tree::UnaryNode::set_child>(node.get());
  }

  template<class NodeType>
  void dispatch_rhs(NodeType& node)
  {
    dispatch<hydla::parse_tree::BinaryNode, 
      &hydla::parse_tree::BinaryNode::get_rhs, 
      &hydla::parse_tree::BinaryNode::set_rhs>(node.get());
  }

  template<class NodeType>
  void dispatch_lhs(NodeType& node)
  {
    dispatch<hydla::parse_tree::BinaryNode, 
      &hydla::parse_tree::BinaryNode::get_lhs, 
      &hydla::parse_tree::BinaryNode::set_lhs>(node.get());
  }

  /**
   * ��`�̊Ȗ�(�W�J)�������Ȃ�
   */
  node_sptr apply_definition(
    const referenced_definition_t& def_type,
    boost::shared_ptr<hydla::parse_tree::Caller> caller, 
    boost::shared_ptr<hydla::parse_tree::Definition> definition);
};

} //namespace parser
} //namespace hydla

#endif //_INCLUDED_HYDLA_PARSE_TREE_SEMANTIC_ANALYZER_H_

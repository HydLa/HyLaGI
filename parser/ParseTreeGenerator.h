#ifndef _INCLUDED_HYDLA_PARSER_PARSE_TREE_GENERATOR_H_
#define _INCLUDED_HYDLA_PARSER_PARSE_TREE_GENERATOR_H_

#include <boost/shared_ptr.hpp>

#include "Node.h"
#include "HydLaGrammarRule.h"
#include "ParseTree.h"

namespace hydla {
namespace parser {

template<typename NodeFactory>
class ParseTreeGenerator
{
public:

  /**
   * AST������ParseTree���\�z����
   */
  template<typename TreeIter>
  boost::shared_ptr<hydla::parse_tree::ParseTree>
    generate(const TreeIter& tree_iter)
  {
    pt_.reset(new hydla::parse_tree::ParseTree);

    create_parse_tree(tree_iter);

    return pt_;
  }


private:

  /**
   * �w�肵���^�̃m�[�h���쐬����
   */
  template<typename NodeType>
  boost::shared_ptr<NodeType>
  create_node()
  {
    return NodeFactory()(NodeType());
  }

  /**
   * ������1�Ƃ�w�肵���^�̃m�[�h���쐬����
   */
  template<typename NodeType, typename TreeIter>
  boost::shared_ptr<NodeType>
  create_unary_node(const TreeIter& tree_iter)
  {
    boost::shared_ptr<NodeType> n(create_node<NodeType>());
    n->set_child(create_parse_tree(tree_iter));
    return n;
  }
    
  /**
   * ������2�Ƃ�w�肵���^�̃m�[�h���쐬����
   */
  template<typename NodeType, typename TreeIter> 
  boost::shared_ptr<NodeType>  
  create_binary_node(const TreeIter& tree_iter)
  { 
    boost::shared_ptr<NodeType> n(create_node<NodeType>());
    n->set_lhs(create_parse_tree(tree_iter)); 
    n->set_rhs(create_parse_tree(tree_iter+1)); 
    return n; 
  } 

  /**
   * �w�肵���^�̒�`�m�[�h���쐬����
   */
  template<typename NodeType, typename TreeIter>
  boost::shared_ptr<NodeType>  
  create_definition(const TreeIter& tree_iter)
  {
    boost::shared_ptr<NodeType> node(create_node<NodeType>());

    TreeIter gch = tree_iter->children.begin();

    //��`��
    node->set_name(
      std::string(gch->value.begin(), gch->value.end()));

    //������
    if (tree_iter->children.size()>1) {
      TreeIter args = (gch+1)->children.begin();
      size_t bound_variable_count = (gch+1)->children.size();
      for(size_t i=0; i<bound_variable_count; i++) {
        node->add_bound_variable(
          std::string((args+i)->value.begin(), (args+i)->value.end()));
      }
    }

    node->set_child(create_parse_tree(tree_iter+1));
    return node;
  }
  
  /**
   * �w�肵���^�̌Ăяo���m�[�h���쐬����
   */
  template<typename NodeType, typename TreeIter>
  boost::shared_ptr<NodeType>  
  create_caller(const TreeIter& tree_iter)
  {
    boost::shared_ptr<NodeType> node(create_node<NodeType>());

    //�v���O������
    node->set_name(    
      std::string(tree_iter->value.begin(), tree_iter->value.end()));

    //������
    if (tree_iter->children.size()>1) {
      TreeIter args = (tree_iter+1)->children.begin();
      size_t bound_variable_count = (tree_iter+1)->children.size();
      for(size_t i=0; i<bound_variable_count; i++) {
        node->add_actual_arg(create_parse_tree(args+i));
      }
    }
    return node;
  }
  
  /**
   * ParseTree���\�z����
   */
  template<typename TreeIter>
  boost::shared_ptr<hydla::parse_tree::Node>
  create_parse_tree(const TreeIter& tree_iter)
  {
    using namespace hydla::grammer_rule;
    using namespace hydla::parse_tree;

    TreeIter ch = tree_iter->children.begin();

    long node_id = tree_iter->value.id().to_long();
    switch(node_id) 
    {
      // �v���O�����̏W��
      case RI_Statements:
      {
        node_sptr node_tree;
        TreeIter it  = tree_iter->children.begin();
        TreeIter end = tree_iter->children.end();
        while(it != end) {
          if(node_sptr new_tree = create_parse_tree(it++)) {
            // ����錾��������o�������ꍇ�́u,�v�ɂ���ĘA������
            if(node_tree) {
              boost::shared_ptr<Parallel> rn(create_node<Parallel>());
              rn->set_lhs(node_tree);
              rn->set_rhs(new_tree);
              node_tree = rn;
            } else {
              node_tree = new_tree;
            }
          }
        }
        pt_->set_tree(node_tree);
        return node_sptr();
      }

      // �����`
      case RI_ConstraintDef:
      {
        pt_->addConstraintDefinition(
          create_definition<ConstraintDefinition>(ch));
        return node_sptr();
      }

      // �v���O������`
      case RI_ProgramDef:
      {
        pt_->addProgramDefinition(
          create_definition<ProgramDefinition>(ch));
        return node_sptr();
      }
      
      // ����Ăяo��
      case RI_ConstraintCaller:
      {
        return create_caller<ConstraintCaller>(ch);
      }
      
      // �v���O�����Ăяo��
      case RI_ProgramCaller:
      {
        return create_caller<ProgramCaller>(ch);
      }

      // ask����
      case RI_Implies:
      {
        boost::shared_ptr<Ask> node(create_node<Ask>());

        // �K�[�h����
        node->set_guard(create_parse_tree(ch));

        // �q����
        node->set_child(create_parse_tree(ch+1));

        return node;
      }

      // Tell����
      case RI_Tell:         {return create_unary_node<Tell>(ch);}
      
        // ����
      case RI_Constraint:   {return create_unary_node<Constraint>(ch);}

        // ��r���Z�q
      case RI_Equal:        {return create_binary_node<Equal>(ch);}
      case RI_UnEqual:      {return create_binary_node<UnEqual>(ch);}
      case RI_Less:         {return create_binary_node<Less>(ch);}
      case RI_LessEqual:    {return create_binary_node<LessEqual>(ch);}
      case RI_Greater:      {return create_binary_node<Greater>(ch);}
      case RI_GreaterEqual: {return create_binary_node<GreaterEqual>(ch);}
    
        // �_�����Z�q
      case RI_LogicalAnd:   {return create_binary_node<LogicalAnd>(ch);}
      case RI_LogicalOr:    {return create_binary_node<LogicalOr>(ch);}
      
        // �Z�p�񍀉��Z�q
      case RI_Plus:         {return create_binary_node<Plus>(ch);}
      case RI_Subtract:     {return create_binary_node<Subtract>(ch);}
      case RI_Times:        {return create_binary_node<Times>(ch);}
      case RI_Divide:       {return create_binary_node<Divide>(ch);}

        // �Z�p�P�����Z�q
      case RI_Negative:     {return create_unary_node<Negative>(ch);}
      case RI_Positive:     {return create_unary_node<Positive>(ch);}

        // ����K�w��`���Z�q
      case RI_Weaker:       {return create_binary_node<Weaker>(ch);}
      case RI_Parallel:     {return create_binary_node<Parallel>(ch);}

        // �������Z�q
      case RI_Always:       {return create_unary_node<Always>(ch);}

        // ����
      case RI_Differential: {return create_unary_node<Differential>(ch);}
    
        // ���Ɍ�
      case RI_Previous:     {return create_unary_node<Previous>(ch);}

        // �ϐ��E�����ϐ�
      case RI_BoundVariable:
      case RI_Variable:
      {
        boost::shared_ptr<Variable> node(create_node<Variable>());
        node->set_name(
          std::string(tree_iter->value.begin(), tree_iter->value.end()));
        return node;
      }

      // ����
      case RI_Number:
      {
        boost::shared_ptr<Number> node(create_node<Number>());
        node->set_number(
          std::string(tree_iter->value.begin(), tree_iter->value.end()));     
        return node;
      }

      default:
      {
        assert(0);
        return node_sptr();
      }  
    }  
  }

  boost::shared_ptr<hydla::parse_tree::ParseTree> pt_;
};

} // namespace parser
} // namespace hydla

#endif // _INCLUDED_HYDLA_PARSER_PARSE_TREE_GENERATOR_H_

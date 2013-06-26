#ifndef _INCLUDED_HYDLA_PARSER_NODE_TREE_GENERATOR_H_
#define _INCLUDED_HYDLA_PARSER_NODE_TREE_GENERATOR_H_

#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>

#include "Node.h"
#include "HydLaGrammarRule.h"
#include "ParseTree.h"
#include "NodeFactory.h"
#include "DefinitionContainer.h"
#include "ParseError.h"

namespace hydla {
namespace parser {

class NodeTreeGenerator
{
public:
  typedef hydla::parse_tree::node_sptr node_sptr;

  NodeTreeGenerator(
    node_sptr&    assertion_node,
    DefinitionContainer<hydla::parse_tree::ConstraintDefinition>& constraint_definition,
    DefinitionContainer<hydla::parse_tree::ProgramDefinition>&    program_definition,
    const boost::shared_ptr<NodeFactory>& node_factory) :
    
    assertion_node_(assertion_node),
    constraint_definition_(constraint_definition),
    program_definition_(program_definition),
    node_factory_(node_factory)
  {}

  /**
   * AST������ParseTree���\�z����
   */
  template<typename TreeIter>
  node_sptr generate(const TreeIter& tree_iter)
  {
    constraint_definition_.clear();
    program_definition_.clear();

    return create_parse_tree(tree_iter);
  }
  
private:

  /**
   * ������1�Ƃ�w�肵���^�̃m�[�h���쐬����
   */
  template<typename NodeType, typename TreeIter>
  boost::shared_ptr<NodeType>
  create_unary_node(const TreeIter& tree_iter)
  {
    boost::shared_ptr<NodeType> n(node_factory_->create<NodeType>());
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
    boost::shared_ptr<NodeType> n(node_factory_->create<NodeType>());
    n->set_lhs(create_parse_tree(tree_iter)); 
    n->set_rhs(create_parse_tree(tree_iter+1)); 
    return n; 
  }
  
  
  /**
   * ������C�Ӑ����w�肵���^�̃m�[�h���쐬����
   */
  template<typename NodeType, typename TreeIter> 
  boost::shared_ptr<NodeType>  
  create_arbitrary_node(const TreeIter& tree_iter)
  { 
    boost::shared_ptr<NodeType> node(node_factory_->create<NodeType>());
    std::string name(tree_iter->value.begin(), tree_iter->value.end());
    node->set_string(name);
    TreeIter it  = tree_iter->children.begin();
    TreeIter end = tree_iter->children.end();
    while(it != end) {
      node->add_argument(create_parse_tree(it++));
    }
    return node;
  }

  /**
   * �w�肵���^�̒�`�m�[�h���쐬����
   */
  template<typename NodeType, typename TreeIter>
  boost::shared_ptr<NodeType>  
  create_definition(const TreeIter& tree_iter)
  {
    boost::shared_ptr<NodeType> node(node_factory_->create<NodeType>());

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
    TreeIter ch = tree_iter->children.begin();
    boost::shared_ptr<NodeType> node(node_factory_->create<NodeType>());

    //�v���O������
    node->set_name(    
      std::string(ch->value.begin(), ch->value.end()));

    //������
    if (tree_iter->children.size()==2) {
      TreeIter it  = (ch+1)->children.begin();
      TreeIter end = (ch+1)->children.end();
      for(; it!=end; ++it) {
        node->add_actual_arg(create_parse_tree(it));
      }
    }
    return node;
  }
  
  /**
   * ��r���Z�q��Ԃ�
   */
  template<typename TreeIter>
  boost::shared_ptr<hydla::parse_tree::BinaryNode>
  get_comp_node(const TreeIter& tree_iter)
  {
    using namespace hydla::grammer_rule;
    using namespace hydla::parse_tree;
    long node_id = tree_iter->value.id().to_long();
    switch(node_id) 
    {
        // ��r���Z�q
      case RI_Equal:        {return node_factory_->create<Equal>();}
      case RI_UnEqual:      {return node_factory_->create<UnEqual>();}
      case RI_Less:         {return node_factory_->create<Less>();}
      case RI_LessEqual:    {return node_factory_->create<LessEqual>();}
      case RI_Greater:      {return node_factory_->create<Greater>();}
      case RI_GreaterEqual: {return node_factory_->create<GreaterEqual>();}
      default: assert(0); return node_factory_->create<Equal>();
    }
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
              boost::shared_ptr<Parallel> rn(node_factory_->create<Parallel>());
              rn->set_lhs(node_tree);
              rn->set_rhs(new_tree);
              node_tree = rn;
            } else {
              node_tree = new_tree;
            }
          }
        }
        return node_tree;
      }

      // �����`
      case RI_ConstraintDef:
        {
          // ��`�m�[�h�̍쐬
          boost::shared_ptr<ConstraintDefinition> d(
            create_definition<ConstraintDefinition>(ch));

          // ���ɐ���/�v���O������`�Ƃ��Ē�`����Ă��Ȃ����ǂ���
          if(constraint_definition_.is_registered(d) ||
             program_definition_.is_registered(d)) {
            throw hydla::parse_error::MultipleDefinition(d->get_name()); 
          }

          // ��`�m�[�h�̓o�^
          constraint_definition_.add_definition(d);

          return node_sptr();
        }

      // �v���O������`
      case RI_ProgramDef:
      {
          // ��`�m�[�h�̍쐬
          boost::shared_ptr<ProgramDefinition> d(
            create_definition<ProgramDefinition>(ch));

          // ���ɐ���/�v���O������`�Ƃ��Ē�`����Ă��Ȃ����ǂ���
          if(constraint_definition_.is_registered(d) ||
             program_definition_.is_registered(d)) {
            throw hydla::parse_error::MultipleDefinition(d->get_name()); 
          }

          // ��`�m�[�h�̓o�^
          program_definition_.add_definition(d);

          return node_sptr();
      }
      
      // ����Ăяo��
      case RI_ConstraintCaller:
      {
        return create_caller<ConstraintCaller>(tree_iter);
      }
      
      // �v���O�����Ăяo��
      case RI_ProgramCaller:
      {
        return create_caller<ProgramCaller>(tree_iter);
      }
      
      
      // �i�A���j����
      case RI_Chain:
      {
        node_sptr node_tree;
        TreeIter it  = tree_iter->children.begin();
        TreeIter end = tree_iter->children.end();
        node_sptr lhs_exp, rhs_exp;
        assert(it!=end);
        lhs_exp = create_parse_tree(it++);
        while(it != end) {
          boost::shared_ptr<BinaryNode> comp_op = get_comp_node(it++);
          assert(it!=end);
          rhs_exp = create_parse_tree(it++);
          comp_op->set_lhs(lhs_exp);
          comp_op->set_rhs(rhs_exp);

          // 2�ȏ�̔�r���Z�q�łȂ����Ă���ꍇ�́A�_���ςłȂ���
          if(node_tree) {
            boost::shared_ptr<LogicalAnd> and_node(node_factory_->create<LogicalAnd>());
            and_node->set_lhs(node_tree);
            and_node->set_rhs(comp_op);
            node_tree = and_node;
          } else {
            node_tree = comp_op;
          }
          lhs_exp = rhs_exp;
        }
        return node_tree;
      }

      // Tell����
      case RI_Tell:         {return create_unary_node<Tell>(ch);}

      // Ask����
      case RI_Implies:      {return create_binary_node<Ask>(ch);}
      
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
      case RI_LogicalNot:    {return create_unary_node<Not>(ch);}
      
        // �Z�p�񍀉��Z�q
      case RI_Plus:         {return create_binary_node<Plus>(ch);}
      case RI_Subtract:     {return create_binary_node<Subtract>(ch);}
      case RI_Times:        {return create_binary_node<Times>(ch);}
      case RI_Divide:       {return create_binary_node<Divide>(ch);}
      case RI_Power:        {return create_binary_node<Power>(ch);}

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
      
        // �֐�
      case RI_Function:
      {
        boost::shared_ptr<Function> node = create_arbitrary_node<Function>(tree_iter);
        return node;
      }
        // �T�|�[�g�O�֐�
      case RI_UnsupportedFunction:          
      {
        boost::shared_ptr<UnsupportedFunction> node = create_arbitrary_node<UnsupportedFunction>(tree_iter);
        return node;
      }
      

        // �~����
      case RI_Pi:
      {
        boost::shared_ptr<Pi> node(node_factory_->create<Pi>());
        return node;
      }
      
     
        // ���R�ΐ��̒�
      case RI_E:          
      {
        boost::shared_ptr<E> node(node_factory_->create<E>());
        return node;
      }
      
        // �ϐ��E�����ϐ�
      case RI_BoundVariable:
      case RI_Variable:
      {
        boost::shared_ptr<Variable> node(node_factory_->create<Variable>());
        node->set_name(
          std::string(tree_iter->value.begin(), tree_iter->value.end()));
        return node;
      }

      // ����
      case RI_Number:
      {
        std::string str(tree_iter->value.begin(), tree_iter->value.end());
        std::string::size_type sz = str.find(".");
        if(sz == std::string::npos){
          boost::shared_ptr<Number> node(node_factory_->create<Number>());
          node->set_number(str);
          return node;
        }else{
          //�����Ȃ�C�����ɕϊ�����
          std::string denominator("1");
          for(unsigned int i=0; i<str.size()-(sz+1);i++){
            denominator += "0";
          }
          str = str.substr(0, sz) + str.substr(sz+1);
          boost::shared_ptr<Number> num_node(node_factory_->create<Number>());
          num_node->set_number(str);
          boost::shared_ptr<Number> den_node(node_factory_->create<Number>());
          den_node->set_number(denominator);
          boost::shared_ptr<Divide> node(node_factory_->create<Divide>());
          node->set_lhs(num_node);
          node->set_rhs(den_node);
          return node;
        }
      }
      
      case RI_Assert:
      {
        //assertion�͂P����
        assert(!assertion_node_);
        
        assertion_node_ = create_parse_tree(ch);
        return node_sptr();
      }
      
      case RI_Command://wada
      {
        std::string command_name(tree_iter->value.begin(), tree_iter->value.end());
        boost::shared_ptr<IONode> io_node;
        if(command_name == "PRINTPP"){
          io_node = node_factory_->create<PrintPP>();
          assert(ch != tree_iter->children.end());
          std::string str(ch->value.begin(), ch->value.end());
          io_node->set_string(
          std::string(str, 0, str.find(",", 0)));
          io_node->set_args(
          std::string(str, str.find(",", 0), str.length()));
        } else if( command_name == "PRINTIP"){
          io_node = node_factory_->create<PrintIP>();
          assert(ch != tree_iter->children.end());
          std::string str(ch->value.begin(), ch->value.end());
          io_node->set_string(
          std::string(str, 0, str.find(",", 0)));
          io_node->set_args(
          std::string(str, str.find(",", 0), str.length()));
        }else if( command_name == "SCAN"){
          io_node = node_factory_->create<Scan>();
          assert(ch != tree_iter->children.end());
          std::string str(ch->value.begin(), ch->value.end());
          io_node->set_string(str);
          io_node->set_args(str);
        }else{
            throw hydla::parse_error::InvalidCommand(command_name); 
        }
        return io_node;
      }

	  //SystemVarible
	  case RI_SVtimer: 
	  {
        boost::shared_ptr<SVtimer> node(node_factory_->create<SVtimer>());
        return node;
	  }
      
      default:
      {
        assert(0);
        return node_sptr();
      }  
    }
  }
  
  
  node_sptr& assertion_node_;
  DefinitionContainer<hydla::parse_tree::ConstraintDefinition>& constraint_definition_;
  DefinitionContainer<hydla::parse_tree::ProgramDefinition>&    program_definition_;


  boost::shared_ptr<NodeFactory>   node_factory_;
};

} // namespace parser
} // namespace hydla

#endif // _INCLUDED_HYDLA_PARSER_NODE_TREE_GENERATOR_H_

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
    DefinitionContainer<hydla::parse_tree::ConstraintDefinition>& constraint_definition,
    DefinitionContainer<hydla::parse_tree::ProgramDefinition>&    program_definition,
    node_sptr&    assertion_node,
    const boost::shared_ptr<NodeFactory>& node_factory) :
    
    constraint_definition_(constraint_definition),
    program_definition_(program_definition),
    assertion_node_(assertion_node),
    node_factory_(node_factory)
  {}

  /**
   * AST‚ğŒ³‚ÉParseTree‚ğ\’z‚·‚é
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
   * ˆø”‚ğ1‚Â‚Æ‚éw’è‚µ‚½Œ^‚Ìƒm[ƒh‚ğì¬‚·‚é
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
   * ˆø”‚ğ2‚Â‚Æ‚éw’è‚µ‚½Œ^‚Ìƒm[ƒh‚ğì¬‚·‚é
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
   * w’è‚µ‚½Œ^‚Ì’è‹`ƒm[ƒh‚ğì¬‚·‚é
   */
  template<typename NodeType, typename TreeIter>
  boost::shared_ptr<NodeType>  
  create_definition(const TreeIter& tree_iter)
  {
    boost::shared_ptr<NodeType> node(node_factory_->create<NodeType>());

    TreeIter gch = tree_iter->children.begin();

    //’è‹`–¼
    node->set_name(
      std::string(gch->value.begin(), gch->value.end()));

    //‰¼ˆø”
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
   * w’è‚µ‚½Œ^‚ÌŒÄ‚Ño‚µƒm[ƒh‚ğì¬‚·‚é
   */
  template<typename NodeType, typename TreeIter>
  boost::shared_ptr<NodeType>  
  create_caller(const TreeIter& tree_iter)
  {
    TreeIter ch = tree_iter->children.begin();
    boost::shared_ptr<NodeType> node(node_factory_->create<NodeType>());

    //ƒvƒƒOƒ‰ƒ€–¼
    node->set_name(    
      std::string(ch->value.begin(), ch->value.end()));

    //Àˆø”
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
   * ParseTree‚ğ\’z‚·‚é
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
      // ƒvƒƒOƒ‰ƒ€‚ÌW‡
      case RI_Statements:
      {
        node_sptr node_tree;
        TreeIter it  = tree_iter->children.begin();
        TreeIter end = tree_iter->children.end();
        while(it != end) {
          if(node_sptr new_tree = create_parse_tree(it++)) {
            // §–ñéŒ¾‚ª•¡”‰ñoŒ»‚µ‚½ê‡‚Íu,v‚É‚æ‚Á‚Ä˜AŒ‹‚·‚é
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

      // §–ñ’è‹`
      case RI_ConstraintDef:
        {
          // ’è‹`ƒm[ƒh‚Ìì¬
          boost::shared_ptr<ConstraintDefinition> d(
            create_definition<ConstraintDefinition>(ch));

          // Šù‚É§–ñ/ƒvƒƒOƒ‰ƒ€’è‹`‚Æ‚µ‚Ä’è‹`‚³‚ê‚Ä‚¢‚È‚¢‚©‚Ç‚¤‚©
          if(constraint_definition_.is_registered(d) ||
             program_definition_.is_registered(d)) {
            throw hydla::parse_error::MultipleDefinition(d->get_name()); 
          }

          // ’è‹`ƒm[ƒh‚Ì“o˜^
          constraint_definition_.add_definition(d);

          return node_sptr();
        }

      // ƒvƒƒOƒ‰ƒ€’è‹`
      case RI_ProgramDef:
      {
          // ’è‹`ƒm[ƒh‚Ìì¬
          boost::shared_ptr<ProgramDefinition> d(
            create_definition<ProgramDefinition>(ch));

          // Šù‚É§–ñ/ƒvƒƒOƒ‰ƒ€’è‹`‚Æ‚µ‚Ä’è‹`‚³‚ê‚Ä‚¢‚È‚¢‚©‚Ç‚¤‚©
          if(constraint_definition_.is_registered(d) ||
             program_definition_.is_registered(d)) {
            throw hydla::parse_error::MultipleDefinition(d->get_name()); 
          }

          // ’è‹`ƒm[ƒh‚Ì“o˜^
          program_definition_.add_definition(d);

          return node_sptr();
      }
      
      // §–ñŒÄ‚Ño‚µ
      case RI_ConstraintCaller:
      {
        return create_caller<ConstraintCaller>(tree_iter);
      }
      
      // ƒvƒƒOƒ‰ƒ€ŒÄ‚Ño‚µ
      case RI_ProgramCaller:
      {
        return create_caller<ProgramCaller>(tree_iter);
      }

      // Tell§–ñ
      case RI_Tell:         {return create_unary_node<Tell>(ch);}

      // Ask§–ñ
      case RI_Implies:      {return create_binary_node<Ask>(ch);}
      
        // §–ñ®
      case RI_Constraint:   {return create_unary_node<Constraint>(ch);}

        // ”äŠr‰‰Zq
      case RI_Equal:        {return create_binary_node<Equal>(ch);}
      case RI_UnEqual:      {return create_binary_node<UnEqual>(ch);}
      case RI_Less:         {return create_binary_node<Less>(ch);}
      case RI_LessEqual:    {return create_binary_node<LessEqual>(ch);}
      case RI_Greater:      {return create_binary_node<Greater>(ch);}
      case RI_GreaterEqual: {return create_binary_node<GreaterEqual>(ch);}
    
        // ˜_—‰‰Zq
      case RI_LogicalAnd:   {return create_binary_node<LogicalAnd>(ch);}
      case RI_LogicalOr:    {return create_binary_node<LogicalOr>(ch);}
      case RI_LogicalNot:    {return create_unary_node<Not>(ch);}
      
        // Zp“ñ€‰‰Zq
      case RI_Plus:         {return create_binary_node<Plus>(ch);}
      case RI_Subtract:     {return create_binary_node<Subtract>(ch);}
      case RI_Times:        {return create_binary_node<Times>(ch);}
      case RI_Divide:       {return create_binary_node<Divide>(ch);}
      case RI_Power:        {return create_binary_node<Power>(ch);}

        // Zp’P€‰‰Zq
      case RI_Negative:     {return create_unary_node<Negative>(ch);}
      case RI_Positive:     {return create_unary_node<Positive>(ch);}

        // §–ñŠK‘w’è‹`‰‰Zq
      case RI_Weaker:       {return create_binary_node<Weaker>(ch);}
      case RI_Parallel:     {return create_binary_node<Parallel>(ch);}

        // ‘Š‰‰Zq
      case RI_Always:       {return create_unary_node<Always>(ch);}

        // ”÷•ª
      case RI_Differential: {return create_unary_node<Differential>(ch);}
    
        // ¶‹ÉŒÀ
      case RI_Previous:     {return create_unary_node<Previous>(ch);}
      
        // OŠpŠÖ”
      case RI_Sin:          {return create_unary_node<Sin>(ch);}
      case RI_Cos:          {return create_unary_node<Cos>(ch);}
      case RI_Tan:          {return create_unary_node<Tan>(ch);}
        // ‹tOŠpŠÖ”
      case RI_Asin:          {return create_unary_node<Asin>(ch);}
      case RI_Acos:          {return create_unary_node<Acos>(ch);}
      case RI_Atan:          {return create_unary_node<Atan>(ch);}

        // ‰~ü—¦
      case RI_Pi:
      {
        boost::shared_ptr<Pi> node(node_factory_->create<Pi>());
        return node;
      }
      
      
        // ‘Î”
      case RI_Log:          {return create_binary_node<Log>(ch);}
      case RI_Ln:           {return create_unary_node<Ln>(ch);}

        // ©‘R‘Î”‚Ì’ê
      case RI_E:          
      {
        boost::shared_ptr<E> node(node_factory_->create<E>());
        return node;
      }
      
      
        // ”CˆÓ‚Ì•¶š—ñ
      case RI_ArbitraryBinary:          
      {
        boost::shared_ptr<ArbitraryBinary> node = create_binary_node<ArbitraryBinary>(ch);
        node->set_string(
          std::string(tree_iter->value.begin(), tree_iter->value.end()));
        return node;
      }

      case RI_ArbitraryUnary:
      {
        boost::shared_ptr<ArbitraryUnary> node = create_unary_node<ArbitraryUnary>(ch);
        node->set_string(
          std::string(tree_iter->value.begin(), tree_iter->value.end()));
        return node;
      }

      case RI_ArbitraryFactor:
      {
        boost::shared_ptr<ArbitraryFactor> node(node_factory_->create<ArbitraryFactor>());
        node->set_string(
          std::string(tree_iter->value.begin(), tree_iter->value.end()));
        return node;
      }
      
        // •Ï”E‘©”›•Ï”
      case RI_BoundVariable:
      case RI_Variable:
      {
        boost::shared_ptr<Variable> node(node_factory_->create<Variable>());
        node->set_name(
          std::string(tree_iter->value.begin(), tree_iter->value.end()));
        return node;
      }

      // ”š
      case RI_Number:
      {
        boost::shared_ptr<Number> node(node_factory_->create<Number>());
        node->set_number(
          std::string(tree_iter->value.begin(), tree_iter->value.end()));     
        return node;
      }
      
      case RI_Assert:
      {
        //assertion‚Í‚P‚Â‚¾‚¯
        assert(!assertion_node_);
        
        assertion_node_ = create_parse_tree(ch);
        return node_sptr();
      }
      case RI_Print://wada
      {
        boost::shared_ptr<Print> node(node_factory_->create<Print>());
        std::string str(tree_iter->value.begin(), tree_iter->value.end());
        node->set_string(
        std::string(str, 0, str.find("\",", 0)));
        node->set_args(
        std::string(str, str.find("\",", 0), str.length()));
       return node;
       // return create_unary_node<Print>(ch);
      }
      case RI_Print_PP://wada
      {
        boost::shared_ptr<PrintPP> node(node_factory_->create<PrintPP>());
        std::string str(tree_iter->value.begin(), tree_iter->value.end());
        node->set_string(
        std::string(str, 0, str.find("\",", 0)));
        node->set_args(
        std::string(str, str.find("\",", 0), str.length()));
       return node;
       // return create_unary_node<Print>(ch);
      }
      case RI_Print_IP://wada
      {
        boost::shared_ptr<PrintIP> node(node_factory_->create<PrintIP>());
        std::string str(tree_iter->value.begin(), tree_iter->value.end());
        node->set_string(
        std::string(str, 0, str.find("\",", 0)));
        node->set_args(
        std::string(str, str.find("\",", 0), str.length()));
       return node;
       // return create_unary_node<Print>(ch);
      }
      case RI_Scan://wada
      {
        boost::shared_ptr<Scan> node(node_factory_->create<Scan>());
        std::string str(tree_iter->value.begin(), tree_iter->value.end());
        node->set_string(
        std::string(str));
        node->set_args(
        std::string(str));
       return node;
       // return create_unary_node<Print>(ch);
      }
      case RI_Exit://wada
      {
        boost::shared_ptr<Exit> node(node_factory_->create<Exit>());
        std::string str(tree_iter->value.begin(), tree_iter->value.end());
        node->set_string(
        std::string(str));
        node->set_args(
        std::string(str));
       return node;
       // return create_unary_node<Print>(ch);
      }
      case RI_Abort://wada
      {
        boost::shared_ptr<Abort> node(node_factory_->create<Abort>());
        std::string str(tree_iter->value.begin(), tree_iter->value.end());
        node->set_string(
        std::string(str, 0, str.find("\",", 0)));
        node->set_args(
        std::string(str, str.find("\",", 0), str.length()));
       return node;
       // return create_unary_node<Print>(ch);
      }
      default://a
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

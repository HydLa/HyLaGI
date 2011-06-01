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
    const boost::shared_ptr<NodeFactory>& node_factory) :
    
    constraint_definition_(constraint_definition),
    program_definition_(program_definition),
    node_factory_(node_factory)
  {}

  /**
   * ASTを元にParseTreeを構築する
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
   * 引数を1つとる指定した型のノードを作成する
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
   * 引数を2つとる指定した型のノードを作成する
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
   * 指定した型の定義ノードを作成する
   */
  template<typename NodeType, typename TreeIter>
  boost::shared_ptr<NodeType>  
  create_definition(const TreeIter& tree_iter)
  {
    boost::shared_ptr<NodeType> node(node_factory_->create<NodeType>());

    TreeIter gch = tree_iter->children.begin();

    //定義名
    node->set_name(
      std::string(gch->value.begin(), gch->value.end()));

    //仮引数
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
   * 指定した型の呼び出しノードを作成する
   */
  template<typename NodeType, typename TreeIter>
  boost::shared_ptr<NodeType>  
  create_caller(const TreeIter& tree_iter)
  {
    TreeIter ch = tree_iter->children.begin();
    boost::shared_ptr<NodeType> node(node_factory_->create<NodeType>());

    //プログラム名
    node->set_name(    
      std::string(ch->value.begin(), ch->value.end()));

    //実引数
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
   * ParseTreeを構築する
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
      // プログラムの集合
      case RI_Statements:
      {
        node_sptr node_tree;
        TreeIter it  = tree_iter->children.begin();
        TreeIter end = tree_iter->children.end();
        while(it != end) {
          if(node_sptr new_tree = create_parse_tree(it++)) {
            // 制約宣言が複数回出現した場合は「,」によって連結する
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

      // 制約定義
      case RI_ConstraintDef:
        {
          // 定義ノードの作成
          boost::shared_ptr<ConstraintDefinition> d(
            create_definition<ConstraintDefinition>(ch));

          // 既に制約/プログラム定義として定義されていないかどうか
          if(constraint_definition_.is_registered(d) ||
             program_definition_.is_registered(d)) {
            throw hydla::parse_error::MultipleDefinition(d->get_name()); 
          }

          // 定義ノードの登録
          constraint_definition_.add_definition(d);

          return node_sptr();
        }

      // プログラム定義
      case RI_ProgramDef:
      {
          // 定義ノードの作成
          boost::shared_ptr<ProgramDefinition> d(
            create_definition<ProgramDefinition>(ch));

          // 既に制約/プログラム定義として定義されていないかどうか
          if(constraint_definition_.is_registered(d) ||
             program_definition_.is_registered(d)) {
            throw hydla::parse_error::MultipleDefinition(d->get_name()); 
          }

          // 定義ノードの登録
          program_definition_.add_definition(d);

          return node_sptr();
      }
      
      // 制約呼び出し
      case RI_ConstraintCaller:
      {
        return create_caller<ConstraintCaller>(tree_iter);
      }
      
      // プログラム呼び出し
      case RI_ProgramCaller:
      {
        return create_caller<ProgramCaller>(tree_iter);
      }

      // Tell制約
      case RI_Tell:         {return create_unary_node<Tell>(ch);}

      // Ask制約
      case RI_Implies:      {return create_binary_node<Ask>(ch);}
      
        // 制約式
      case RI_Constraint:   {return create_unary_node<Constraint>(ch);}

        // 比較演算子
      case RI_Equal:        {return create_binary_node<Equal>(ch);}
      case RI_UnEqual:      {return create_binary_node<UnEqual>(ch);}
      case RI_Less:         {return create_binary_node<Less>(ch);}
      case RI_LessEqual:    {return create_binary_node<LessEqual>(ch);}
      case RI_Greater:      {return create_binary_node<Greater>(ch);}
      case RI_GreaterEqual: {return create_binary_node<GreaterEqual>(ch);}
    
        // 論理演算子
      case RI_LogicalAnd:   {return create_binary_node<LogicalAnd>(ch);}
      case RI_LogicalOr:    {return create_binary_node<LogicalOr>(ch);}
      
        // 算術二項演算子
      case RI_Plus:         {return create_binary_node<Plus>(ch);}
      case RI_Subtract:     {return create_binary_node<Subtract>(ch);}
      case RI_Times:        {return create_binary_node<Times>(ch);}
      case RI_Divide:       {return create_binary_node<Divide>(ch);}
      case RI_Power:        {return create_binary_node<Power>(ch);}

        // 算術単項演算子
      case RI_Negative:     {return create_unary_node<Negative>(ch);}
      case RI_Positive:     {return create_unary_node<Positive>(ch);}

        // 制約階層定義演算子
      case RI_Weaker:       {return create_binary_node<Weaker>(ch);}
      case RI_Parallel:     {return create_binary_node<Parallel>(ch);}

        // 時相演算子
      case RI_Always:       {return create_unary_node<Always>(ch);}

        // 微分
      case RI_Differential: {return create_unary_node<Differential>(ch);}
    
        // 左極限
      case RI_Previous:     {return create_unary_node<Previous>(ch);}
        // 直線のPPの値
      case RI_PreviousPoint:     {return create_unary_node<PreviousPoint>(ch);}

        // 変数・束縛変数
      case RI_BoundVariable:
      case RI_Variable:
      {
        boost::shared_ptr<Variable> node(node_factory_->create<Variable>());
        node->set_name(
          std::string(tree_iter->value.begin(), tree_iter->value.end()));
        return node;
      }

      // 数字
      case RI_Number:
      {
        boost::shared_ptr<Number> node(node_factory_->create<Number>());
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

  DefinitionContainer<hydla::parse_tree::ConstraintDefinition>& constraint_definition_;
  DefinitionContainer<hydla::parse_tree::ProgramDefinition>&    program_definition_;


  boost::shared_ptr<NodeFactory>   node_factory_;
};

} // namespace parser
} // namespace hydla

#endif // _INCLUDED_HYDLA_PARSER_NODE_TREE_GENERATOR_H_

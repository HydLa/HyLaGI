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
   * ASTを元にParseTreeを構築する
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
   * 指定した型のノードを作成する
   */
  template<typename NodeType>
  boost::shared_ptr<NodeType>
  create_node()
  {
    return NodeFactory()(NodeType());
  }

  /**
   * 引数を1つとる指定した型のノードを作成する
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
   * 引数を2つとる指定した型のノードを作成する
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
   * 指定した型の定義ノードを作成する
   */
  template<typename NodeType, typename TreeIter>
  boost::shared_ptr<NodeType>  
  create_definition(const TreeIter& tree_iter)
  {
    boost::shared_ptr<NodeType> node(create_node<NodeType>());

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
    boost::shared_ptr<NodeType> node(create_node<NodeType>());

    //プログラム名
    node->set_name(    
      std::string(tree_iter->value.begin(), tree_iter->value.end()));

    //実引数
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

      // 制約定義
      case RI_ConstraintDef:
      {
        pt_->addConstraintDefinition(
          create_definition<ConstraintDefinition>(ch));
        return node_sptr();
      }

      // プログラム定義
      case RI_ProgramDef:
      {
        pt_->addProgramDefinition(
          create_definition<ProgramDefinition>(ch));
        return node_sptr();
      }
      
      // 制約呼び出し
      case RI_ConstraintCaller:
      {
        return create_caller<ConstraintCaller>(ch);
      }
      
      // プログラム呼び出し
      case RI_ProgramCaller:
      {
        return create_caller<ProgramCaller>(ch);
      }

      // ask制約
      case RI_Implies:
      {
        boost::shared_ptr<Ask> node(create_node<Ask>());

        // ガード条件
        node->set_guard(create_parse_tree(ch));

        // 子制約
        node->set_child(create_parse_tree(ch+1));

        return node;
      }

      // Tell制約
      case RI_Tell:         {return create_unary_node<Tell>(ch);}
      
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

        // 変数・束縛変数
      case RI_BoundVariable:
      case RI_Variable:
      {
        boost::shared_ptr<Variable> node(create_node<Variable>());
        node->set_name(
          std::string(tree_iter->value.begin(), tree_iter->value.end()));
        return node;
      }

      // 数字
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

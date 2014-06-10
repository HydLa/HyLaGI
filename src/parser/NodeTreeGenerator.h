#pragma once

#include <boost/shared_ptr.hpp>

#include "Node.h"
#include "HydLaGrammarRule.h"
#include "DefinitionContainer.h"
#include "ParseError.h"
#include "Utility.h"

using namespace hydla::utility;

namespace hydla {
namespace parser {

class NodeTreeGenerator
{
public:
  typedef symbolic_expression::node_sptr node_sptr;

  /**
   * default constructor
   * (use temporary ones)
   */
  NodeTreeGenerator():
    temporary_constraint_definition_(new DefinitionContainer<symbolic_expression::ConstraintDefinition>()),
    temporary_program_definition_(new DefinitionContainer<symbolic_expression::ProgramDefinition>()),
    assertion_node_(temporary_assertion_node_),
    constraint_definition_(*temporary_constraint_definition_),
    program_definition_(*temporary_program_definition_)
  {}

  NodeTreeGenerator(
    symbolic_expression::node_sptr&    assertion_node,
    DefinitionContainer<symbolic_expression::ConstraintDefinition>& constraint_definition,
    DefinitionContainer<symbolic_expression::ProgramDefinition>&    program_definition):
    temporary_constraint_definition_(nullptr),
    temporary_program_definition_(nullptr),
    assertion_node_(assertion_node),
    constraint_definition_(constraint_definition),
    program_definition_(program_definition)
  {}

  ~NodeTreeGenerator()
    {
      if(temporary_constraint_definition_ != nullptr)
      {
        delete temporary_constraint_definition_;
      }

      if(temporary_program_definition_ != nullptr)
      {
        delete temporary_program_definition_;
      }
    }

  /**
   * ASTを元にNodeTreeを構築する
   */
  template<typename TreeIter>
  symbolic_expression::node_sptr generate(const TreeIter& tree_iter)
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
    boost::shared_ptr<NodeType> n(new NodeType());
    
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
    boost::shared_ptr<NodeType> n(new NodeType());
    n->set_lhs(create_parse_tree(tree_iter)); 
    n->set_rhs(create_parse_tree(tree_iter+1)); 
    return n; 
  }
  
  
  /**
   * 引数を任意数取る指定した型のノードを作成する
   */
  template<typename NodeType, typename TreeIter> 
  boost::shared_ptr<NodeType>  
  create_arbitrary_node(const TreeIter& tree_iter)
  { 
    boost::shared_ptr<NodeType> node(new NodeType());
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
   * 指定した型の定義ノードを作成する
   */
  template<typename NodeType, typename TreeIter>
  boost::shared_ptr<NodeType>  
  create_definition(const TreeIter& tree_iter)
  {
    boost::shared_ptr<NodeType> node(new NodeType());

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
    boost::shared_ptr<NodeType> node(new NodeType());

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
   * 比較演算子を返す
   */
  template<typename TreeIter>
  boost::shared_ptr<symbolic_expression::BinaryNode>
  get_comp_node(const TreeIter& tree_iter)
  {
    using namespace grammer_rule;
    using namespace symbolic_expression;
    long node_id = tree_iter->value.id().to_long();
    switch(node_id) 
    {
        // 比較演算子
    case RI_Equal:        {return boost::shared_ptr<BinaryNode>(new Equal());}
    case RI_UnEqual:      {return boost::shared_ptr<BinaryNode>(new UnEqual());}
    case RI_Less:         {return boost::shared_ptr<BinaryNode>(new Less());}
    case RI_LessEqual:    {return boost::shared_ptr<BinaryNode>(new LessEqual());}
    case RI_Greater:      {return boost::shared_ptr<BinaryNode>(new Greater());}
    case RI_GreaterEqual: {return boost::shared_ptr<BinaryNode>(new GreaterEqual());}
    default: assert(0); return boost::shared_ptr<BinaryNode>(new Equal());
    }
  }
  
  /**
   * NodeTreeを構築する
   */
  template<typename TreeIter>
  boost::shared_ptr<symbolic_expression::Node>
  create_parse_tree(const TreeIter& tree_iter)
  {
    using namespace grammer_rule;
    using namespace symbolic_expression;

    TreeIter ch = tree_iter->children.begin();

    long node_id = tree_iter->value.id().to_long();
    switch(node_id) 
    {
      // プログラムの集合
      case RI_Statements:
      {
        symbolic_expression::node_sptr node_tree;
        TreeIter it  = tree_iter->children.begin();
        TreeIter end = tree_iter->children.end();
        while(it != end) {
          if(symbolic_expression::node_sptr new_tree = create_parse_tree(it++)) {
            // 制約宣言が複数回出現した場合は「,」によって連結する
            if(node_tree) {
              boost::shared_ptr<Parallel> rn(new Parallel());
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
            throw parser::error::MultipleDefinition(d->get_name()); 
          }

          // 定義ノードの登録
          constraint_definition_.add_definition(d);

          return symbolic_expression::node_sptr();
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
            throw parser::error::MultipleDefinition(d->get_name()); 
          }

          // 定義ノードの登録
          program_definition_.add_definition(d);

          return symbolic_expression::node_sptr();
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
      
      
      // （連鎖）制約
      case RI_Chain:
      {
        symbolic_expression::node_sptr node_tree;
        TreeIter it  = tree_iter->children.begin();
        TreeIter end = tree_iter->children.end();
        symbolic_expression::node_sptr lhs_exp, rhs_exp;
        assert(it!=end);
        lhs_exp = create_parse_tree(it++);
        while(it != end) {
          boost::shared_ptr<BinaryNode> comp_op = get_comp_node(it++);
          assert(it!=end);
          rhs_exp = create_parse_tree(it++);
          comp_op->set_lhs(lhs_exp);
          comp_op->set_rhs(rhs_exp);

          // 2つ以上の比較演算子でつながっている場合は、論理積でつなげる
          if(node_tree) {
            boost::shared_ptr<LogicalAnd> and_node(new LogicalAnd());
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
      case RI_LogicalNot:    {return create_unary_node<Not>(ch);}
      
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
      
        // 関数
      case RI_Function:
      {
        boost::shared_ptr<Function> node = create_arbitrary_node<Function>(tree_iter);
        return node;
      }
        // サポート外関数
      case RI_UnsupportedFunction:          
      {
        boost::shared_ptr<UnsupportedFunction> node = create_arbitrary_node<UnsupportedFunction>(tree_iter);
        return node;
      }
      

        // 円周率
      case RI_Pi:
      {
        boost::shared_ptr<Pi> node(new Pi());
        return node;
      }
      
     
        // 自然対数の底
      case RI_E:          
      {
        boost::shared_ptr<E> node(new E());
        return node;
      }
      
        // 変数・束縛変数
      case RI_BoundVariable:
      case RI_Variable:
      {
        std::string name(tree_iter->value.begin(), tree_iter->value.end());
        if(name == "t")
        {
          boost::shared_ptr<SymbolicT> node(new SymbolicT());
          return node;
        }
        boost::shared_ptr<Variable> node(new Variable());
        node->set_name(name);
        return node;
      }

      case RI_Parameter:
      {
        if(tree_iter->children.size() != 3)throw error::InvalidParameter();        
        TreeIter name_it = tree_iter->children.begin();
        std::string name(name_it->value.begin(), name_it->value.end());
        TreeIter diff_it = ++name_it;
        int diff = atoi(std::string(diff_it->value.begin(), diff_it->value.end()).c_str());
        TreeIter id_it = ++diff_it;
        int phase_id = atoi(std::string(id_it->value.begin(), id_it->value.end()).c_str());
        boost::shared_ptr<Parameter> node(new Parameter(name, diff, phase_id));
        return node;
      }

      case RI_Integer:
      {
        std::string str(tree_iter->value.begin(), tree_iter->value.end());
        boost::shared_ptr<Number> node(new Number());
        node->set_number(str);
        return node;
      }

      // 数字
      case RI_Number:
      {
        std::string str(tree_iter->value.begin(), tree_iter->value.end());
        std::string numerator;
        std::string denominator;
        bool fraction = num_denom_str(str, numerator, denominator);
        if(!fraction){
          boost::shared_ptr<Number> node(new Number());
          node->set_number(str);
          return node;
        }else if(fraction){
          boost::shared_ptr<Number> num_node(new Number());
          num_node->set_number(numerator);
          boost::shared_ptr<Number> den_node(new Number());
          den_node->set_number(denominator);
          boost::shared_ptr<Divide> node(new Divide());
          node->set_lhs(num_node);
          node->set_rhs(den_node);
          return node;
        }
      }
      
      case RI_Assert:
      {
        //assertionは１つだけ
        assert(!assertion_node_);
        
        assertion_node_ = create_parse_tree(ch);
        return symbolic_expression::node_sptr();
      }
      
      case RI_Command://wada
      {
        std::string command_name(tree_iter->value.begin(), tree_iter->value.end());
        boost::shared_ptr<IONode> io_node;
        if(command_name == "PRINTPP"){
          io_node = boost::shared_ptr<IONode>(new PrintPP());
          assert(ch != tree_iter->children.end());
          std::string str(ch->value.begin(), ch->value.end());
          io_node->set_string(
          std::string(str, 0, str.find(",", 0)));
          io_node->set_args(
          std::string(str, str.find(",", 0), str.length()));
        } else if( command_name == "PRINTIP"){
          io_node = boost::shared_ptr<IONode>(new PrintIP());
          assert(ch != tree_iter->children.end());
          std::string str(ch->value.begin(), ch->value.end());
          io_node->set_string(
          std::string(str, 0, str.find(",", 0)));
          io_node->set_args(
          std::string(str, str.find(",", 0), str.length()));
        }else if( command_name == "SCAN"){
          io_node = boost::shared_ptr<IONode>(new Scan());
          assert(ch != tree_iter->children.end());
          std::string str(ch->value.begin(), ch->value.end());
          io_node->set_string(str);
          io_node->set_args(str);
        }else{
            throw parser::error::InvalidCommand(command_name); 
        }
        return io_node;
      }

	  //SystemVarible
      case RI_SVtimer: 
      {
        boost::shared_ptr<SVtimer> node(new SVtimer());
        return node;
      }
      
      case RI_True:
      {
        boost::shared_ptr<True> node(new True());
	return node;
      }

      case RI_SymbolicT:
      {
        boost::shared_ptr<SymbolicT> node(new SymbolicT()); 
        return node;
      }
      
      default:
      {
        assert(0);
        return symbolic_expression::node_sptr();
      }  
    }
  }  

  symbolic_expression::node_sptr temporary_assertion_node_;
  DefinitionContainer<symbolic_expression::ConstraintDefinition>* temporary_constraint_definition_;
  DefinitionContainer<symbolic_expression::ProgramDefinition>*    temporary_program_definition_;
  
  symbolic_expression::node_sptr& assertion_node_;
  DefinitionContainer<symbolic_expression::ConstraintDefinition>& constraint_definition_;
  DefinitionContainer<symbolic_expression::ProgramDefinition>&    program_definition_;

};

} // namespace parser
} // namespace hydla

#include <iostream>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <algorithm>
#include <assert.h>

#include <boost/bind.hpp>

#include "HydLaParser.h"
#include "HydLaGrammarRule.h"
#include "ParseError.h"
#include "PreprocessParseTree.h"

using namespace std;
using namespace boost;

using namespace hydla::parse_tree;
using namespace hydla::grammer_rule;

namespace hydla {
namespace parser {

HydLaParser::HydLaParser(node_factory_sptr node_factory) :
  debug_dump_(false),
  node_factory_(node_factory)
{
}
  
HydLaParser::HydLaParser(node_factory_sptr node_factory, 
                         bool debug_dump) :
  debug_dump_(debug_dump),
  node_factory_(node_factory)
{
}
  

HydLaParser::~HydLaParser()
{
}

void HydLaParser::parse(std::istream& s) 
{
  clear_tree();
 
  if(debug_dump_ ) std::cout << "#*** AST Tree ***\n";
  create_ast(s);  
  if(debug_dump_ ) dump_ast(std::cout);

  if(debug_dump_ ) std::cout << "#*** Parse Tree ***\n";
  create_parse_tree();
  if(debug_dump_ ) dump_parse_tree(std::cout);

  if(debug_dump_ ) std::cout << "#*** Preprocessed Parse Tree ***\n";
  PreprocessParseTree ppt;
  ppt.start(&parse_tree_);

  //execute_preprocess();
  if(debug_dump_ ) dump_parse_tree(std::cout);

  if(debug_dump_ ) {
    std::cout << "#*** Variables ***\n";
    const variable_map_t &variable_map = parse_tree_.get_variable_map();
    variable_map_t::const_iterator it  = variable_map.begin();
    variable_map_t::const_iterator end = variable_map.end();
    for(; it!=end; it++) {
      cout << it->first << ":" << it->second << endl;
    }
  }
}

void HydLaParser::parse_flie(const std::string& filename) 
{
  ifstream in(filename.c_str());
  if (!in) {    
    throw std::runtime_error(string("cannot open \"") + filename + "\"");
  }

  parse(in);
}

void HydLaParser::parse_string(const std::string& str)
{    
  stringstream in(str);
  parse(in);
}

std::ostream& HydLaParser::dump_ast(std::ostream& outstream, 
                                    tree_iter_t iter, 
                                    int nest)
{
  for(int i=0; i<nest; i++) {
    outstream << "  "; // スペース２個分インデント
  }

#define OUT_ID_NAME(X) case X: outstream << " " << #X; break

  long id = iter->value.id().to_long();
  outstream << "ID:" << id;
  switch(id) {
    OUT_ID_NAME(RI_Constraint);
    OUT_ID_NAME(RI_ConstraintCaller);
    OUT_ID_NAME(RI_ConstraintCallee);
    OUT_ID_NAME(RI_ProgramCaller);
    OUT_ID_NAME(RI_ProgramCallee);
    OUT_ID_NAME(RI_Tell);
    OUT_ID_NAME(RI_ConstraintDef);
    OUT_ID_NAME(RI_ProgramDef);
  }

  outstream << " Node:" << string(iter->value.begin(), iter->value.end());

  size_t size = iter->children.size();
  if (size > 0) {
    outstream << " Child Size:" << size << endl;
  } else {
    outstream << endl;
  }

  for(size_t j=0; j<size; j++) {
    dump_ast(outstream, iter->children.begin()+j, nest+1);
  }

  return outstream;
}

boost::shared_ptr<parse_tree::Node>
HydLaParser::create_parse_tree(const tree_node_t &tree_node)
{
  const_tree_iter_t ch = tree_node.children.begin();

  long node_id = tree_node.value.id().to_long();
  switch(node_id) 
  {
    // プログラムの集合
    case RI_Statements:
    {
      node_sptr node_tree;
      const_tree_iter_t it  = tree_node.children.begin();
      const_tree_iter_t end = tree_node.children.end();
      while(it != end) {
        if(node_sptr new_tree = create_parse_tree(*it++)) {
          // 制約宣言が複数回出現した場合は「,」によって連結する
          if(node_tree) {
            shared_ptr<Parallel> rn(node_factory_->createParallel());
            rn->set_lhs(node_tree);
            rn->set_rhs(new_tree);
            node_tree = rn;
          } else {
            node_tree = new_tree;
          }
        }
      }
      parse_tree_.set_tree(node_tree);
      return node_sptr();
    }

    // 制約定義
    case RI_ConstraintDef:
    {
      const_tree_iter_t gch = ch->children.begin();
      std::string name(gch->value.begin(), gch->value.end());

      shared_ptr<ConstraintDefinition> 
        node(node_factory_->createConstraintDefinition());

      //定義名
      node->set_name(name);

      //仮引数
      if (ch->children.size()>1) {
        const_tree_iter_t args = (gch+1)->children.begin();
        size_t bound_variable_count = (gch+1)->children.size();
        for(size_t i=0; i<bound_variable_count; i++) {
          string v((args+i)->value.begin(), (args+i)->value.end());
          node->add_bound_variable(v);
        }
      }

      node->set_child(create_parse_tree(*(ch+1)));

      parse_tree_.addConstraintDefinition(node);

      return node_sptr();
    }

    // プログラム定義
    case RI_ProgramDef:
    {
      const_tree_iter_t gch = ch->children.begin();
      std::string name(gch->value.begin(), gch->value.end());

      shared_ptr<ProgramDefinition> 
        node(node_factory_->createProgramDefinition());

      //定義名
      node->set_name(name);

      //仮引数
      if (ch->children.size()>1) {
        const_tree_iter_t args = (gch+1)->children.begin();
        size_t bound_variable_count = (gch+1)->children.size();
        for(size_t i=0; i<bound_variable_count; i++) {
          string v((args+i)->value.begin(), (args+i)->value.end());
          node->add_bound_variable(v);
        }
      }

      node->set_child(create_parse_tree(*(ch+1)));

      parse_tree_.addProgramDefinition(node);

      return node_sptr();
    }

    // 制約呼び出し
    case RI_ConstraintCaller:
    {
      std::string name(ch->value.begin(), ch->value.end());

      shared_ptr<ConstraintCaller> 
        node(node_factory_->createConstraintCaller());

      //プログラム名
      node->set_name(name);

      //実引数
      if (tree_node.children.size()>1) {
        const_tree_iter_t args = (ch+1)->children.begin();
        size_t bound_variable_count = (ch+1)->children.size();
        for(size_t i=0; i<bound_variable_count; i++) {
          node->add_actual_arg(create_parse_tree(*(args+i)));
        }
      }
      return node;
    }

    // プログラム呼び出し
    case RI_ProgramCaller:
    {
      std::string name(ch->value.begin(), ch->value.end());

      shared_ptr<ProgramCaller> 
        node(node_factory_->createProgramCaller());

      //プログラム名
      node->set_name(name);

      //実引数
      if (tree_node.children.size()>1) {
        const_tree_iter_t args = (ch+1)->children.begin();
        size_t bound_variable_count = (ch+1)->children.size();
        for(size_t i=0; i<bound_variable_count; i++) {
          node->add_actual_arg(create_parse_tree(*(args+i)));
        }
      }
      return node;
    }

    // ask制約
    case RI_Implies:
    {
      shared_ptr<Ask> node(node_factory_->createAsk());

      // ガード条件
      node->set_guard(create_parse_tree(*ch));

      // 子制約
      node->set_child(create_parse_tree(*(ch+1)));

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
      shared_ptr<Variable> node(node_factory_->createVariable()); 
      string val(tree_node.value.begin(), tree_node.value.end());
      node->set_name(val);
      return node;
    }

    // 数字
    case RI_Number:
    {
      shared_ptr<Number> node(node_factory_->createNumber()); 
      string val(tree_node.value.begin(), tree_node.value.end());
      node->set_number(val);     
      return node;
    }

    default:
    {
      assert(0);
      return node_sptr();
    }  
  }
}

} //namespace parser
} //namespace hydla

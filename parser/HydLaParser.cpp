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

using namespace std;
using namespace boost;

using namespace hydla::parse_tree;
using namespace hydla::grammer_rule;

namespace hydla {
HydLaParser::HydLaParser(node_factory_sptr node_factory) :
  node_factory_(node_factory),
  debug_dump_(false)

{
}
  
HydLaParser::HydLaParser(node_factory_sptr node_factory, 
                         bool debug_dump) :
  node_factory_(node_factory),
  debug_dump_(debug_dump)
{
}
  

HydLaParser::~HydLaParser()
{
}

void HydLaParser::parse(std::istream& s) 
{
  clear_tree();
 
  if(debug_dump_ ) std::cout << "#*** create ast tree ***\n";
  create_ast(s);  
  if(debug_dump_ ) dump_ast(std::cout);

  if(debug_dump_ ) std::cout << "#*** create parse tree ***\n";
  create_parse_tree();
  if(debug_dump_ ) dump_parse_tree(std::cout);

  if(debug_dump_ ) std::cout << "#*** begin parse tree preprocessing ***\n";
  execute_preprocess();
  if(debug_dump_ ) dump_parse_tree(std::cout);

  if(debug_dump_ ) {
    std::cout << "#*** variables ***\n";
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

#define CREATE_BINARY_OP(X) \
  case RI_##X: \
    {  \
      shared_ptr<X> node(node_factory_->create##X()); \
      node->set_lhs(create_parse_tree(*ch)); \
      node->set_rhs(create_parse_tree(*(ch+1))); \
      return node; \
    }

#define CREATE_UNARY_OP(X) \
  case RI_##X: \
    {  \
      shared_ptr<X> node(node_factory_->create##X()); \
      node->set_child_node(create_parse_tree(*ch)); \
      return node; \
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
      parse_tree_.setTree(node_tree);
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

      node->set_child_node(create_parse_tree(*(ch+1)));

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

      node->set_child_node(create_parse_tree(*(ch+1)));

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
      node->set_guard_node(create_parse_tree(*ch));

      // 子制約
      node->set_child_node(create_parse_tree(*(ch+1)));

      return node;
    }

    // Tell制約
    CREATE_UNARY_OP(Tell)

    // 制約式
    CREATE_UNARY_OP(Constraint)

    // 比較演算子
    CREATE_BINARY_OP(Equal)
    CREATE_BINARY_OP(UnEqual)
    CREATE_BINARY_OP(Less)
    CREATE_BINARY_OP(LessEqual)
    CREATE_BINARY_OP(Greater)
    CREATE_BINARY_OP(GreaterEqual)

    // 論理演算子
    CREATE_BINARY_OP(LogicalAnd)
    CREATE_BINARY_OP(LogicalOr)

    // 算術二項演算子
    CREATE_BINARY_OP(Plus)
    CREATE_BINARY_OP(Subtract)
    CREATE_BINARY_OP(Times)
    CREATE_BINARY_OP(Divide)

    // 算術単項演算子
    CREATE_UNARY_OP(Negative)
    CREATE_UNARY_OP(Positive)

    // 制約階層定義演算子
    CREATE_BINARY_OP(Weaker)
    CREATE_BINARY_OP(Parallel)

    // 時相演算子
    CREATE_UNARY_OP(Always)

    // 微分
    CREATE_UNARY_OP(Differential)
    
    // 左極限
    CREATE_UNARY_OP(Previous)

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




std::string HydLaParser::create_interlanguage(std::string max_time)
{

/*
  std::string str;
  str += "HydLaMain[";
  str += create_interlanguage(ast_tree_.trees.begin());
  str += ", {";
  variable_map_t::iterator iter = variable_.begin();
  while(iter!=variable_.end()) {
    str += iter->first;
    if(++iter != variable_.end()) str += ", ";
  }
  str += "}," + max_time + "];";
  */


  return "";
}

#define TransExpArg2(X) case RI_##X:                \
  return #X "[" + create_interlanguage(ch) + ", " + \
    create_interlanguage(ch+1) + "]";


std::string HydLaParser::create_interlanguage(const_tree_iter_t &iter) 
{
  return "";
}

} //namespace hydla

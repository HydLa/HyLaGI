#include "PreprocessParseTree.h"

#include <assert.h>
#include <algorithm>
#include <boost/bind.hpp>

#include "ParseError.h"

using namespace hydla::parse_error;

namespace hydla { 
namespace parse_tree {

PreprocessParseTree::PreprocessParseTree()
{}

PreprocessParseTree::~PreprocessParseTree()
{}

// 制約定義
void PreprocessParseTree::visit(boost::shared_ptr<ConstraintDefinition> node)  
{
  assert(0);
}

// プログラム定義
void PreprocessParseTree::visit(boost::shared_ptr<ProgramDefinition> node)     
{
  assert(0);
}

// 制約呼び出し
void PreprocessParseTree::visit(boost::shared_ptr<ConstraintCaller> node)      
{
  /*
  State& state = state_stack_.top();

  difinition_type_t def_type(node->get_name(), 
                             node->actual_arg_size());

  // 制約定義から探す
  if(constraint_def_map_.find(def_type) == 
     constraint_def_map_.end()) {
    throw UndefinedReference(node->to_string());
  }

  //循環参照のチェック
  if (state.referenced_definition_list.find(def_type) != 
      state.referenced_definition_list.end()) {
    throw CircularReference(node->to_string());
  }


  // 実引数に対しpreprocess適用
  for_each(actual_arg_list_.begin(), actual_arg_list_.end(), 
           bind(&Node::preprocess, _1, _1, arg));

  dispatch<Ask, &Ask::get_guard, &Ask::set_guard>(node.get());


  (*it).second->preprocess(child_, arg, actual_arg_list_);
*/
}

// プログラム呼び出し
void PreprocessParseTree::visit(boost::shared_ptr<ProgramCaller> node)         
{
  assert(0);
}

// 制約式
void PreprocessParseTree::visit(boost::shared_ptr<Constraint> node)            
{
  assert(0);
}

// Ask制約
void PreprocessParseTree::visit(boost::shared_ptr<Ask> node)                   
{
  State& state = state_stack_.top();

  // ガードノードの探索
  state_stack_.push(state);
  state_stack_.top().in_guard = true;
  dispatch<Ask, &Ask::get_guard, &Ask::set_guard>(node.get());
  state_stack_.pop();

  // 子ノードの探索
  state_stack_.push(state);
  state_stack_.top().in_always = false;
  dispatch<Ask, &Ask::get_child, &Ask::set_child>(node.get());
  state_stack_.pop();
}

// Tell制約
void PreprocessParseTree::visit(boost::shared_ptr<Tell> node)                  
{
  dispatch_child(node);
}

// 算術単項演算子
void PreprocessParseTree::visit(boost::shared_ptr<Negative> node)
{
  dispatch_child(node);
}

void PreprocessParseTree::visit(boost::shared_ptr<Positive> node)
{
  dispatch_child(node);
}

void PreprocessParseTree::visit(boost::shared_ptr<Equal> node)                 
{
  dispatch_lhs(node);
  dispatch_rhs(node);
}

void PreprocessParseTree::visit(boost::shared_ptr<UnEqual> node)               
{
  dispatch_lhs(node);
  dispatch_rhs(node);
}

void PreprocessParseTree::visit(boost::shared_ptr<Less> node)
{
  dispatch_lhs(node);
  dispatch_rhs(node);
}

void PreprocessParseTree::visit(boost::shared_ptr<LessEqual> node)
{
  dispatch_lhs(node);
  dispatch_rhs(node);
}

void PreprocessParseTree::visit(boost::shared_ptr<Greater> node)
{
  dispatch_lhs(node);
  dispatch_rhs(node);
}

void PreprocessParseTree::visit(boost::shared_ptr<GreaterEqual> node)
{
  dispatch_lhs(node);
  dispatch_rhs(node);
}

void PreprocessParseTree::visit(boost::shared_ptr<Plus> node)
{
  dispatch_lhs(node);
  dispatch_rhs(node);
}

void PreprocessParseTree::visit(boost::shared_ptr<Subtract> node)
{
  dispatch_lhs(node);
  dispatch_rhs(node);
}

void PreprocessParseTree::visit(boost::shared_ptr<Times> node)
{
  dispatch_lhs(node);
  dispatch_rhs(node);
}

void PreprocessParseTree::visit(boost::shared_ptr<Divide> node)
{
  dispatch_lhs(node);
  dispatch_rhs(node);
}

void PreprocessParseTree::visit(boost::shared_ptr<LogicalAnd> node)
{
  if(!state_stack_.top().in_constraint) {
    throw InvalidConjunction(node->get_lhs()->to_string(), 
                             node->get_rhs()->to_string());
  }

  dispatch_lhs(node);
  dispatch_rhs(node);
}

void PreprocessParseTree::visit(boost::shared_ptr<LogicalOr> node)
{
  if(!state_stack_.top().in_guard) {
    throw InvalidDisjunction(node->get_lhs()->to_string(), 
                             node->get_rhs()->to_string());
  }

  dispatch_lhs(node);
  dispatch_rhs(node);
}  

void PreprocessParseTree::visit(boost::shared_ptr<Weaker> node)
{
  if(state_stack_.top().in_constraint) {
    throw InvalidWeakComposition(node->get_lhs()->to_string(), 
                                 node->get_rhs()->to_string());
  }

  dispatch_lhs(node);
  dispatch_rhs(node);
}

void PreprocessParseTree::visit(boost::shared_ptr<Parallel> node)
{
  if(state_stack_.top().in_constraint) {
    throw InvalidParallelComposition(node->get_lhs()->to_string(), 
                                     node->get_rhs()->to_string());
  }

  dispatch_lhs(node);
  dispatch_rhs(node);
}
  
// 時相演算子
void PreprocessParseTree::visit(boost::shared_ptr<Always> node)
{
  State& state = state_stack_.top();

  // ガードの中にはない
  if(state.in_guard) {
    throw InvalidAlways(node->get_child()->to_string());
  }

  // 子ノードの探索
  state_stack_.push(state);
  state_stack_.top().in_always = true;
  dispatch_child(node);
  state_stack_.pop();
    
  // すでにalways制約内であった場合このノードをはずす
  if(state.in_always) {
    new_child_ = node->get_child();
  }
}
  
// 微分
void PreprocessParseTree::visit(boost::shared_ptr<Differential> node)
{
  State& state = state_stack_.top();

  // 子ノードの探索
  state_stack_.push(state);
  state_stack_.top().differential_count++;
  dispatch_child(node);
  state_stack_.pop();
}

// 左極限
void PreprocessParseTree::visit(boost::shared_ptr<Previous> node)
{
  dispatch_child(node);
}
  
// 変数
void PreprocessParseTree::visit(boost::shared_ptr<Variable> node)
{
  State& state = state_stack_.top();

  formal_arg_map_t::iterator it = 
    state.formal_arg_map.find(node->get_name());
  if(it != state.formal_arg_map.end()) {
    // 自身が仮引数であった場合、書き換える
    new_child_ = (*it).second;
  } 
  else {
    // 自身が実引数であった場合、
    // 自身のすでに登録済みの微分回数よりも
    // 大きかったら変数のリストに登録する
    variable_map_t::iterator it = variable_map_.find(node->get_name());
    if(it == variable_map_.end() ||
       it->second < state.differential_count) 
    {
      variable_map_.insert(make_pair(node->get_name(), 
                                     state.differential_count));
    }
  }
}

// 数字
void PreprocessParseTree::visit(boost::shared_ptr<Number> node)
{
  //do nothing
}


} //namespace parse_tree
} //namespace hydla

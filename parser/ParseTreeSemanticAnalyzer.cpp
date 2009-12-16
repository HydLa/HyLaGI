#include "ParseTreeSemanticAnalyzer.h"

#include <assert.h>
#include <algorithm>
#include <boost/bind.hpp>

#include "ParseError.h"

using namespace hydla::parse_tree;
using namespace hydla::parse_error;

namespace hydla { 
namespace parser {

ParseTreeSemanticAnalyzer::ParseTreeSemanticAnalyzer()
{}

ParseTreeSemanticAnalyzer::~ParseTreeSemanticAnalyzer()
{}

void ParseTreeSemanticAnalyzer::analyze(hydla::parse_tree::ParseTree* pt)
{
  parse_tree_         = pt;

  State state;
  state.in_guard           = false;
  state.in_always          = false;
  state.in_constraint      = false;
  state.differential_count = 0;
  state_stack_.push(state);

  pt->dispatch(this);
  assert(state_stack_.size() == 1);
}

// 制約定義
void ParseTreeSemanticAnalyzer::visit(boost::shared_ptr<ConstraintDefinition> node)  
{
  assert(0);
}

// プログラム定義
void ParseTreeSemanticAnalyzer::visit(boost::shared_ptr<ProgramDefinition> node)     
{
  assert(0);
}

node_sptr ParseTreeSemanticAnalyzer::apply_definition(
  difinition_type_t* def_type,
  const boost::shared_ptr<hydla::parse_tree::Caller>& caller, 
  Definition* definition)
{
  State& state = state_stack_.top();
  
  //循環参照のチェック
  if (state.referenced_definition_list.find(*def_type) != 
      state.referenced_definition_list.end()) {
    throw CircularReference(caller);
  }

  State new_state(state);
  new_state.formal_arg_map.clear();

  assert(definition->bound_variable_size() == caller->actual_arg_size());
  
  Definition::bound_variables_iterator 
    bv_it = definition->bound_variable_begin();
  Definition::bound_variables_iterator 
    bv_end = definition->bound_variable_end();
  Caller::actual_args_iterator         
    aa_it = caller->actual_arg_begin();
  for(; bv_it!=bv_end; ++bv_it, ++aa_it) {
    // 実引数に対しpreprocess適用
    accept(*aa_it);
    if(new_child_) {
      *aa_it = new_child_;
      new_child_.reset();
    }

    // 仮引数と実引数の対応付け
    new_state.formal_arg_map.insert(make_pair(*bv_it, *aa_it));
  }

  // 循環参照検出用リストに登録
  new_state.referenced_definition_list.insert(*def_type);

  // 定義の子ノードに対しpreprocess適用
  boost::shared_ptr<Definition> defnode(
    boost::shared_static_cast<Definition>(definition->clone()));
  state_stack_.push(new_state);
  dispatch_child(defnode);
  state_stack_.pop();

  return defnode->get_child();
}

// 制約呼び出し
void ParseTreeSemanticAnalyzer::visit(boost::shared_ptr<ConstraintCaller> node)      
{
  difinition_type_t def_type(node->get_name(), node->actual_arg_size());

  // 制約定義から探す
  boost::shared_ptr<ConstraintDefinition> cons_def(
    parse_tree_->get_constraint_difinition(def_type));
  if(!cons_def) {
    throw UndefinedReference(node);
  }

  // 定義の展開
  node->set_child(
    apply_definition(&def_type, node, cons_def.get()));

  update_node_id(node);
}

// プログラム呼び出し
void ParseTreeSemanticAnalyzer::visit(boost::shared_ptr<ProgramCaller> node)         
{
  difinition_type_t def_type(node->get_name(), node->actual_arg_size());
  Definition* defnode;

  // 制約定義から探す
  boost::shared_ptr<ConstraintDefinition> cons_def(
    parse_tree_->get_constraint_difinition(def_type));
  if(cons_def) {
    defnode = cons_def.get();
  } else {
    // プログラム定義から探す
    boost::shared_ptr<ProgramDefinition> prog_def(
      parse_tree_->get_program_difinition(def_type));
    if(prog_def) {
      defnode = prog_def.get();
    } else {
      throw UndefinedReference(node);
    }
  }

  // 定義の展開
  node->set_child(
    apply_definition(&def_type, node, defnode));

  update_node_id(node);
}

// 制約式
void ParseTreeSemanticAnalyzer::visit(boost::shared_ptr<Constraint> node)            
{
  State& state = state_stack_.top();

  // すでに制約式の中であった場合は自分自身を取り除く
  if(state.in_constraint) {
    dispatch_child(node);
    new_child_ = node->get_child();
  } 
  else {
    state_stack_.push(state);
    state_stack_.top().in_constraint = true;
    dispatch_child(node);
    state_stack_.pop();
  }

  update_node_id(node);
}

// Ask制約
void ParseTreeSemanticAnalyzer::visit(boost::shared_ptr<Ask> node)                   
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


  update_node_id(node);
}

// Tell制約
void ParseTreeSemanticAnalyzer::visit(boost::shared_ptr<Tell> node)                  
{
  dispatch_child(node);

  update_node_id(node);
}

// 算術単項演算子
void ParseTreeSemanticAnalyzer::visit(boost::shared_ptr<Negative> node)
{
  dispatch_child(node);
  update_node_id(node);
}

void ParseTreeSemanticAnalyzer::visit(boost::shared_ptr<Positive> node)
{
  dispatch_child(node);
  update_node_id(node);
}

void ParseTreeSemanticAnalyzer::visit(boost::shared_ptr<Equal> node)                 
{
  dispatch_lhs(node);
  dispatch_rhs(node); 
  update_node_id(node);
}

void ParseTreeSemanticAnalyzer::visit(boost::shared_ptr<UnEqual> node)               
{
  dispatch_lhs(node);
  dispatch_rhs(node); 
  update_node_id(node);
}

void ParseTreeSemanticAnalyzer::visit(boost::shared_ptr<Less> node)
{
  dispatch_lhs(node);
  dispatch_rhs(node); 
  update_node_id(node);
}

void ParseTreeSemanticAnalyzer::visit(boost::shared_ptr<LessEqual> node)
{
  dispatch_lhs(node);
  dispatch_rhs(node); 
  update_node_id(node);
}

void ParseTreeSemanticAnalyzer::visit(boost::shared_ptr<Greater> node)
{
  dispatch_lhs(node);
  dispatch_rhs(node); 
  update_node_id(node);
}

void ParseTreeSemanticAnalyzer::visit(boost::shared_ptr<GreaterEqual> node)
{
  dispatch_lhs(node);
  dispatch_rhs(node); 
  update_node_id(node);
}

void ParseTreeSemanticAnalyzer::visit(boost::shared_ptr<Plus> node)
{
  dispatch_lhs(node);
  dispatch_rhs(node); 
  update_node_id(node);
}

void ParseTreeSemanticAnalyzer::visit(boost::shared_ptr<Subtract> node)
{
  dispatch_lhs(node);
  dispatch_rhs(node); 
  update_node_id(node);
}

void ParseTreeSemanticAnalyzer::visit(boost::shared_ptr<Times> node)
{
  dispatch_lhs(node);
  dispatch_rhs(node); 
  update_node_id(node);
}

void ParseTreeSemanticAnalyzer::visit(boost::shared_ptr<Divide> node)
{
  dispatch_lhs(node);
  dispatch_rhs(node); 
  update_node_id(node);
}

void ParseTreeSemanticAnalyzer::visit(boost::shared_ptr<LogicalAnd> node)
{
  if(!state_stack_.top().in_constraint) {
    throw InvalidConjunction(node->get_lhs(), 
                             node->get_rhs());
  }

  dispatch_lhs(node);
  dispatch_rhs(node); 
  update_node_id(node);
}

void ParseTreeSemanticAnalyzer::visit(boost::shared_ptr<LogicalOr> node)
{
  if(!state_stack_.top().in_guard) {
    throw InvalidDisjunction(node->get_lhs(), 
                             node->get_rhs());
  }

  dispatch_lhs(node);
  dispatch_rhs(node); 
  update_node_id(node);
}  

void ParseTreeSemanticAnalyzer::visit(boost::shared_ptr<Weaker> node)
{
  if(state_stack_.top().in_constraint) {
    throw InvalidWeakComposition(node->get_lhs(), 
                                 node->get_rhs());
  }

  dispatch_lhs(node);
  dispatch_rhs(node); 
  update_node_id(node);
}

void ParseTreeSemanticAnalyzer::visit(boost::shared_ptr<Parallel> node)
{
  if(state_stack_.top().in_constraint) {
    throw InvalidParallelComposition(node->get_lhs(), 
                                     node->get_rhs());
  }

  dispatch_lhs(node);
  dispatch_rhs(node); 

  update_node_id(node);
}
  
// 時相演算子
void ParseTreeSemanticAnalyzer::visit(boost::shared_ptr<Always> node)
{
  State& state = state_stack_.top();

  // ガードの中にはない
  if(state.in_guard) {
    throw InvalidAlways(node->get_child());
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

  update_node_id(node);
}
  
// 微分
void ParseTreeSemanticAnalyzer::visit(boost::shared_ptr<Differential> node)
{
  State& state = state_stack_.top();

  // 子ノードの探索
  state_stack_.push(state);
  state_stack_.top().differential_count++;
  dispatch_child(node);
  state_stack_.pop(); 

  update_node_id(node);
}

// 左極限
void ParseTreeSemanticAnalyzer::visit(boost::shared_ptr<Previous> node)
{
  dispatch_child(node); 

  update_node_id(node);
}
  
// 変数
void ParseTreeSemanticAnalyzer::visit(boost::shared_ptr<Variable> node)
{
  State& state = state_stack_.top();

  formal_arg_map_t::iterator it = 
    state.formal_arg_map.find(node->get_name());
  if(it != state.formal_arg_map.end()) {
    // 自身が仮引数であった場合、書き換える
    new_child_ = (*it).second;
  } 
  else {
    // 自身が実引数であった場合
    parse_tree_->register_variable(node->get_name(), 
                                   state.differential_count);
  } 

  update_node_id(node);
}

// 数字
void ParseTreeSemanticAnalyzer::visit(boost::shared_ptr<Number> node)
{
  update_node_id(node);
}


} //namespace parser
} //namespace hydla

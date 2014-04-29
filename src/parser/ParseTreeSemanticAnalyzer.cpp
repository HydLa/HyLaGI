#include "ParseTreeSemanticAnalyzer.h"

#include <assert.h>
#include <algorithm>
#include <boost/bind.hpp>

#include "ParseError.h"

using namespace hydla::symbolic_expression;
using namespace hydla::parser::error;

namespace hydla { 
namespace parser {


#define DEFINE_DEFAULT_VISIT_ARBITRARY(NODE_NAME)        \
void ParseTreeSemanticAnalyzer::visit(boost::shared_ptr<NODE_NAME> node) \
{                                                     \
  for(int i=0;i<node->get_arguments_size();i++){      \
    accept(node->get_argument(i));                    \
    if(new_child_) {                                  \
      node->set_argument((new_child_), i);            \
      new_child_.reset();                             \
    }                                                 \
  }                                                   \
}

#define DEFINE_DEFAULT_VISIT_BINARY(NODE_NAME)        \
void ParseTreeSemanticAnalyzer::visit(boost::shared_ptr<NODE_NAME> node) \
{                                                     \
  dispatch_lhs(node);                                 \
  dispatch_rhs(node);                                 \
}

#define DEFINE_DEFAULT_VISIT_UNARY(NODE_NAME)        \
void ParseTreeSemanticAnalyzer::visit(boost::shared_ptr<NODE_NAME> node) \
{ dispatch_child(node);}

#define DEFINE_DEFAULT_VISIT_FACTOR(NODE_NAME)        \
void ParseTreeSemanticAnalyzer::visit(boost::shared_ptr<NODE_NAME> node){}


ParseTreeSemanticAnalyzer::ParseTreeSemanticAnalyzer(
  DefinitionContainer<hydla::symbolic_expression::ConstraintDefinition>& constraint_definition,
  DefinitionContainer<hydla::symbolic_expression::ProgramDefinition>&    program_definition,
  hydla::parse_tree::ParseTree* parse_tree) :
    constraint_definition_(constraint_definition),
    program_definition_(program_definition),
    parse_tree_(parse_tree)
{}

ParseTreeSemanticAnalyzer::~ParseTreeSemanticAnalyzer()
{}

void ParseTreeSemanticAnalyzer::analyze(symbolic_expression::node_sptr& n/*, variable_map_t& variable_map*/)
{
  if(n) {
    State state;
    state.in_guard           = false;
    state.in_always          = false;
    state.in_constraint      = false;
    state.differential_count = 0;
    todo_stack_.push(state);
  //  variable_map_ = &variable_map;

    accept(n);
    if(new_child_) n = new_child_;

    assert(todo_stack_.size() == 1);
  }
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

symbolic_expression::node_sptr ParseTreeSemanticAnalyzer::apply_definition(
  const referenced_definition_t& def_type,
  boost::shared_ptr<hydla::symbolic_expression::Caller> caller, 
  boost::shared_ptr<Definition> definition)
{
  State& state = todo_stack_.top();

  definition = boost::dynamic_pointer_cast<Definition>(definition->clone());
  
  //循環参照のチェック
  if (state.referenced_definition_list.find(def_type) != 
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
  new_state.referenced_definition_list.insert(def_type);

  // 定義の子ノードに対しpreprocess適用
  todo_stack_.push(new_state);
  dispatch_child(definition);
  todo_stack_.pop();

  return definition->get_child();
}

// 制約呼び出し
void ParseTreeSemanticAnalyzer::visit(boost::shared_ptr<ConstraintCaller> node)      
{
  referenced_definition_t deftype(
    std::make_pair(node->get_name(), 
                   node->actual_arg_size()));

  if(!node->get_child()) {
    // 制約定義から探す
    boost::shared_ptr<ConstraintDefinition> cons_def(
      constraint_definition_.get_definition(deftype));
    if(!cons_def) {
      throw UndefinedReference(node);
    }

    // 定義の展開
    node->set_child( 
      apply_definition(deftype, node, cons_def));
  }
}

// プログラム呼び出し
void ParseTreeSemanticAnalyzer::visit(boost::shared_ptr<ProgramCaller> node)         
{
  referenced_definition_t deftype(
    std::make_pair(node->get_name(), 
                   node->actual_arg_size()));

  if(!node->get_child()) {
    boost::shared_ptr<Definition> defnode;

    // 制約定義から探す
    boost::shared_ptr<ConstraintDefinition> cons_def(
      constraint_definition_.get_definition(deftype));
    if(cons_def) {
      defnode = cons_def;
    } else {
      // プログラム定義から探す
      boost::shared_ptr<ProgramDefinition> prog_def(
        program_definition_.get_definition(deftype));
      if(prog_def) {
        defnode = prog_def;
      } else {
        throw UndefinedReference(node);
      }
    }

    // 定義の展開
    node->set_child(
      apply_definition(deftype, node, defnode));
  }
}

// 制約式
void ParseTreeSemanticAnalyzer::visit(boost::shared_ptr<Constraint> node)            
{
  State& state = todo_stack_.top();

  // すでに制約式の中であった場合は自分自身を取り除く
  if(state.in_constraint) {
    dispatch_child(node);
    new_child_ = node->get_child();
  } 
  else {
    todo_stack_.push(state);
    todo_stack_.top().in_constraint = true;
    dispatch_child(node);
    todo_stack_.pop();
  }


}

// Ask制約
void ParseTreeSemanticAnalyzer::visit(boost::shared_ptr<Ask> node)                   
{
  State& state = todo_stack_.top();

  // ガードノードの探索
  todo_stack_.push(state);
  todo_stack_.top().in_guard = true;
  dispatch<Ask, &Ask::get_guard, &Ask::set_guard>(node.get());
  todo_stack_.pop();

  // 子ノードの探索
  todo_stack_.push(state);
  todo_stack_.top().in_always = false;
  dispatch<Ask, &Ask::get_child, &Ask::set_child>(node.get());
  todo_stack_.pop();
}


// Tell制約
DEFINE_DEFAULT_VISIT_UNARY(Tell)

// 算術単項演算子
DEFINE_DEFAULT_VISIT_UNARY(Negative)
DEFINE_DEFAULT_VISIT_UNARY(Positive)


// 比較演算子
DEFINE_DEFAULT_VISIT_BINARY(Equal)
DEFINE_DEFAULT_VISIT_BINARY(UnEqual)
DEFINE_DEFAULT_VISIT_BINARY(Less)
DEFINE_DEFAULT_VISIT_BINARY(LessEqual)
DEFINE_DEFAULT_VISIT_BINARY(Greater)
DEFINE_DEFAULT_VISIT_BINARY(GreaterEqual)

// 算術二項演算子
DEFINE_DEFAULT_VISIT_BINARY(Plus)
DEFINE_DEFAULT_VISIT_BINARY(Subtract)
DEFINE_DEFAULT_VISIT_BINARY(Times)
DEFINE_DEFAULT_VISIT_BINARY(Divide)
DEFINE_DEFAULT_VISIT_BINARY(Power)

// 論理演算子
void ParseTreeSemanticAnalyzer::visit(boost::shared_ptr<LogicalAnd> node)
{
  if(!todo_stack_.top().in_constraint) {
    throw InvalidConjunction(node->get_lhs(), 
                             node->get_rhs());
  }

  dispatch_lhs(node);
  dispatch_rhs(node); 
}

void ParseTreeSemanticAnalyzer::visit(boost::shared_ptr<LogicalOr> node)
{
  if(!todo_stack_.top().in_guard) {
    throw InvalidDisjunction(node->get_lhs(), 
                             node->get_rhs());
  }

  dispatch_lhs(node);
  dispatch_rhs(node); 
}

DEFINE_DEFAULT_VISIT_UNARY(Not)

void ParseTreeSemanticAnalyzer::visit(boost::shared_ptr<Weaker> node)
{
  if(todo_stack_.top().in_constraint) {
    throw InvalidWeakComposition(node->get_lhs(), 
                                 node->get_rhs());
  }

  dispatch_lhs(node);
  dispatch_rhs(node); 

}

void ParseTreeSemanticAnalyzer::visit(boost::shared_ptr<Parallel> node)
{
  if(todo_stack_.top().in_constraint) {
    throw InvalidParallelComposition(node->get_lhs(), 
                                     node->get_rhs());
  }

  dispatch_lhs(node);
  dispatch_rhs(node); 


}

// 時相演算子
void ParseTreeSemanticAnalyzer::visit(boost::shared_ptr<Always> node)
{
  State& state = todo_stack_.top();

  // ガードの中にはない
  if(state.in_guard) {
    throw InvalidAlways(node->get_child());
  }

  // 子ノードの探索
  todo_stack_.push(state);
  todo_stack_.top().in_always = true;
  dispatch_child(node);
  todo_stack_.pop();
    
  // すでにalways制約内であった場合このノードをはずす
  if(state.in_always) {
    new_child_ = node->get_child();
  } 


}
  
// 微分
void ParseTreeSemanticAnalyzer::visit(boost::shared_ptr<Differential> node)
{
  // 子ノードが微分か変数でなかったらエラー
  if(!boost::dynamic_pointer_cast<Differential>(node->get_child()) &&
     !boost::dynamic_pointer_cast<Variable>(node->get_child())) {
       throw InvalidDifferential(node);
  }

  // 子ノードの探索
  todo_stack_.top().differential_count++;
  dispatch_child(node);
  todo_stack_.top().differential_count--;
}

// 左極限
void ParseTreeSemanticAnalyzer::visit(boost::shared_ptr<Previous> node)
{  
  // 子ノードが微分か変数でなかったらエラー
  if(!boost::dynamic_pointer_cast<Differential>(node->get_child()) &&
     !boost::dynamic_pointer_cast<Variable>(node->get_child())) {
       throw InvalidPrevious(node);
  }

  dispatch_child(node); 
}



// 関数
DEFINE_DEFAULT_VISIT_ARBITRARY(Function)
DEFINE_DEFAULT_VISIT_ARBITRARY(UnsupportedFunction)

// 円周率
DEFINE_DEFAULT_VISIT_FACTOR(Pi)

// 自然対数の底
DEFINE_DEFAULT_VISIT_FACTOR(E)

// True
DEFINE_DEFAULT_VISIT_FACTOR(True)

// False
DEFINE_DEFAULT_VISIT_FACTOR(False)

// 変数
void ParseTreeSemanticAnalyzer::visit(boost::shared_ptr<Variable> node)
{
  State& state = todo_stack_.top();

  formal_arg_map_t::iterator it = 
    state.formal_arg_map.find(node->get_name());
  if(it != state.formal_arg_map.end()) {
    // 自身が仮引数であった場合、書き換える
    new_child_ = (*it).second->clone();
    boost::shared_ptr<hydla::symbolic_expression::Variable> v_ptr = boost::dynamic_pointer_cast<Variable>(new_child_);
    if(v_ptr){
      //変数だった場合、登録する
      parse_tree_->register_variable(v_ptr->get_name(), state.differential_count);
    }
  } 
  else {
    // 自身が実引数であった場合
    parse_tree_->register_variable(node->get_name(), 
                                   state.differential_count);
  } 
}

// 数字
DEFINE_DEFAULT_VISIT_FACTOR(Number)
DEFINE_DEFAULT_VISIT_FACTOR(Float)

// Print
DEFINE_DEFAULT_VISIT_FACTOR(Print)
DEFINE_DEFAULT_VISIT_FACTOR(PrintPP)
DEFINE_DEFAULT_VISIT_FACTOR(PrintIP)
DEFINE_DEFAULT_VISIT_FACTOR(Scan)
DEFINE_DEFAULT_VISIT_FACTOR(Exit)
DEFINE_DEFAULT_VISIT_FACTOR(Abort)

// SystemVariable
DEFINE_DEFAULT_VISIT_FACTOR(SVtimer)

DEFINE_DEFAULT_VISIT_FACTOR(SymbolicT)


} //namespace parser
} //namespace hydla

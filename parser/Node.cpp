#include "Node.h"

#include <assert.h>
#include <algorithm>
#include <boost/bind.hpp>

#include "ParseError.h"
#include "TreeVisitor.h"

using namespace std;
using namespace boost;
using namespace hydla::parse_error;

namespace hydla { 
namespace parse_tree {
  
std::ostream& operator<< (std::ostream& s, Node& node)
{
  s << node.to_string();
  return s;
}

void ProgramCaller::preprocess(node_sptr& own, preprocess_arg_t& arg)
{
  shared_ptr<Definition> defnode;

  difinition_type_t def_type(name_, actual_arg_list_.size());

  // 制約定義から探す
  constraint_def_map_t::iterator cons_it = arg.cons_def_map_.find(def_type);
  if(cons_it!=arg.cons_def_map_.end()) {
    defnode = (*cons_it).second;
  } else {
    // プログラム定義から探す
    program_def_map_t::iterator prog_it = arg.prog_def_map_.find(def_type);
    if(prog_it!=arg.prog_def_map_.end()) {
      defnode = (*prog_it).second;
    } else {
      throw UndefinedReference(to_string());
    }
  }

  //循環参照のチェック
  if (arg.refered_def_.find(def_type) != arg.refered_def_.end()) {
    throw CircularReference(to_string());
  }

  // 実引数に対しpreprocess適用
  for_each(actual_arg_list_.begin(), actual_arg_list_.end(), 
           bind(&Node::preprocess, _1, _1, arg));

  defnode->preprocess(child_, arg, actual_arg_list_);
}

void ConstraintCaller::preprocess(node_sptr& own, preprocess_arg_t& arg)
{
  difinition_type_t def_type(name_, actual_arg_list_.size());

  // 制約定義から探す
  constraint_def_map_t::iterator it = arg.cons_def_map_.find(def_type);
  if(it==arg.cons_def_map_.end()) {
    throw UndefinedReference(to_string());
  }

  //循環参照のチェック
  if (arg.refered_def_.find(def_type) != arg.refered_def_.end()) {
    throw CircularReference(to_string());
  }


  // 実引数に対しpreprocess適用
  for_each(actual_arg_list_.begin(), actual_arg_list_.end(), 
           bind(&Node::preprocess, _1, _1, arg));

  (*it).second->preprocess(child_, arg, actual_arg_list_);
}

std::string Caller::to_string() const
{
  actual_arg_list_t::const_iterator it  = actual_arg_list_.begin();
  actual_arg_list_t::const_iterator end = actual_arg_list_.end();

  std::string s;
  s += "call<";
  s += name_;
  s += "(";

  if(it!=end) s += (*it++)->to_string();
  while(it!=end) {
    s += ",";
    s += (*it++)->to_string();
  }

  s += ")>";
  if(child_) {
    s +=  "[";
    s += child_->to_string();
    s += "]";
  }
  return s;
}

node_sptr Caller::clone()
{
  boost::shared_ptr<ProgramCaller> n(new ProgramCaller());
  n->name_ = name_;
  n->actual_arg_list_.resize(actual_arg_list_.size());
  copy(actual_arg_list_.begin(), actual_arg_list_.end(),  n->actual_arg_list_.begin());
  if(child_) n->child_ = child_->clone();
  
  return n;
}

void Definition::preprocess(node_sptr& own, 
                            preprocess_arg_t& arg,
                            actual_arg_list_t& actual_arg_list) 
{
  formal_arg_map_t fam;

  // 仮引数と実引数の対応付け
  bound_variables_t::iterator bv_it = bound_variables_.begin();
  actual_arg_list_t::iterator aa_it = actual_arg_list.begin();
  for(; aa_it!=actual_arg_list.end(); ++bv_it, ++aa_it) {
    (*aa_it)->preprocess(*aa_it, arg);
    fam.insert(make_pair(*bv_it, *aa_it));
  }

  // 循環参照検出用リストに登録
  preprocess_arg_t narg(arg, fam);
  narg.refered_def_.insert(make_pair(name_, bound_variables_.size()));

  own = child_->clone(); 
  own->preprocess(own, narg);
}

std::string Definition::to_string() const
{
  bound_variables_t::const_iterator it  = bound_variables_.begin();
  bound_variables_t::const_iterator end = bound_variables_.end();

  std::string s;
  s += name_;
  s += "(";

  if(it!=end) s += *(it++);
  while(it!=end) {
    s += ",";
    s += *(it++);
  }

  s += "):=";
  s += child_->to_string();
  return s;
}

node_sptr Definition::clone()
{
  boost::shared_ptr<ConstraintDefinition> n(new ConstraintDefinition());
  n->name_ = name_;
  n->bound_variables_.resize(bound_variables_.size());
  copy(bound_variables_.begin(), bound_variables_.end(),  n->bound_variables_.begin());
  n->child_ = child_->clone();

  return n;
}

void LogicalAnd::preprocess(node_sptr& own, preprocess_arg_t& arg)
{
  if(!arg.in_constraint_) {
    throw InvalidConjunction(lhs_->to_string(), rhs_->to_string());
  }

  lhs_->preprocess(lhs_, arg);
  rhs_->preprocess(rhs_, arg);
}

void LogicalOr::preprocess(node_sptr& own, preprocess_arg_t& arg)
{
  if(!arg.in_guard_) {
    throw InvalidDisjunction(lhs_->to_string(), rhs_->to_string());
  }

  lhs_->preprocess(lhs_, arg);
  rhs_->preprocess(rhs_, arg);
}

void Parallel::preprocess(node_sptr& own, preprocess_arg_t& arg)
{
  if(arg.in_constraint_) {
    throw InvalidParallelComposition(lhs_->to_string(), rhs_->to_string());
  }
    
  lhs_->preprocess(lhs_, arg);
  rhs_->preprocess(rhs_, arg);
}

void Weaker::preprocess(node_sptr& own, preprocess_arg_t& arg)
{
  if(arg.in_constraint_) {
    throw InvalidWeakComposition(lhs_->to_string(), rhs_->to_string());
  }
    
  lhs_->preprocess(lhs_, arg);
  rhs_->preprocess(rhs_, arg);
}

void Always::preprocess(node_sptr& own, preprocess_arg_t& arg)
{
  if(arg.in_guard_) {
    throw InvalidAlways(child_->to_string());
  }

  preprocess_arg_t narg(arg);
  narg.in_always_ = true;    
  child_->preprocess(child_, narg);

    
  // すでにalways制約内であった場合このノードははずす
  if(arg.in_always_) {
    own = child_;
  }
}

void Variable::preprocess(node_sptr& own, preprocess_arg_t& arg)
{
  formal_arg_map_t::iterator it = arg.formal_arg_map_.find(name_);
  if(it != arg.formal_arg_map_.end()) {
    // 自身が仮引数であった場合、書き換える
    own = (*it).second;
  } else {
    // 実引数であった場合、自身のすでに登録済みの微分回数よりも
    // 大きかったら変数のリストに登録する
    variable_map_t::iterator it = arg.variable_map_.find(name_);
    if(it == arg.variable_map_.end() ||
      it->second < arg.differential_count_) {

        arg.variable_map_.insert(
          make_pair(name_, arg.differential_count_));
    }
  }
}

/**
 * 各ノードのaccept関数定義
 */

#define DEFINE_ACCEPT_FUNC(CLASS) \
  void CLASS::accept(node_sptr own, \
                     TreeVisitor* visitor) \
  { \
    assert(this == own.get()); \
    visitor->visit(boost::shared_static_cast<CLASS>(own)); \
  }

//定義
DEFINE_ACCEPT_FUNC(ProgramDefinition)
DEFINE_ACCEPT_FUNC(ConstraintDefinition)

//呼び出し
DEFINE_ACCEPT_FUNC(ProgramCaller)
DEFINE_ACCEPT_FUNC(ConstraintCaller)

 //制約式
DEFINE_ACCEPT_FUNC(Constraint);

//Tell制約
DEFINE_ACCEPT_FUNC(Tell)

//Ask制約
DEFINE_ACCEPT_FUNC(Ask)

//比較演算子
DEFINE_ACCEPT_FUNC(Equal)
DEFINE_ACCEPT_FUNC(UnEqual)
DEFINE_ACCEPT_FUNC(Less)
DEFINE_ACCEPT_FUNC(LessEqual)
DEFINE_ACCEPT_FUNC(Greater)
DEFINE_ACCEPT_FUNC(GreaterEqual)

//論理演算子
DEFINE_ACCEPT_FUNC(LogicalAnd)
DEFINE_ACCEPT_FUNC(LogicalOr)

//算術二項演算子
DEFINE_ACCEPT_FUNC(Plus)
DEFINE_ACCEPT_FUNC(Subtract)
DEFINE_ACCEPT_FUNC(Times)
DEFINE_ACCEPT_FUNC(Divide)

//算術単項演算子
DEFINE_ACCEPT_FUNC(Negative)
DEFINE_ACCEPT_FUNC(Positive)

//制約階層定義演算子
DEFINE_ACCEPT_FUNC(Weaker)
DEFINE_ACCEPT_FUNC(Parallel)

// 時相演算子
DEFINE_ACCEPT_FUNC(Always)

//微分
DEFINE_ACCEPT_FUNC(Differential)

//左極限
DEFINE_ACCEPT_FUNC(Previous)

//変数・束縛変数
DEFINE_ACCEPT_FUNC(Variable)

//数字
DEFINE_ACCEPT_FUNC(Number)

} //namespace parse_tree
} //namespace hydla

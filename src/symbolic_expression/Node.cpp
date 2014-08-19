#include "Node.h"

#include <assert.h>

#include "ParseError.h"
#include "BaseNodeVisitor.h"
#include "TreeInfixPrinter.h"
#include "Logger.h"

using namespace std;
using namespace boost;
using namespace hydla::parser::error;
using namespace hydla::logger;

namespace hydla { 
namespace symbolic_expression {

std::ostream& operator<<(std::ostream& s, const Node& node)
{
  return node.dump(s);
}

std::string get_infix_string(const node_sptr& node)
{
  return TreeInfixPrinter().get_infix_string(node);
}

bool Node::is_same_struct(const Node& n, bool exactly_same) const
{
  return typeid(*this) == typeid(n);
}

bool UnaryNode::is_same_struct(const Node& n, bool exactly_same) const
{
  return typeid(*this) == typeid(n) &&
          child_->is_same_struct(*static_cast<const UnaryNode*>(&n)->child_.get(), 
                                 exactly_same);
}

bool BinaryNode::is_exactly_same(const Node& n, bool exactly_same) const
{
  return typeid(*this) == typeid(n) &&
    lhs_->is_same_struct(*static_cast<const BinaryNode*>(&n)->lhs_.get(), exactly_same) &&
    rhs_->is_same_struct(*static_cast<const BinaryNode*>(&n)->rhs_.get(), exactly_same);
}

void BinaryNode::create_child_node_list(child_node_list_t& cnl, 
                                        const Node* n) const
{
  const BinaryNode* binnode = dynamic_cast<const BinaryNode*>(n);

  if(binnode != NULL) {
    const Node* lhs = binnode->lhs_.get();
    const Node* rhs = binnode->rhs_.get();


    // 左辺ノード
    if(typeid(*n) == typeid(*lhs)) {
      create_child_node_list(cnl, lhs);
    }
    else {
      cnl.push_back(make_pair(lhs, false));
    }

    // 右辺ノード
    if(typeid(*n) == typeid(*rhs)) {
      create_child_node_list(cnl, rhs);
    }
    else {
      cnl.push_back(make_pair(rhs, false));
    }
  }
}

struct BinaryNode::CheckInclude
{
  CheckInclude(const Node* n) :
    node(n)
  {}

  template<typename T>
  bool operator()(T& it)
  {
    if(!it.second && node->is_same_struct(*it.first, false)) {
      it.second = true;
      return true;
    }
    return false;
  }
  
  const Node* node;
};

bool BinaryNode::is_same_struct(const Node& n, bool exactly_same) const
{
  if(exactly_same) {
    return is_exactly_same(n, exactly_same);
  }

  if(typeid(*this) == typeid(n)) {
    // 双方の子ノードの集合が同一かどうか調べる

    child_node_list_t this_node;
    child_node_list_t target_node;
    create_child_node_list(this_node,   this);
    create_child_node_list(target_node, static_cast<const BinaryNode*>(&n));
    if(this_node.size() != target_node.size()) return false;

    child_node_list_t::const_iterator it  = this_node.begin();
    child_node_list_t::const_iterator end = this_node.end();
    for(; it!=end; ++it) {
      if(std::find_if(target_node.begin(), 
                      target_node.end(), 
                      CheckInclude(it->first)) == target_node.end()) {
        return false;
      }
    }
    return true;
  }

  return false;
}





    /*
    const Node& n_lhs = *(static_cast<const BinaryNode*>(&n))->lhs_.get();
    const Node& n_rhs = *(static_cast<const BinaryNode*>(&n))->rhs_.get();

    // 対象性
    ret = 
      ((lhs_->is_same_struct(n_lhs, false) &&
      rhs_->is_same_struct(n_rhs, false)) ||
      (lhs_->is_same_struct(n_rhs, false) &&
      rhs_->is_same_struct(n_lhs, false)));


    // node1
    //      &
    //    /   \  
    //   &     c
    //  /  \  
    // a    b
    //
    // と
    //
    // node2
    //   &
    //  / \  
    // a   &
    //    / \  
    //   b   c
    //
    // の同一視

    // このノードがnode1で，比較ノードがnode2だった場合
    if(!ret && typeid(*this)==typeid(*lhs_) && typeid(n)==typeid(n_rhs)) {
      const BinaryNode* b_lhs = static_cast<const BinaryNode*>(lhs_.get());
      const BinaryNode* b_n_rhs = static_cast<const BinaryNode*>(&n_rhs);

      ret = 
        b_lhs->lhs_->is_same_struct(n_lhs, false) &&
        b_lhs->rhs_->is_same_struct(*b_n_rhs->lhs_.get(), false) &&
        rhs_->is_same_struct(*b_n_rhs->rhs_.get(), false);
    }
     

    // このノードがnode2で，比較ノードがnode1だった場合
    if(!ret && typeid(*this)==typeid(*rhs_) && typeid(n)==typeid(n_lhs)) {
      const BinaryNode* b_rhs = static_cast<const BinaryNode*>(rhs_.get());
      const BinaryNode* b_n_lhs = static_cast<const BinaryNode*>(&n_lhs);

      ret = 
        b_rhs->lhs_->is_same_struct(*b_n_lhs->rhs_.get(), false) &&
        b_rhs->rhs_->is_same_struct(n_rhs, false) &&
        lhs_->is_same_struct(*b_n_lhs->lhs_.get(), false);
    }
    */

bool Ask::is_same_struct(const Node& n, bool exactly_same) const
{
  return is_exactly_same(n, exactly_same);
}

bool Less::is_same_struct(const Node& n, bool exactly_same) const
{
  return is_exactly_same(n, exactly_same);
}

bool LessEqual::is_same_struct(const Node& n, bool exactly_same) const
{
  return is_exactly_same(n, exactly_same);
}

bool Greater::is_same_struct(const Node& n, bool exactly_same) const
{
  return is_exactly_same(n, exactly_same);
}

bool GreaterEqual::is_same_struct(const Node& n, bool exactly_same) const
{
  return is_exactly_same(n, exactly_same);
}

bool Subtract::is_same_struct(const Node& n, bool exactly_same) const
{
  return is_exactly_same(n, exactly_same);
}

bool Divide::is_same_struct(const Node& n, bool exactly_same) const
{
  return is_exactly_same(n, exactly_same);
}

bool Power::is_same_struct(const Node& n, bool exactly_same) const
{
  return is_exactly_same(n, exactly_same);
}

bool Weaker::is_same_struct(const Node& n, bool exactly_same) const
{
  return is_exactly_same(n, exactly_same);
}

bool Number::is_same_struct(const Node& n, bool exactly_same) const
{
  return typeid(*this) == typeid(n) &&
    number_ == static_cast<const Number*>(&n)->number_;          
}


bool Float::is_same_struct(const Node& n, bool exactly_same) const
{
  return typeid(*this) == typeid(n) &&
    number_ == static_cast<const Float*>(&n)->number_;          
}


bool True::is_same_struct(const Node& n, bool exactly_same) const
{
  return typeid(*this) == typeid(n);
}

bool False::is_same_struct(const Node& n, bool exactly_same) const
{
  return typeid(*this) == typeid(n);
}
bool EachElement::is_same_struct(const Node& n, bool exactly_same) const
{
  return typeid(*this) == typeid(n);
}

//Print
bool Print::is_same_struct(const Node& n, bool exactly_same) const
{
  return typeid(*this) == typeid(n) &&
          string_ == static_cast<const Print*>(&n)->string_;
}
bool PrintPP::is_same_struct(const Node& n, bool exactly_same) const
{
  return typeid(*this) == typeid(n) &&
          string_ == static_cast<const PrintPP*>(&n)->string_;
}
bool PrintIP::is_same_struct(const Node& n, bool exactly_same) const
{
  return typeid(*this) == typeid(n) &&
          string_ == static_cast<const PrintIP*>(&n)->string_;
}
bool Scan::is_same_struct(const Node& n, bool exactly_same) const
{
  return typeid(*this) == typeid(n) &&
          string_ == static_cast<const Scan*>(&n)->string_;
}
bool Exit::is_same_struct(const Node& n, bool exactly_same) const
{
  return typeid(*this) == typeid(n) &&
          string_ == static_cast<const Exit*>(&n)->string_;
}
bool Abort::is_same_struct(const Node& n, bool exactly_same) const
{
  return typeid(*this) == typeid(n) &&
          string_ == static_cast<const Abort*>(&n)->string_;
}

bool Variable::is_same_struct(const Node& n, bool exactly_same) const
{
  return typeid(*this) == typeid(n) &&
          name_ == static_cast<const Variable*>(&n)->name_;          
}


bool Parameter::is_same_struct(const Node& n, bool exactly_same) const
{
  return typeid(*this) == typeid(n) &&
          name_ == static_cast<const Parameter*>(&n)->name_;          
}

std::ostream& ProgramListCaller::dump(std::ostream& s) const 
{
  actual_args_t::const_iterator it  = actual_args_.begin();
  actual_args_t::const_iterator end = actual_args_.end();

  s << "program_list_call<" 
    << get_id()
    << ","
    << name_
    << "(";

  if(it!=end) s << **(it++);
  while(it!=end) {
    s << "," << **(it++);
  }

  s << ")>";
  if(child_) {
    s <<  "["
      << *child_
      << "]";
  }

  return s;
}

std::ostream& ExpressionListCaller::dump(std::ostream& s) const 
{
  actual_args_t::const_iterator it  = actual_args_.begin();
  actual_args_t::const_iterator end = actual_args_.end();

  s << "expression_list_call<" 
    << get_id()
    << ","
    << name_
    << "(";

  if(it!=end) s << **(it++);
  while(it!=end) {
    s << "," << **(it++);
  }

  s << ")>";
  if(child_) {
    s <<  "["
      << *child_
      << "]";
  }

  return s;
}

std::ostream& ConstraintCaller::dump(std::ostream& s) const 
{
  actual_args_t::const_iterator it  = actual_args_.begin();
  actual_args_t::const_iterator end = actual_args_.end();

  s << "constraint_call<" 
    << get_id()
    << ","
    << name_
    << "(";

  if(it!=end) s << **(it++);
  while(it!=end) {
    s << "," << **(it++);
  }

  s << ")>";
  if(child_) {
    s <<  "["
      << *child_
      << "]";
  }

  return s;
}

std::ostream& ProgramCaller::dump(std::ostream& s) const 
{
  actual_args_t::const_iterator it  = actual_args_.begin();
  actual_args_t::const_iterator end = actual_args_.end();

  s << "program_call<" 
    << get_id()
    << ","
    << name_
    << "(";

  if(it!=end) s << **(it++);
  while(it!=end) {
    s << "," << **(it++);
  }

  s << ")>";
  if(child_) {
    s <<  "["
      << *child_
      << "]";
  }

  return s;
}
std::ostream& Caller::dump(std::ostream& s) const 
{
  actual_args_t::const_iterator it  = actual_args_.begin();
  actual_args_t::const_iterator end = actual_args_.end();

  s << "call<" 
    << get_id()
    << ","
    << name_
    << "(";

  if(it!=end) s << **(it++);
  while(it!=end) {
    s << "," << **(it++);
  }

  s << ")>";
  if(child_) {
    s <<  "["
      << *child_
      << "]";
  }

  return s;
}

std::ostream& Definition::dump(std::ostream& s) const 
{
  bound_variables_t::const_iterator it  = bound_variables_.begin();
  bound_variables_t::const_iterator end = bound_variables_.end();

  s << name_
    << "<"
    << get_id()
    << ">(";

  if(it!=end) s << *(it++);
  while(it!=end) {
    s << "," << *(it++);
  }
  s << "):=" << *child_;

  return s;
}

node_sptr ExpressionList::clone(){
  node_type_sptr n(new ExpressionList(list_name_));
  for(unsigned int i=0;i<arguments_.size();i++){
    n->add_argument(arguments_[i]->clone());
  }
  return n;
}

node_sptr ConditionalExpressionList::clone(){
  node_type_sptr n(new ConditionalExpressionList(list_name_));
  n->set_expression(expression_);
  for(unsigned int i=0;i<arguments_.size();i++){
    n->add_argument(arguments_[i]->clone());
  }
  return n;
}

node_sptr ProgramList::clone(){
  node_type_sptr n(new ProgramList(list_name_));
  for(unsigned int i=0;i<arguments_.size();i++){
    n->add_argument(arguments_[i]->clone());
  }
  return n;
}

node_sptr ConditionalProgramList::clone(){
  node_type_sptr n(new ConditionalProgramList(list_name_));
  n->set_program(program_);
  for(unsigned int i=0;i<arguments_.size();i++){
    n->add_argument(arguments_[i]->clone());
  }
  return n;
}

node_sptr Function::clone(){
  node_type_sptr n(new Function(string_));
  for(unsigned int i=0;i<arguments_.size();i++){
    n->add_argument(arguments_[i]->clone());
  }
  return n;
}


node_sptr UnsupportedFunction::clone(){
  node_type_sptr n(new UnsupportedFunction(string_));
  for(unsigned int i=0;i<arguments_.size();i++){
    n->add_argument(arguments_[i]->clone());
  }
  return n;
}


void ArbitraryNode::accept(node_sptr own, 
                   BaseNodeVisitor* visitor) 
{
  assert(this == own.get()); 
  visitor->visit(boost::dynamic_pointer_cast<ArbitraryNode>(own));
}


std::ostream& ArbitraryNode::dump(std::ostream& s) const 
{
  Node::dump(s);
  s << "[" << get_string() << "]";
  s << "[";
  for(unsigned int i=0;i<arguments_.size();i++){
     s << *arguments_[i] << ",";
  }
  s << "]";
  return s;
}


void ArbitraryNode::add_argument(node_sptr node){
  arguments_.push_back(node);
}


void ArbitraryNode::set_argument(node_sptr node, int i){
  arguments_[i] = node;
}


int ArbitraryNode::get_arguments_size(){
  return arguments_.size();
}

node_sptr ArbitraryNode::get_argument(int i){
  return arguments_[i];
}

node_sptr Caller::clone()
{
  boost::shared_ptr<ProgramCaller> n(new ProgramCaller());
  n->name_ = name_;

  n->actual_args_.resize(actual_args_.size());
  copy(actual_args_.begin(), actual_args_.end(),  n->actual_args_.begin());
  
  if(child_) n->child_ = child_->clone();
  
  return n;
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

/**
 * 各ノードのaccept関数定義
 */

#define DEFINE_ACCEPT_FUNC(CLASS, VISITOR) \
  void CLASS::accept(node_sptr own, \
                     VISITOR* visitor) \
  { \
    assert(this == own.get()); \
    visitor->visit(boost::dynamic_pointer_cast<CLASS>(own)); \
  }

/// BaseNodeVisitorのaccept関数定義
#define DEFINE_BASE_NODE_VISITOR_ACCEPT_FUNC(CLASS) \
  DEFINE_ACCEPT_FUNC(CLASS, BaseNodeVisitor)

DEFINE_BASE_NODE_VISITOR_ACCEPT_FUNC(FactorNode)
DEFINE_BASE_NODE_VISITOR_ACCEPT_FUNC(UnaryNode)
DEFINE_BASE_NODE_VISITOR_ACCEPT_FUNC(BinaryNode)

/// TreeVisitorのaccept関数定義
#define DEFINE_TREE_VISITOR_ACCEPT_FUNC(CLASS) \
  DEFINE_ACCEPT_FUNC(CLASS, TreeVisitor)

//定義
DEFINE_TREE_VISITOR_ACCEPT_FUNC(ProgramDefinition)
DEFINE_TREE_VISITOR_ACCEPT_FUNC(ConstraintDefinition)

//呼び出し
DEFINE_TREE_VISITOR_ACCEPT_FUNC(ProgramCaller)
DEFINE_TREE_VISITOR_ACCEPT_FUNC(ConstraintCaller)
DEFINE_TREE_VISITOR_ACCEPT_FUNC(ExpressionListCaller)
DEFINE_TREE_VISITOR_ACCEPT_FUNC(ProgramListCaller)

 //制約式
DEFINE_TREE_VISITOR_ACCEPT_FUNC(Constraint);

//Tell制約
DEFINE_TREE_VISITOR_ACCEPT_FUNC(Tell)

//Ask制約
DEFINE_TREE_VISITOR_ACCEPT_FUNC(Ask)

//比較演算子
DEFINE_TREE_VISITOR_ACCEPT_FUNC(Equal)
DEFINE_TREE_VISITOR_ACCEPT_FUNC(UnEqual)
DEFINE_TREE_VISITOR_ACCEPT_FUNC(Less)
DEFINE_TREE_VISITOR_ACCEPT_FUNC(LessEqual)
DEFINE_TREE_VISITOR_ACCEPT_FUNC(Greater)
DEFINE_TREE_VISITOR_ACCEPT_FUNC(GreaterEqual)

//論理演算子
DEFINE_TREE_VISITOR_ACCEPT_FUNC(LogicalAnd)
DEFINE_TREE_VISITOR_ACCEPT_FUNC(LogicalOr)

//算術二項演算子
DEFINE_TREE_VISITOR_ACCEPT_FUNC(Plus)
DEFINE_TREE_VISITOR_ACCEPT_FUNC(Subtract)
DEFINE_TREE_VISITOR_ACCEPT_FUNC(Times)
DEFINE_TREE_VISITOR_ACCEPT_FUNC(Divide)
DEFINE_TREE_VISITOR_ACCEPT_FUNC(Power)

//算術単項演算子
DEFINE_TREE_VISITOR_ACCEPT_FUNC(Negative)
DEFINE_TREE_VISITOR_ACCEPT_FUNC(Positive)

//制約階層定義演算子
DEFINE_TREE_VISITOR_ACCEPT_FUNC(Weaker)
DEFINE_TREE_VISITOR_ACCEPT_FUNC(Parallel)

// 時相演算子
DEFINE_TREE_VISITOR_ACCEPT_FUNC(Always)

//微分
DEFINE_TREE_VISITOR_ACCEPT_FUNC(Differential)

//左極限
DEFINE_TREE_VISITOR_ACCEPT_FUNC(Previous)

//否定
DEFINE_TREE_VISITOR_ACCEPT_FUNC(Not)

//円周率
DEFINE_TREE_VISITOR_ACCEPT_FUNC(Pi)
//自然対数の底
DEFINE_TREE_VISITOR_ACCEPT_FUNC(E)

//Lists
DEFINE_TREE_VISITOR_ACCEPT_FUNC(ExpressionList)
DEFINE_TREE_VISITOR_ACCEPT_FUNC(ConditionalExpressionList)
DEFINE_TREE_VISITOR_ACCEPT_FUNC(ProgramList)
DEFINE_TREE_VISITOR_ACCEPT_FUNC(ConditionalProgramList)

DEFINE_TREE_VISITOR_ACCEPT_FUNC(SizeOfList);
DEFINE_TREE_VISITOR_ACCEPT_FUNC(SumOfList);
DEFINE_TREE_VISITOR_ACCEPT_FUNC(ExpressionListDefinition);
DEFINE_TREE_VISITOR_ACCEPT_FUNC(ProgramListDefinition);

DEFINE_TREE_VISITOR_ACCEPT_FUNC(ExpressionListElement)
DEFINE_TREE_VISITOR_ACCEPT_FUNC(ProgramListElement)
DEFINE_TREE_VISITOR_ACCEPT_FUNC(Range)
DEFINE_TREE_VISITOR_ACCEPT_FUNC(Union)
DEFINE_TREE_VISITOR_ACCEPT_FUNC(Intersection)
//ListCondition
DEFINE_TREE_VISITOR_ACCEPT_FUNC(EachElement)
DEFINE_TREE_VISITOR_ACCEPT_FUNC(DifferentVariable)

//任意の文字列
DEFINE_TREE_VISITOR_ACCEPT_FUNC(Function)
DEFINE_TREE_VISITOR_ACCEPT_FUNC(UnsupportedFunction)

//変数・束縛変数
DEFINE_TREE_VISITOR_ACCEPT_FUNC(Variable)

//数字
DEFINE_TREE_VISITOR_ACCEPT_FUNC(Number)
DEFINE_TREE_VISITOR_ACCEPT_FUNC(Float)

//Print
DEFINE_TREE_VISITOR_ACCEPT_FUNC(Print)
DEFINE_TREE_VISITOR_ACCEPT_FUNC(PrintPP)
DEFINE_TREE_VISITOR_ACCEPT_FUNC(PrintIP)
DEFINE_TREE_VISITOR_ACCEPT_FUNC(Scan)
DEFINE_TREE_VISITOR_ACCEPT_FUNC(Exit)
DEFINE_TREE_VISITOR_ACCEPT_FUNC(Abort)

//SystemVariable
DEFINE_TREE_VISITOR_ACCEPT_FUNC(SVtimer)

//記号定数
DEFINE_TREE_VISITOR_ACCEPT_FUNC(Parameter)

//t
DEFINE_TREE_VISITOR_ACCEPT_FUNC(SymbolicT)
//無限大
DEFINE_TREE_VISITOR_ACCEPT_FUNC(Infinity)
//True
DEFINE_TREE_VISITOR_ACCEPT_FUNC(True)

DEFINE_TREE_VISITOR_ACCEPT_FUNC(False)
} //namespace symbolic_expression
} //namespace hydla

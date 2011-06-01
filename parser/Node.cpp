#include "Node.h"

#include <assert.h>
#include <algorithm>
#include <typeinfo>

#include <boost/bind.hpp>

#include "ParseError.h"
#include "BaseNodeVisitor.h"
#include "TreeVisitor.h"
#include "Logger.h"

using namespace std;
using namespace boost;
using namespace hydla::parse_error;
using namespace hydla::logger;

namespace hydla { 
namespace parse_tree {



std::ostream& operator<<(std::ostream& s, const Node& node)
{
	if(Logger::ptflag==0){
    return node.dump(s);
  }else{
    return node.dump(s);
  }
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


    // ���Ӄm�[�h
    if(typeid(*n) == typeid(*lhs)) {
      create_child_node_list(cnl, lhs);
    }
    else {
      cnl.push_back(make_pair(lhs, false));
    }

    // �E�Ӄm�[�h
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
    // �o���̎q�m�[�h�̏W�������ꂩ�ǂ������ׂ�

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

    // �Ώې�
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
    // ��
    //
    // node2
    //   &
    //  / \  
    // a   &
    //    / \  
    //   b   c
    //
    // �̓��ꎋ

    // ���̃m�[�h��node1�ŁC��r�m�[�h��node2�������ꍇ
    if(!ret && typeid(*this)==typeid(*lhs_) && typeid(n)==typeid(n_rhs)) {
      const BinaryNode* b_lhs = static_cast<const BinaryNode*>(lhs_.get());
      const BinaryNode* b_n_rhs = static_cast<const BinaryNode*>(&n_rhs);

      ret = 
        b_lhs->lhs_->is_same_struct(n_lhs, false) &&
        b_lhs->rhs_->is_same_struct(*b_n_rhs->lhs_.get(), false) &&
        rhs_->is_same_struct(*b_n_rhs->rhs_.get(), false);
    }
     

    // ���̃m�[�h��node2�ŁC��r�m�[�h��node1�������ꍇ
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
 * �e�m�[�h��accept�֐���`
 */

#define DEFINE_ACCEPT_FUNC(CLASS, VISITOR) \
  void CLASS::accept(node_sptr own, \
                     VISITOR* visitor) \
  { \
    assert(this == own.get()); \
    visitor->visit(boost::shared_static_cast<CLASS>(own)); \
  }

/// BaseNodeVisitor��accept�֐���`
#define DEFINE_BASE_NODE_VISITOR_ACCEPT_FUNC(CLASS) \
  DEFINE_ACCEPT_FUNC(CLASS, BaseNodeVisitor)

DEFINE_BASE_NODE_VISITOR_ACCEPT_FUNC(FactorNode)
DEFINE_BASE_NODE_VISITOR_ACCEPT_FUNC(UnaryNode)
DEFINE_BASE_NODE_VISITOR_ACCEPT_FUNC(BinaryNode)

/// TreeVisitor��accept�֐���`
#define DEFINE_TREE_VISITOR_ACCEPT_FUNC(CLASS) \
  DEFINE_ACCEPT_FUNC(CLASS, TreeVisitor)

//��`
DEFINE_TREE_VISITOR_ACCEPT_FUNC(ProgramDefinition)
DEFINE_TREE_VISITOR_ACCEPT_FUNC(ConstraintDefinition)

//�Ăяo��
DEFINE_TREE_VISITOR_ACCEPT_FUNC(ProgramCaller)
DEFINE_TREE_VISITOR_ACCEPT_FUNC(ConstraintCaller)

 //����
DEFINE_TREE_VISITOR_ACCEPT_FUNC(Constraint);

//Tell����
DEFINE_TREE_VISITOR_ACCEPT_FUNC(Tell)

//Ask����
DEFINE_TREE_VISITOR_ACCEPT_FUNC(Ask)

//��r���Z�q
DEFINE_TREE_VISITOR_ACCEPT_FUNC(Equal)
DEFINE_TREE_VISITOR_ACCEPT_FUNC(UnEqual)
DEFINE_TREE_VISITOR_ACCEPT_FUNC(Less)
DEFINE_TREE_VISITOR_ACCEPT_FUNC(LessEqual)
DEFINE_TREE_VISITOR_ACCEPT_FUNC(Greater)
DEFINE_TREE_VISITOR_ACCEPT_FUNC(GreaterEqual)

//�_�����Z�q
DEFINE_TREE_VISITOR_ACCEPT_FUNC(LogicalAnd)
DEFINE_TREE_VISITOR_ACCEPT_FUNC(LogicalOr)

//�Z�p�񍀉��Z�q
DEFINE_TREE_VISITOR_ACCEPT_FUNC(Plus)
DEFINE_TREE_VISITOR_ACCEPT_FUNC(Subtract)
DEFINE_TREE_VISITOR_ACCEPT_FUNC(Times)
DEFINE_TREE_VISITOR_ACCEPT_FUNC(Divide)
DEFINE_TREE_VISITOR_ACCEPT_FUNC(Power)

//�Z�p�P�����Z�q
DEFINE_TREE_VISITOR_ACCEPT_FUNC(Negative)
DEFINE_TREE_VISITOR_ACCEPT_FUNC(Positive)

//����K�w��`���Z�q
DEFINE_TREE_VISITOR_ACCEPT_FUNC(Weaker)
DEFINE_TREE_VISITOR_ACCEPT_FUNC(Parallel)

// �������Z�q
DEFINE_TREE_VISITOR_ACCEPT_FUNC(Always)

//����
DEFINE_TREE_VISITOR_ACCEPT_FUNC(Differential)

//���Ɍ�
DEFINE_TREE_VISITOR_ACCEPT_FUNC(Previous)
//���Ɍ�
DEFINE_TREE_VISITOR_ACCEPT_FUNC(PreviousPoint)

//�ے�
DEFINE_TREE_VISITOR_ACCEPT_FUNC(Not)

//�ϐ��E�����ϐ�
DEFINE_TREE_VISITOR_ACCEPT_FUNC(Variable)

//����
DEFINE_TREE_VISITOR_ACCEPT_FUNC(Number)

//�L���萔
DEFINE_TREE_VISITOR_ACCEPT_FUNC(Parameter)

//t
DEFINE_TREE_VISITOR_ACCEPT_FUNC(SymbolicT)

} //namespace parse_tree
} //namespace hydla

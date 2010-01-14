#include "AskDisjunctionSplitter.h"

using namespace hydla::parse_tree;

namespace hydla {
namespace simulator {

AskDisjunctionSplitter::AskDisjunctionSplitter()
{}

AskDisjunctionSplitter::~AskDisjunctionSplitter()
{}

void AskDisjunctionSplitter::split(hydla::parse_tree::ParseTree* pt)
{
  pt_ = pt;
  pt->dispatch(this);
  pt->rebuild_node_id_list();
}

// ����Ăяo��
void AskDisjunctionSplitter::visit(boost::shared_ptr<hydla::parse_tree::ConstraintCaller> node)
{
  dispatch_unary_node(node);
}

// �v���O�����Ăяo��
void AskDisjunctionSplitter::visit(boost::shared_ptr<hydla::parse_tree::ProgramCaller> node)
{    
  dispatch_unary_node(node);
}

// ����
void AskDisjunctionSplitter::visit(boost::shared_ptr<hydla::parse_tree::Constraint> node)
{    
  dispatch_unary_node(node);
}

// Ask����
void AskDisjunctionSplitter::visit(boost::shared_ptr<hydla::parse_tree::Ask> node)
{
  splitted_guard_nodes_.clear();
  accept(node->get_guard());

  // ����
  if(splitted_guard_nodes_.size() > 1) {
    splitted_guard_nodes_t::iterator it       = splitted_guard_nodes_.begin();
    splitted_guard_nodes_t::iterator prev_end = --splitted_guard_nodes_.end();
    
    logical_and_sptr and_node = pt_->create_node<LogicalAnd>();
    new_child_ = and_node;

    // 1��
    and_node->set_lhs(create_ask_node(*it++, node->get_child()->clone()));

    // 2�`n-1��
    while(it!=prev_end) {
      logical_and_sptr new_and_node = pt_->create_node<LogicalAnd>();
      new_and_node->set_lhs(create_ask_node(*it++, node->get_child()->clone()));
      and_node->set_rhs(new_and_node);
      and_node = new_and_node;
    }

    // n��
    and_node->set_rhs(create_ask_node(*it++, node->get_child()->clone()));
  }
}

// Tell����
void AskDisjunctionSplitter::visit(boost::shared_ptr<hydla::parse_tree::Tell> node)
{
  // do nothing
}

// ��r���Z�q
void AskDisjunctionSplitter::visit(boost::shared_ptr<hydla::parse_tree::Equal> node)
{
  // do nothing
}

void AskDisjunctionSplitter::visit(boost::shared_ptr<hydla::parse_tree::UnEqual> node)
{
  // do nothing
}

void AskDisjunctionSplitter::visit(boost::shared_ptr<hydla::parse_tree::Less> node)
{
  // do nothing
}

void AskDisjunctionSplitter::visit(boost::shared_ptr<hydla::parse_tree::LessEqual> node)
{
  // do nothing
}

void AskDisjunctionSplitter::visit(boost::shared_ptr<hydla::parse_tree::Greater> node)
{
  // do nothing
}

void AskDisjunctionSplitter::visit(boost::shared_ptr<hydla::parse_tree::GreaterEqual> node)
{
  // do nothing
}

// �_�����Z�q
void AskDisjunctionSplitter::visit(boost::shared_ptr<hydla::parse_tree::LogicalAnd> node)
{
  // do nothing
}

void AskDisjunctionSplitter::visit(boost::shared_ptr<hydla::parse_tree::LogicalOr> node)
{
  // LHS 
  logical_or_sptr lhs_child = boost::dynamic_pointer_cast<LogicalOr>(node->get_lhs());
  if(lhs_child) {
    dispatch_lhs(node);
  }
  else {
    splitted_guard_nodes_.push_back(node->get_lhs());
  }

  // RHS 
  logical_or_sptr rhs_child = boost::dynamic_pointer_cast<LogicalOr>(node->get_rhs());
  if(rhs_child) {
    dispatch_rhs(node);
  }
  else {
    splitted_guard_nodes_.push_back(node->get_rhs());
  }
}

// ����K�w��`���Z�q
void AskDisjunctionSplitter::visit(boost::shared_ptr<hydla::parse_tree::Weaker> node)
{
  dispatch_binary_node(node);
}

void AskDisjunctionSplitter::visit(boost::shared_ptr<hydla::parse_tree::Parallel> node)
{
  dispatch_binary_node(node);
}

// �������Z�q
void AskDisjunctionSplitter::visit(boost::shared_ptr<hydla::parse_tree::Always> node)
{
  dispatch_unary_node(node);
}


} //namespace simulator
} //namespace hydla 
#include "InitNodeRemover.h"

#include <assert.h>

namespace hydla {
namespace simulator {
  
InitNodeRemover::InitNodeRemover()
{}

InitNodeRemover::~InitNodeRemover()
{}

void InitNodeRemover::apply(hydla::parse_tree::ParseTree* pt)
{
   pt->dispatch(this);
   pt->swap_tree(child_);
}

void InitNodeRemover::unary_node(boost::shared_ptr<hydla::parse_tree::UnaryNode> node)
{  
  accept(node->get_child());
  if(child_) {
    node->set_child(child_);
    child_ = node;
  }
}

void InitNodeRemover::binary_node(boost::shared_ptr<hydla::parse_tree::BinaryNode> node)
{
  accept(node->get_lhs());  
  node_sptr lhs_child = child_;

  accept(node->get_rhs());  
  node_sptr rhs_child = child_;

  if(lhs_child && rhs_child) {
    node->set_lhs(lhs_child);
    node->set_rhs(rhs_child);
    child_ = node;
  }
  else if(lhs_child) {
    child_ = lhs_child;
  }  
  else if(rhs_child) {
    child_ = rhs_child;
  }
  else {
    child_ = node_sptr();
  }
}

// Ask制約
void InitNodeRemover::visit(boost::shared_ptr<hydla::parse_tree::Ask> node)
{
  // 削除
  child_ = node_sptr();
}

// Tell制約
void InitNodeRemover::visit(boost::shared_ptr<hydla::parse_tree::Tell> node)
{
  // 削除
  child_ = node_sptr();
}

// 時相演算子
void InitNodeRemover::visit(boost::shared_ptr<hydla::parse_tree::Always> node)
{
  // 現状維持
  child_ = node;
}

// 制約式
void InitNodeRemover::visit(boost::shared_ptr<hydla::parse_tree::Constraint> node)
{
  unary_node(node);
}

// 論理積
void InitNodeRemover::visit(boost::shared_ptr<hydla::parse_tree::LogicalAnd> node)
{
  binary_node(node);
}

// モジュールの弱合成
void InitNodeRemover::visit(boost::shared_ptr<hydla::parse_tree::Weaker> node)
{
  binary_node(node);
}

// モジュールの並列合成
void InitNodeRemover::visit(boost::shared_ptr<hydla::parse_tree::Parallel> node)
{
  binary_node(node);
}

// 制約呼び出し
void InitNodeRemover::visit(boost::shared_ptr<hydla::parse_tree::ConstraintCaller> node)
{
  unary_node(node);
}

// プログラム呼び出し
void InitNodeRemover::visit(boost::shared_ptr<hydla::parse_tree::ProgramCaller> node)
{
  unary_node(node);
}

} //namespace simulator
} //namespace hydla 

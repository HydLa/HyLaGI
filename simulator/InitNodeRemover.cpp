#include "InitNodeRemover.h"

#include <assert.h>

namespace hydla {
namespace simulator {
  
InitNodeRemover::InitNodeRemover()
{}

InitNodeRemover::~InitNodeRemover()
{}

void InitNodeRemover::apply(boost::shared_ptr<hydla::parse_tree::ParseTree> pt)
{
 
}

// 制約式
void InitNodeRemover::visit(boost::shared_ptr<hydla::parse_tree::Constraint> node)
{
  accept(node->get_child());
}

// Ask制約
void InitNodeRemover::visit(boost::shared_ptr<hydla::parse_tree::Ask> node)
{

}

// Tell制約
void InitNodeRemover::visit(boost::shared_ptr<hydla::parse_tree::Tell> node)
{

}

// 論理積
void InitNodeRemover::visit(boost::shared_ptr<hydla::parse_tree::LogicalAnd> node)
{
  accept(node->get_lhs());
  accept(node->get_rhs());
}

// 時相演算子
void InitNodeRemover::visit(boost::shared_ptr<hydla::parse_tree::Always> node)
{

}

// モジュールの弱合成
void InitNodeRemover::visit(boost::shared_ptr<hydla::parse_tree::Weaker> node)
{
  accept(node->get_lhs());
  accept(node->get_rhs());
}

// モジュールの並列合成
void InitNodeRemover::visit(boost::shared_ptr<hydla::parse_tree::Parallel> node)
{
  accept(node->get_lhs());
  accept(node->get_rhs());
}

// 制約呼び出し
void InitNodeRemover::visit(boost::shared_ptr<hydla::parse_tree::ConstraintCaller> node)
{
  accept(node->get_child());
}

// プログラム呼び出し
void InitNodeRemover::visit(boost::shared_ptr<hydla::parse_tree::ProgramCaller> node)
{
  accept(node->get_child());
}

} //namespace simulator
} //namespace hydla 

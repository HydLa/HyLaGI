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

// ����
void InitNodeRemover::visit(boost::shared_ptr<hydla::parse_tree::Constraint> node)
{
  accept(node->get_child());
}

// Ask����
void InitNodeRemover::visit(boost::shared_ptr<hydla::parse_tree::Ask> node)
{

}

// Tell����
void InitNodeRemover::visit(boost::shared_ptr<hydla::parse_tree::Tell> node)
{

}

// �_����
void InitNodeRemover::visit(boost::shared_ptr<hydla::parse_tree::LogicalAnd> node)
{
  accept(node->get_lhs());
  accept(node->get_rhs());
}

// �������Z�q
void InitNodeRemover::visit(boost::shared_ptr<hydla::parse_tree::Always> node)
{

}

// ���W���[���̎㍇��
void InitNodeRemover::visit(boost::shared_ptr<hydla::parse_tree::Weaker> node)
{
  accept(node->get_lhs());
  accept(node->get_rhs());
}

// ���W���[���̕��񍇐�
void InitNodeRemover::visit(boost::shared_ptr<hydla::parse_tree::Parallel> node)
{
  accept(node->get_lhs());
  accept(node->get_rhs());
}

// ����Ăяo��
void InitNodeRemover::visit(boost::shared_ptr<hydla::parse_tree::ConstraintCaller> node)
{
  accept(node->get_child());
}

// �v���O�����Ăяo��
void InitNodeRemover::visit(boost::shared_ptr<hydla::parse_tree::ProgramCaller> node)
{
  accept(node->get_child());
}

} //namespace simulator
} //namespace hydla 

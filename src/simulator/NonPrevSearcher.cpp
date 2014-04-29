#include "NonPrevSearcher.h"


namespace hydla {
namespace simulator {

NonPrevSearcher::NonPrevSearcher()
{}

NonPrevSearcher::~NonPrevSearcher()
{}

void NonPrevSearcher::accept(const boost::shared_ptr<hydla::symbolic_expression::Node>& n)
{
  if(non_prev_ == false)
    n->accept(n, this);
}

bool NonPrevSearcher::judge_non_prev(boost::shared_ptr<symbolic_expression::Node> node)
{
  non_prev_ = false;
  in_prev_ = false;
  accept(node);
  return non_prev_;
}

// 左極限
void NonPrevSearcher::visit(boost::shared_ptr<hydla::symbolic_expression::Previous> node)
{
  in_prev_ = true;
  accept(node->get_child());
}


void NonPrevSearcher::visit(boost::shared_ptr<hydla::symbolic_expression::Variable> node)
{
  if(!in_prev_)
    non_prev_ = true;
}



} //namespace simulator
} //namespace hydla
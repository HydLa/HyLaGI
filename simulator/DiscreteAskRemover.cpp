#include "DiscreteAskRemover.h"

#include <boost/make_shared.hpp>

using namespace hydla::parse_tree;

namespace hydla {
namespace simulator {
  
DiscreteAskRemover::DiscreteAskRemover()
{}

DiscreteAskRemover::~DiscreteAskRemover()
{}

void DiscreteAskRemover::apply(hydla::parse_tree::ParseTree* pt)
{
   pt->dispatch(this);
   pt->swap_tree(child_);
}

void DiscreteAskRemover::dispatch(boost::shared_ptr<hydla::parse_tree::UnaryNode> node)
{  
  accept(node->get_child());
  if(child_) {
    node->set_child(child_);
    child_ = node;
  }
}

void DiscreteAskRemover::dispatch(boost::shared_ptr<hydla::parse_tree::BinaryNode> node)
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

// 制約呼び出し
void DiscreteAskRemover::visit(boost::shared_ptr<ConstraintCaller> node)      
{
  dispatch(node);
}

// プログラム呼び出し
void DiscreteAskRemover::visit(boost::shared_ptr<ProgramCaller> node)         
{
  dispatch(node);
}

// 制約式
void DiscreteAskRemover::visit(boost::shared_ptr<Constraint> node)            
{
  dispatch(node);
}

// Ask制約
void DiscreteAskRemover::visit(boost::shared_ptr<Ask> node)                   
{
  exist_prev_cons_ = false;
  accept(node->get_guard());
  if(exist_prev_cons_) {
    child_ = node_sptr();
  }
  else {
    child_ = node;
  }
}

// Tell制約
void DiscreteAskRemover::visit(boost::shared_ptr<Tell> node)                  
{
  child_ = node;
}

// 算術単項演算子
void DiscreteAskRemover::visit(boost::shared_ptr<Negative> node)
{
  dispatch(node);
}

void DiscreteAskRemover::visit(boost::shared_ptr<Positive> node)
{
  dispatch(node);
}

void DiscreteAskRemover::visit(boost::shared_ptr<Equal> node)                 
{
  dispatch(node);
}

void DiscreteAskRemover::visit(boost::shared_ptr<UnEqual> node)               
{
  dispatch(node);
}

void DiscreteAskRemover::visit(boost::shared_ptr<Less> node)
{
  dispatch(node);
}

void DiscreteAskRemover::visit(boost::shared_ptr<LessEqual> node)
{
  dispatch(node);
}

void DiscreteAskRemover::visit(boost::shared_ptr<Greater> node)
{
  dispatch(node);
}

void DiscreteAskRemover::visit(boost::shared_ptr<GreaterEqual> node)
{
  dispatch(node);
}

void DiscreteAskRemover::visit(boost::shared_ptr<Plus> node)
{
  dispatch(node);
}

void DiscreteAskRemover::visit(boost::shared_ptr<Subtract> node)
{
  dispatch(node);
}

void DiscreteAskRemover::visit(boost::shared_ptr<Times> node)
{
  dispatch(node);
}

void DiscreteAskRemover::visit(boost::shared_ptr<Divide> node)
{
  dispatch(node);
}

void DiscreteAskRemover::visit(boost::shared_ptr<LogicalAnd> node)
{
  dispatch(node);
}

void DiscreteAskRemover::visit(boost::shared_ptr<LogicalOr> node)
{
  dispatch(node);
}  

void DiscreteAskRemover::visit(boost::shared_ptr<Weaker> node)
{
  dispatch(node);
}

void DiscreteAskRemover::visit(boost::shared_ptr<Parallel> node)
{
  dispatch(node);
}
  
// 時相演算子
void DiscreteAskRemover::visit(boost::shared_ptr<Always> node)
{
  dispatch(node);
}
  
// 微分
void DiscreteAskRemover::visit(boost::shared_ptr<Differential> node)
{
  dispatch(node);
}

// 左極限
void DiscreteAskRemover::visit(boost::shared_ptr<Previous> node)
{  
  exist_prev_cons_ = true;
  dispatch(node);
}
  
// 変数
void DiscreteAskRemover::visit(boost::shared_ptr<Variable> node)
{
  child_ = node;
}

// 数字
void DiscreteAskRemover::visit(boost::shared_ptr<Number> node)
{
  child_ = node;
}

} //namespace simulator
} //namespace hydla 

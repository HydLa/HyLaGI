#include "AskTypeAnalyzer.h"

#include <boost/make_shared.hpp>

using namespace hydla::parse_tree;

namespace hydla {
namespace simulator {
  
AskTypeAnalyzer::AskTypeAnalyzer()
{}

AskTypeAnalyzer::~AskTypeAnalyzer()
{}

void AskTypeAnalyzer::analyze(hydla::parse_tree::ParseTree* pt)
{
  new_child_.reset();

  pt->dispatch(this);

  if(new_child_) {
    pt->swap_tree(new_child_);
  }
}

void AskTypeAnalyzer::dispatch(boost::shared_ptr<hydla::parse_tree::UnaryNode> node)
{  
  accept(node->get_child());
  if(new_child_) {
    node->set_child(new_child_);
    new_child_.reset();
  }
}

void AskTypeAnalyzer::dispatch(boost::shared_ptr<hydla::parse_tree::BinaryNode> node)
{
  accept(node->get_lhs());
  if(new_child_) {
    node->set_lhs(new_child_);
    new_child_.reset();
  }

  accept(node->get_rhs());
  if(new_child_) {
    node->set_rhs(new_child_);
    new_child_.reset();
  }
}

// 制約呼び出し
void AskTypeAnalyzer::visit(boost::shared_ptr<ConstraintCaller> node)      
{
  dispatch(node);
}

// プログラム呼び出し
void AskTypeAnalyzer::visit(boost::shared_ptr<ProgramCaller> node)         
{
  dispatch(node);
}

// 制約式
void AskTypeAnalyzer::visit(boost::shared_ptr<Constraint> node)            
{
  dispatch(node);
}

// Ask制約
void AskTypeAnalyzer::visit(boost::shared_ptr<Ask> node)                   
{
  exist_prev_cons_ = false;
  accept(node->get_guard());
  if(exist_prev_cons_) {
    new_child_ = boost::make_shared<DiscreteAsk>(
                    node->get_guard(), node->get_child());
  }
  else {
    new_child_ = boost::make_shared<ContinuousAsk>(
                    node->get_guard(), node->get_child());
  }
}

// Tell制約
void AskTypeAnalyzer::visit(boost::shared_ptr<Tell> node)                  
{
  // do nothing
}

// 算術単項演算子
void AskTypeAnalyzer::visit(boost::shared_ptr<Negative> node)
{
  dispatch(node);
}

void AskTypeAnalyzer::visit(boost::shared_ptr<Positive> node)
{
  dispatch(node);
}

void AskTypeAnalyzer::visit(boost::shared_ptr<Equal> node)                 
{
  dispatch(node);
}

void AskTypeAnalyzer::visit(boost::shared_ptr<UnEqual> node)               
{
  dispatch(node);
}

void AskTypeAnalyzer::visit(boost::shared_ptr<Less> node)
{
  dispatch(node);
}

void AskTypeAnalyzer::visit(boost::shared_ptr<LessEqual> node)
{
  dispatch(node);
}

void AskTypeAnalyzer::visit(boost::shared_ptr<Greater> node)
{
  dispatch(node);
}

void AskTypeAnalyzer::visit(boost::shared_ptr<GreaterEqual> node)
{
  dispatch(node);
}

void AskTypeAnalyzer::visit(boost::shared_ptr<Plus> node)
{
  dispatch(node);
}

void AskTypeAnalyzer::visit(boost::shared_ptr<Subtract> node)
{
  dispatch(node);
}

void AskTypeAnalyzer::visit(boost::shared_ptr<Times> node)
{
  dispatch(node);
}

void AskTypeAnalyzer::visit(boost::shared_ptr<Divide> node)
{
  dispatch(node);
}

void AskTypeAnalyzer::visit(boost::shared_ptr<LogicalAnd> node)
{
  dispatch(node);
}

void AskTypeAnalyzer::visit(boost::shared_ptr<LogicalOr> node)
{
  dispatch(node);
}  

void AskTypeAnalyzer::visit(boost::shared_ptr<Weaker> node)
{
  dispatch(node);
}

void AskTypeAnalyzer::visit(boost::shared_ptr<Parallel> node)
{
  dispatch(node);
}
  
// 時相演算子
void AskTypeAnalyzer::visit(boost::shared_ptr<Always> node)
{
  dispatch(node);
}
  
// 微分
void AskTypeAnalyzer::visit(boost::shared_ptr<Differential> node)
{
  dispatch(node);
}

// 左極限
void AskTypeAnalyzer::visit(boost::shared_ptr<Previous> node)
{  
  exist_prev_cons_ = true;
  dispatch(node);
}
  
  
// 以前のPPの値
void AskTypeAnalyzer::visit(boost::shared_ptr<PreviousPoint> node)
{  
  dispatch(node);
}
  
// 変数
void AskTypeAnalyzer::visit(boost::shared_ptr<Variable> node)
{
  //do nothing
}

// 数字
void AskTypeAnalyzer::visit(boost::shared_ptr<Number> node)
{
  //do nothing
}

} //namespace simulator
} //namespace hydla 

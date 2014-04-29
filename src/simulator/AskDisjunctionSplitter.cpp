#include "AskDisjunctionSplitter.h"

#include "Logger.h"
#include <iostream>

using namespace hydla::symbolic_expression;
using namespace hydla::logger;

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
  pt->update_node_id_list();
  HYDLA_LOGGER_DEBUG("#*** ask disjunction split result ***\n",
                     *pt, "\n",
                     pt->to_graphviz());
  
}

// 制約呼び出し
void AskDisjunctionSplitter::visit(boost::shared_ptr<hydla::symbolic_expression::ConstraintCaller> node)
{
  dispatch_unary_node(node);
}

// プログラム呼び出し
void AskDisjunctionSplitter::visit(boost::shared_ptr<hydla::symbolic_expression::ProgramCaller> node)
{    
  dispatch_unary_node(node);
}

// 制約式
void AskDisjunctionSplitter::visit(boost::shared_ptr<hydla::symbolic_expression::Constraint> node)
{    
  dispatch_unary_node(node);
}

// Ask制約
void AskDisjunctionSplitter::visit(boost::shared_ptr<hydla::symbolic_expression::Ask> node)
{
  splitted_guard_nodes_.clear();
  accept(node->get_guard());

  // 分割
  if(splitted_guard_nodes_.size() > 1) {
    splitted_guard_nodes_t::iterator it       = splitted_guard_nodes_.begin();
    splitted_guard_nodes_t::iterator prev_end = --splitted_guard_nodes_.end();
    
    logical_and_sptr and_node = pt_->create_node<LogicalAnd>();
    new_child_ = and_node;

    // 1個目
    and_node->set_lhs(create_ask_node(*it++, node->get_child()->clone()));

    // 2〜n-1個目
    while(it!=prev_end) {
      logical_and_sptr new_and_node = pt_->create_node<LogicalAnd>();
      new_and_node->set_lhs(create_ask_node(*it++, node->get_child()->clone()));
      and_node->set_rhs(new_and_node);
      and_node = new_and_node;
    }

    // n個目
    and_node->set_rhs(create_ask_node(*it++, node->get_child()->clone()));
  }
}

// Tell制約
void AskDisjunctionSplitter::visit(boost::shared_ptr<hydla::symbolic_expression::Tell> node)
{
  // do nothing
}

// 比較演算子
void AskDisjunctionSplitter::visit(boost::shared_ptr<hydla::symbolic_expression::Equal> node)
{
  // do nothing
}

void AskDisjunctionSplitter::visit(boost::shared_ptr<hydla::symbolic_expression::UnEqual> node)
{
  // do nothing
}

void AskDisjunctionSplitter::visit(boost::shared_ptr<hydla::symbolic_expression::Less> node)
{
  // do nothing
}

void AskDisjunctionSplitter::visit(boost::shared_ptr<hydla::symbolic_expression::LessEqual> node)
{
  // do nothing
}

void AskDisjunctionSplitter::visit(boost::shared_ptr<hydla::symbolic_expression::True> node)
{
  // do nothing
}

void AskDisjunctionSplitter::visit(boost::shared_ptr<hydla::symbolic_expression::Greater> node)
{
  // do nothing
}

void AskDisjunctionSplitter::visit(boost::shared_ptr<hydla::symbolic_expression::GreaterEqual> node)
{
  // do nothing
}

// 論理演算子
void AskDisjunctionSplitter::visit(boost::shared_ptr<hydla::symbolic_expression::LogicalAnd> node)
{
  // do nothing
}

void AskDisjunctionSplitter::visit(boost::shared_ptr<hydla::symbolic_expression::LogicalOr> node)
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


// 制約階層定義演算子
void AskDisjunctionSplitter::visit(boost::shared_ptr<hydla::symbolic_expression::Not> node)
{
  dispatch_unary_node(node);
}

// 制約階層定義演算子
void AskDisjunctionSplitter::visit(boost::shared_ptr<hydla::symbolic_expression::Weaker> node)
{
  dispatch_binary_node(node);
}

void AskDisjunctionSplitter::visit(boost::shared_ptr<hydla::symbolic_expression::Parallel> node)
{
  dispatch_binary_node(node);
}

// 時相演算子
void AskDisjunctionSplitter::visit(boost::shared_ptr<hydla::symbolic_expression::Always> node)
{
  dispatch_unary_node(node);
}


} //namespace simulator
} //namespace hydla 

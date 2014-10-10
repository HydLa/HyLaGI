#include "AskDisjunctionFormatter.h"

#include "Logger.h"
#include <iostream>
using namespace hydla::symbolic_expression;
using namespace hydla::logger;

namespace hydla {
namespace simulator {

AskDisjunctionFormatter::AskDisjunctionFormatter()
{}

AskDisjunctionFormatter::~AskDisjunctionFormatter()
{}

void AskDisjunctionFormatter::format(hydla::parse_tree::ParseTree* pt)
{
  HYDLA_LOGGER_DEBUG("#*** Begin AskDisjunctionFormatter::format ***\n");
  pt_ = pt;
  
  
  do {
    swapped_ = false;
    pt->dispatch(this);
  } while(swapped_);
  HYDLA_LOGGER_DEBUG("--- ask disjunction format result ---\n",
                     *pt, "\n",
                     pt->to_graphviz());
  HYDLA_LOGGER_DEBUG("#*** End AskDisjunctionFormatter::format ***\n");
}

// 制約呼び出し
void AskDisjunctionFormatter::visit(boost::shared_ptr<hydla::symbolic_expression::ConstraintCaller> node)
{
  dispatch_unary_node(node);
}

// プログラム呼び出し
void AskDisjunctionFormatter::visit(boost::shared_ptr<hydla::symbolic_expression::ProgramCaller> node)
{    
  dispatch_unary_node(node);
}

// 制約式
void AskDisjunctionFormatter::visit(boost::shared_ptr<hydla::symbolic_expression::Constraint> node)
{    
  dispatch_unary_node(node);
}

// Ask制約
void AskDisjunctionFormatter::visit(boost::shared_ptr<hydla::symbolic_expression::Ask> node)
{
  accept(node->get_guard());
  if(new_child_) {
    node->set_guard(new_child_);
    new_child_.reset();
  }
}

// Tell制約
void AskDisjunctionFormatter::visit(boost::shared_ptr<hydla::symbolic_expression::Tell> node)
{
  // do nothing
}

// 比較演算子
void AskDisjunctionFormatter::visit(boost::shared_ptr<hydla::symbolic_expression::Equal> node)
{
  // do nothing
}

void AskDisjunctionFormatter::visit(boost::shared_ptr<hydla::symbolic_expression::UnEqual> node)
{
  // do nothing
}

void AskDisjunctionFormatter::visit(boost::shared_ptr<hydla::symbolic_expression::Less> node)
{
  // do nothing
}

void AskDisjunctionFormatter::visit(boost::shared_ptr<hydla::symbolic_expression::LessEqual> node)
{
  // do nothing
}

void AskDisjunctionFormatter::visit(boost::shared_ptr<hydla::symbolic_expression::Greater> node)
{
  // do nothing
}

void AskDisjunctionFormatter::visit(boost::shared_ptr<hydla::symbolic_expression::True> node)
{
  // do nothing
}

void AskDisjunctionFormatter::visit(boost::shared_ptr<hydla::symbolic_expression::GreaterEqual> node)

{
  // do nothing
}

// 論理演算子
void AskDisjunctionFormatter::visit(boost::shared_ptr<hydla::symbolic_expression::LogicalAnd> node)
{
  boost::shared_ptr<hydla::symbolic_expression::BinaryNode> n(node);

  // andの左子ノードがorであった場合
  logical_or_sptr lhs_child = 
    boost::dynamic_pointer_cast<hydla::symbolic_expression::LogicalOr>(n->get_lhs());
  if(lhs_child) {
    logical_or_sptr node_or(new LogicalOr());

    logical_and_sptr lhs_and(new LogicalAnd());
    node_or->set_lhs(lhs_and);
    lhs_and->set_lhs(lhs_child->get_lhs());
    lhs_and->set_rhs(node->get_rhs());

    logical_and_sptr rhs_and(new LogicalAnd());
    node_or->set_rhs(rhs_and);
    rhs_and->set_lhs(lhs_child->get_rhs());
    rhs_and->set_rhs(n->get_rhs());

    n = node_or;
    swapped_ = true;
  }
  dispatch_lhs(n);        


  // andの右子ノードがorであった場合
  logical_or_sptr rhs_child = 
    boost::dynamic_pointer_cast<hydla::symbolic_expression::LogicalOr>(n->get_rhs());
  if(rhs_child) {
    logical_or_sptr node_or(new LogicalOr());

    logical_and_sptr lhs_and(new LogicalAnd());
    node_or->set_lhs(lhs_and);
    lhs_and->set_lhs(rhs_child->get_lhs());
    lhs_and->set_rhs(node->get_lhs());

    logical_and_sptr rhs_and(new LogicalAnd());
    node_or->set_rhs(rhs_and);
    rhs_and->set_lhs(rhs_child->get_rhs());
    rhs_and->set_rhs(n->get_lhs());

    n = node_or;
    swapped_ = true;
  }
  dispatch_rhs(n);        

  new_child_ = n;
}

void AskDisjunctionFormatter::visit(boost::shared_ptr<hydla::symbolic_expression::LogicalOr> node)
{
  dispatch_binary_node(node);
}


void AskDisjunctionFormatter::visit(boost::shared_ptr<hydla::symbolic_expression::Not> node)
{
  dispatch_unary_node(node);
}

// 制約階層定義演算子
void AskDisjunctionFormatter::visit(boost::shared_ptr<hydla::symbolic_expression::Weaker> node)
{
  dispatch_binary_node(node);
}

void AskDisjunctionFormatter::visit(boost::shared_ptr<hydla::symbolic_expression::Parallel> node)
{
  dispatch_binary_node(node);
}

// 時相演算子
void AskDisjunctionFormatter::visit(boost::shared_ptr<hydla::symbolic_expression::Always> node)
{
  dispatch_unary_node(node);
}

void AskDisjunctionFormatter::visit(boost::shared_ptr<hydla::symbolic_expression::Print> node)
{
  //dispatch_unary_node(node);
}



} //namespace simulator
} //namespace hydla 

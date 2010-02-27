#include "AskDisjunctionFormatter.h"

#include "Logger.h"

using namespace hydla::parse_tree;

namespace hydla {
namespace simulator {

AskDisjunctionFormatter::AskDisjunctionFormatter()
{}

AskDisjunctionFormatter::~AskDisjunctionFormatter()
{}

void AskDisjunctionFormatter::format(hydla::parse_tree::ParseTree* pt)
{
  pt_ = pt;

  do {
    swapped_ = false;
    pt->dispatch(this);
  } while(swapped_);

  pt->update_node_id_list();

  HYDLA_LOGGER_DEBUG("#*** ask disjunction format result ***\n",
                     *pt, "\n",
                     pt->to_graphviz());
}

// 制約呼び出し
void AskDisjunctionFormatter::visit(boost::shared_ptr<hydla::parse_tree::ConstraintCaller> node)
{
  dispatch_unary_node(node);
}

// プログラム呼び出し
void AskDisjunctionFormatter::visit(boost::shared_ptr<hydla::parse_tree::ProgramCaller> node)
{    
  dispatch_unary_node(node);
}

// 制約式
void AskDisjunctionFormatter::visit(boost::shared_ptr<hydla::parse_tree::Constraint> node)
{    
  dispatch_unary_node(node);
}

// Ask制約
void AskDisjunctionFormatter::visit(boost::shared_ptr<hydla::parse_tree::Ask> node)
{
  accept(node->get_guard());
  if(new_child_) {
    node->set_guard(new_child_);
    new_child_.reset();
  }
}

// Tell制約
void AskDisjunctionFormatter::visit(boost::shared_ptr<hydla::parse_tree::Tell> node)
{
  // do nothing
}

// 比較演算子
void AskDisjunctionFormatter::visit(boost::shared_ptr<hydla::parse_tree::Equal> node)
{
  // do nothing
}

void AskDisjunctionFormatter::visit(boost::shared_ptr<hydla::parse_tree::UnEqual> node)
{
  // do nothing
}

void AskDisjunctionFormatter::visit(boost::shared_ptr<hydla::parse_tree::Less> node)
{
  // do nothing
}

void AskDisjunctionFormatter::visit(boost::shared_ptr<hydla::parse_tree::LessEqual> node)
{
  // do nothing
}

void AskDisjunctionFormatter::visit(boost::shared_ptr<hydla::parse_tree::Greater> node)
{
  // do nothing
}

void AskDisjunctionFormatter::visit(boost::shared_ptr<hydla::parse_tree::GreaterEqual> node)

{
  // do nothing
}

// 論理演算子
void AskDisjunctionFormatter::visit(boost::shared_ptr<hydla::parse_tree::LogicalAnd> node)
{
  boost::shared_ptr<hydla::parse_tree::BinaryNode> n(node);

  // andの左子ノードがorであった場合
  logical_or_sptr lhs_child = 
    boost::dynamic_pointer_cast<hydla::parse_tree::LogicalOr>(n->get_lhs());
  if(lhs_child) {
    logical_or_sptr node_or(pt_->create_node<LogicalOr>());

    logical_and_sptr lhs_and(pt_->create_node<LogicalAnd>());
    node_or->set_lhs(lhs_and);
    lhs_and->set_lhs(lhs_child->get_lhs());
    lhs_and->set_rhs(node->get_rhs());

    logical_and_sptr rhs_and(pt_->create_node<LogicalAnd>());
    node_or->set_rhs(rhs_and);
    rhs_and->set_lhs(lhs_child->get_rhs());
    rhs_and->set_rhs(n->get_rhs());

    n = node_or;
    swapped_ = true;
  }
  dispatch_lhs(n);        


  // andの右子ノードがorであった場合
  logical_or_sptr rhs_child = 
    boost::dynamic_pointer_cast<hydla::parse_tree::LogicalOr>(n->get_rhs());
  if(rhs_child) {
    logical_or_sptr node_or(pt_->create_node<LogicalOr>());

    logical_and_sptr lhs_and(pt_->create_node<LogicalAnd>());
    node_or->set_lhs(lhs_and);
    lhs_and->set_lhs(rhs_child->get_lhs());
    lhs_and->set_rhs(node->get_lhs());

    logical_and_sptr rhs_and(pt_->create_node<LogicalAnd>());
    node_or->set_rhs(rhs_and);
    rhs_and->set_lhs(rhs_child->get_rhs());
    rhs_and->set_rhs(n->get_lhs());

    n = node_or;
    swapped_ = true;
  }
  dispatch_rhs(n);        

  new_child_ = n;
}

void AskDisjunctionFormatter::visit(boost::shared_ptr<hydla::parse_tree::LogicalOr> node)
{
  dispatch_binary_node(node);
}

// 制約階層定義演算子
void AskDisjunctionFormatter::visit(boost::shared_ptr<hydla::parse_tree::Weaker> node)
{
  dispatch_binary_node(node);
}

void AskDisjunctionFormatter::visit(boost::shared_ptr<hydla::parse_tree::Parallel> node)
{
  dispatch_binary_node(node);
}

// 時相演算子
void AskDisjunctionFormatter::visit(boost::shared_ptr<hydla::parse_tree::Always> node)
{
  dispatch_unary_node(node);
}


} //namespace simulator
} //namespace hydla 

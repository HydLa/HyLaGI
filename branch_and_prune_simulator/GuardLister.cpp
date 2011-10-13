#include "GuardLister.h"

using namespace hydla::parse_tree;

namespace hydla {
namespace bp_simulator {

GuardLister::GuardLister()
{}

GuardLister::~GuardLister()
{}

std::set<node_sptr> GuardLister::get_guard_list(boost::shared_ptr<Ask> node)
{
  this->accept(node);
  return this->nodes_;
}

// Ask§–ñ
void GuardLister::visit(boost::shared_ptr<Ask> node)
{
  this->accept(node->get_guard());
}

// ”äŠr‰‰Zq
void GuardLister::visit(boost::shared_ptr<Equal> node)
{
  this->nodes_.insert(node);
}

void GuardLister::visit(boost::shared_ptr<UnEqual> node)
{
  this->nodes_.insert(node);
}

void GuardLister::visit(boost::shared_ptr<Less> node)
{
  this->nodes_.insert(node);
}

void GuardLister::visit(boost::shared_ptr<LessEqual> node)
{
  this->nodes_.insert(node);
}

void GuardLister::visit(boost::shared_ptr<Greater> node)
{
  this->nodes_.insert(node);
}

void GuardLister::visit(boost::shared_ptr<GreaterEqual> node)
{
  this->nodes_.insert(node);
}

// ˜_—‰‰Zq
void GuardLister::visit(boost::shared_ptr<LogicalAnd> node)
{
  this->accept(node->get_lhs());
  this->accept(node->get_rhs());
}

} // namespace bp_simulator
} // namespace hydla
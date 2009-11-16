#include "TellCollector.h"

namespace hydla {
namespace simulator {
  
TellCollector::TellCollector()
{}

TellCollector::~TellCollector()
{}

void TellCollector::collect_tell(
  hydla::ch::module_set_sptr& ms,
  std::vector<boost::shared_ptr<hydla::parse_tree::Tell> >* new_tells,
  std::set<boost::shared_ptr<hydla::parse_tree::Tell> >*    collected_tells,
  std::set<boost::shared_ptr<hydla::parse_tree::Ask> >*     entailed_asks)
{
  new_tells_        = new_tells;
  collected_tells_  = collected_tells;
  entailed_asks_    = entailed_asks;

  new_tells->clear();
  ms->dispatch(this);
}

// 制約式
void TellCollector::visit(boost::shared_ptr<hydla::parse_tree::Constraint> node)
{
  node->get_child_node()->accept(node->get_child_node(), this);
}

// Ask制約
void TellCollector::visit(boost::shared_ptr<hydla::parse_tree::Ask> node)
{
  // askがテンテール可能であったら子ノードも探索する
  if(entailed_asks_->find(node) != entailed_asks_->end()) {
      node->get_child_node()->accept(node->get_child_node(), this);
  }
}

// Tell制約
void TellCollector::visit(boost::shared_ptr<hydla::parse_tree::Tell> node)
{
  // 収集済みでないtell制約であったら登録する
  if(collected_tells_->find(node) == collected_tells_->end()) {
    new_tells_->push_back(node);
    collected_tells_->insert(node);
  }
}

// 論理積
void TellCollector::visit(boost::shared_ptr<hydla::parse_tree::LogicalAnd> node)
{
  node->get_lhs()->accept(node->get_lhs(), this);
  node->get_rhs()->accept(node->get_rhs(), this);
}

// 時相演算子
void TellCollector::visit(boost::shared_ptr<hydla::parse_tree::Always> node)
{
  node->get_child_node()->accept(node->get_child_node(), this);
}

} //namespace simulator
} //namespace hydla 

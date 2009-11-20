#include "TellCollector.h"

#include <assert.h>

namespace hydla {
namespace simulator {
  
TellCollector::TellCollector()
{}

TellCollector::~TellCollector()
{}

void TellCollector::collect_tell(module_set_t*      ms,
                                 expanded_always_t* expanded_always,                   
                                 collected_tells_t* collected_tells,
                                 positive_asks_t*   positive_asks)
{
  assert(ms);
  assert(expanded_always);
  assert(collected_tells);
  assert(positive_asks);
  
  expanded_always_ = expanded_always;
  collected_tells_ = collected_tells;
  positive_asks_   = positive_asks;

   // ModuleSetのノードの探索
  in_expanded_always_ = false;
  ms->dispatch(this);
  
  // 展開済みalwaysノードの探索
  in_expanded_always_ = true;
  expanded_always_t::iterator it  = expanded_always->begin();
  expanded_always_t::iterator end = expanded_always->end();
  while(it!=end) {
    if(visited_always_.find(*it) != visited_always_.end()) {
      (*it)->accept(*it, this);
    }
  }
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
  if(positive_asks_->find(node) != positive_asks_->end()) {
    in_ask_ = true;
    node->get_child_node()->accept(node->get_child_node(), this);
    in_ask_ = false;
  }
}

// Tell制約
void TellCollector::visit(boost::shared_ptr<hydla::parse_tree::Tell> node)
{
  // tell制約の登録
  collected_tells_->insert(node);
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
  if(in_expanded_always_) {
    if(visited_always_.find(node) != visited_always_.end()) {
      node->get_child_node()->accept(node->get_child_node(), this);
    }
  } else {
    node->get_child_node()->accept(node->get_child_node(), this);
    visited_always_.insert(node);
  }
}

} //namespace simulator
} //namespace hydla 

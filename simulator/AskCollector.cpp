#include "AskCollector.h"

#include <assert.h>

#include <algorithm>

#include <boost/bind.hpp>
#include <boost/iterator/indirect_iterator.hpp>

using namespace std;
using namespace hydla::parse_tree;

namespace hydla {
namespace simulator {
  
AskCollector::AskCollector()
{}

AskCollector::~AskCollector()
{}

void AskCollector::collect_ask(module_set_t*      ms,
                               expanded_always_t* expanded_always,                   
                               positive_asks_t*   positive_asks,
                               negative_asks_t*   negative_asks)
{
  assert(ms);
  assert(expanded_always);
  assert(negative_asks);
  assert(positive_asks);

  negative_asks_   = negative_asks;
  positive_asks_   = positive_asks;

  // 各ノードの探索
  ms->dispatch(this);
  for_each(expanded_always->begin(), 
           expanded_always->end(),
           bind(&Always::accept, _1, _1, this));
}

// 制約式
void AskCollector::visit(boost::shared_ptr<hydla::parse_tree::Constraint> node)
{
  accept(node->get_child());
}

// Ask制約
void AskCollector::visit(boost::shared_ptr<hydla::parse_tree::Ask> node)
{
  if(positive_asks_->find(node) != positive_asks_->end()) {
    // 既に展開済みのaskノードであった場合
    accept(node->get_child());
  } else {
    // まだ展開されていないaskノードであった場合
    negative_asks_->insert(node);  
  }
}

// Tell制約
void AskCollector::visit(boost::shared_ptr<hydla::parse_tree::Tell> node)
{
  // do nothing
}

// 論理積
void AskCollector::visit(boost::shared_ptr<hydla::parse_tree::LogicalAnd> node)
{
  accept(node->get_lhs());
  accept(node->get_rhs());
}

// 時相演算子
void AskCollector::visit(boost::shared_ptr<hydla::parse_tree::Always> node)
{
  accept(node->get_child());
}

} //namespace simulator
} //namespace hydla 

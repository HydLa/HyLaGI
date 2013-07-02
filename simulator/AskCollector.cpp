#include "AskCollector.h"

#include <cassert>
#include <iostream>
#include <algorithm>
#include <sstream>

#include <boost/bind.hpp>
#include <boost/iterator/indirect_iterator.hpp>

#include "Logger.h"

using namespace std;
using namespace hydla::parse_tree;
using namespace hydla::logger;

namespace hydla {
namespace simulator {
  
namespace {
struct NodeDumper {
      
  template<typename T>
  NodeDumper(T it, T end) 
  {
    for(; it!=end; ++it) {
      ss << **it << "\n";
    }
  }

  friend std::ostream& operator<<(std::ostream& s, const NodeDumper& nd)
  {
    s << nd.ss.str();
    return s;
  }

  std::stringstream ss;
};
}

AskCollector::AskCollector(const module_set_sptr& module_set) :
  module_set_(module_set)
{}

AskCollector::~AskCollector()
{}

void AskCollector::collect_ask(expanded_always_t* expanded_always,                   
                               positive_asks_t*   positive_asks,
                               negative_asks_t*   negative_asks,
                               ask_set_t*         unknown_asks)
{
  assert(expanded_always);
  assert(negative_asks);
  assert(positive_asks);
  assert(unknown_asks);

  expanded_always_ = expanded_always;
  negative_asks_   = negative_asks;
  positive_asks_   = positive_asks;
  unknown_asks_    = unknown_asks;

  // ModuleSetのノードの探索
  in_positive_ask_    = false;
  module_set_->dispatch(this);

  // 展開済みalwaysノードの探索
  in_positive_ask_    = false;
  expanded_always_t::const_iterator it  = expanded_always->begin();
  expanded_always_t::const_iterator end = expanded_always->end();
  for(; it!=end; ++it) {
    // 採用しているモジュール集合内に入っているかどうか
    if(visited_always_.find(*it) != visited_always_.end()) {
      accept((*it)->get_child());
    }
  }

  // 展開済みalwaysノードのリストの更新
  expanded_always->insert(new_expanded_always_.begin(), 
                          new_expanded_always_.end());
}


// Ask制約
void AskCollector::visit(boost::shared_ptr<hydla::parse_tree::Ask> node)
{
  
  if(positive_asks_->find(node) != positive_asks_->end()) 
  {
    // 既に展開済みのaskノードであった場合
    if(in_positive_ask_){
      accept(node->get_child());
    }else{
      in_positive_ask_ = true;
      accept(node->get_child());
      in_positive_ask_ = false;
    }
  }
  else if(negative_asks_->find(node) == negative_asks_->end())
  {
    // 導出されるとも矛盾するとも判定されていないaskノードであった場合
    unknown_asks_->insert(node);
  }
}

// Tell制約
void AskCollector::visit(boost::shared_ptr<hydla::parse_tree::Tell> node)
{
  // do nothing
}

// 時相演算子
void AskCollector::visit(boost::shared_ptr<hydla::parse_tree::Always> node)
{
  accept(node->get_child());
  if(in_positive_ask_){
    new_expanded_always_.insert(node);
  }
  visited_always_.insert(node);
}


} //namespace simulator
} //namespace hydla
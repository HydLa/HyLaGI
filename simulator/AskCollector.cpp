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
                               negative_asks_t*   negative_asks)
{
  assert(expanded_always);
  assert(negative_asks);
  assert(positive_asks);

  expanded_always_ = expanded_always;
  negative_asks_   = negative_asks;
  positive_asks_   = positive_asks;

  // ModuleSetのノードの探索
  in_positive_ask_    = false;
  in_expanded_always_ = false;
  module_set_->dispatch(this);

  // 展開済みalwaysノードの探索
  in_positive_ask_    = false;
  in_expanded_always_ = true;
  expanded_always_t::const_iterator it  = expanded_always->begin();
  expanded_always_t::const_iterator end = expanded_always->end();
  for(; it!=end; ++it) {
    if(visited_always_.find(*it) != visited_always_.end()) {
      accept(*it);
    }
  }

  // 展開済みalwaysノードのリストの更新
  expanded_always->insert(new_expanded_always_.begin(), 
                          new_expanded_always_.end());

    
  HYDLA_LOGGER_DEBUG(
    "#*** ask collector ***\n", 
    "--- positive asks ---\n", 
    NodeDumper(positive_asks->begin(), positive_asks->end()),
    "--- negative asks ---\n",
    NodeDumper(negative_asks->begin(), negative_asks->end()));
//     "--- expanded always ---\n",
//     NodeDumper(expanded_always->begin(), expanded_always->end()));
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
    in_positive_ask_ = true;
    accept(node->get_child());
    in_positive_ask_ = false;
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
  if(in_expanded_always_) {
    if(visited_always_.find(node) != visited_always_.end()) {
      new_expanded_always_.insert(node);
      accept(node->get_child());
    }
  } else {
    accept(node->get_child());
    new_expanded_always_.insert(node);
    visited_always_.insert(node);
  }
}

} //namespace simulator
} //namespace hydla 

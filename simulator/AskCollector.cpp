#include "AskCollector.h"

#include <cassert>
#include <iostream>
#include <algorithm>
#include <sstream>

#include <boost/bind.hpp>
#include <boost/iterator/indirect_iterator.hpp>

#include "Logger.h"
#include "TypedAsk.h"

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

AskCollector::AskCollector(const module_set_sptr& module_set, 
                           collect_flag_t collect_type) :
  module_set_(module_set),
  collect_type_(collect_type)
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

  HYDLA_LOGGER_CLOSURE(
    "#*** ask collector ***\n", 
    "--- expanded always from previous phase ---\n",
    NodeDumper(expanded_always->begin(), expanded_always->end()));

  expanded_always_ = expanded_always;
  negative_asks_   = negative_asks;
  positive_asks_   = positive_asks;

  // ModuleSetのノードの探索
  in_positive_ask_    = false;
  in_negative_ask_    = false;
  in_expanded_always_ = false;
  module_set_->dispatch(this);

  // 展開済みalwaysノードの探索
  in_positive_ask_    = false;
  in_negative_ask_    = false;
  in_expanded_always_ = true;
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

  HYDLA_LOGGER_CLOSURE(
    "#*** ask collector ***\n", 
    "--- positive asks ---\n", 
    NodeDumper(positive_asks->begin(), positive_asks->end()),
    "--- negative asks ---\n",
    NodeDumper(negative_asks->begin(), negative_asks->end()));
  HYDLA_LOGGER_DEBUG(
	"--- expanded always ---\n",
    NodeDumper(expanded_always->begin(), expanded_always->end()));
}

// 制約式
void AskCollector::visit(boost::shared_ptr<hydla::parse_tree::Constraint> node)
{
  accept(node->get_child());
}

// Ask制約
void AskCollector::visit(boost::shared_ptr<hydla::parse_tree::Ask> node)
{

  if(!in_negative_ask_){
    bool collect = false;
    if(boost::dynamic_pointer_cast<DiscreteAsk>(node)) {
      // 離散変化ASK
      if(collect_type_ & ENABLE_COLLECT_DISCRETE_ASK) collect = true;
    }
    else if(boost::dynamic_pointer_cast<ContinuousAsk>(node)) {
      // 連続変化ASK
      if(collect_type_ & ENABLE_COLLECT_CONTINUOUS_ASK) collect = true;
    }
    else {
      // 型が存在しないASK
      if(collect_type_ & ENABLE_COLLECT_NON_TYPED_ASK) collect = true;
    }
    
    if(collect) {
      if(positive_asks_->find(node) != positive_asks_->end()) 
      {
        // 既に展開済みのaskノードであった場合
        in_positive_ask_ = true;
        accept(node->get_child());
        in_positive_ask_ = false;
      }
      else {
        // まだ展開されていないaskノードであった場合
        negative_asks_->insert(node);
        in_negative_ask_ = true;
        accept(node->get_child());
        in_negative_ask_ = false;
      }
    }
  } else {
    accept(node->get_child());
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
    if(visited_always_.find(node) != visited_always_.end() && (!in_negative_ask_)) {
      new_expanded_always_.insert(node);
      accept(node->get_child());
    }
  } else {
    if(!in_negative_ask_){
      accept(node->get_child());
      new_expanded_always_.insert(node);
    }
    visited_always_.insert(node);
  }
}


// モジュールの弱合成
void AskCollector::visit(boost::shared_ptr<hydla::parse_tree::Weaker> node)
{
  accept(node->get_lhs());
  accept(node->get_rhs());
}

// モジュールの並列合成
void AskCollector::visit(boost::shared_ptr<hydla::parse_tree::Parallel> node)
{
  accept(node->get_lhs());
  accept(node->get_rhs());
}

// 制約呼び出し
void AskCollector::visit(boost::shared_ptr<hydla::parse_tree::ConstraintCaller> node)
{
  accept(node->get_child());
}

// プログラム呼び出し
void AskCollector::visit(boost::shared_ptr<hydla::parse_tree::ProgramCaller> node)
{
  accept(node->get_child());
}

} //namespace simulator
} //namespace hydla 

#include "TellCollector.h"
#include "Logger.h"

#include <assert.h>
#include <iostream>

namespace hydla {
namespace simulator {
using namespace hydla::logger;


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

TellCollector::TellCollector(const module_set_sptr& module_set) :
  module_set_(module_set)
{}

TellCollector::~TellCollector()
{}

void TellCollector::collected_tells(tells_t* collected_tells)
{
  collected_tells->clear();
  collected_tells->reserve(collected_tells_.size());
  collected_tells->insert(collected_tells->end(), 
    collected_tells_.begin(), collected_tells_.end());
}

void TellCollector::collect(tells_t*                 tells,
                            const expanded_always_t* expanded_always,                   
                            const positive_asks_t*   positive_asks)
{
  assert(expanded_always);
  assert(tells);
  assert(positive_asks);

  tells->clear();
  tells_          = tells;
  positive_asks_  = positive_asks;
  visited_always_.clear();

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

  HYDLA_LOGGER_CC(
    "#*** tell collector ***\n", 
    "--- collected tells ---\n", 
    NodeDumper(tells->begin(), tells->end()));
}

// 制約式
void TellCollector::visit(boost::shared_ptr<hydla::parse_tree::Constraint> node)
{
  accept(node->get_child());
}

// Ask制約
void TellCollector::visit(boost::shared_ptr<hydla::parse_tree::Ask> node)
{
  // askがエンテール可能であったら子ノードも探索する
  if(positive_asks_->find(node) != positive_asks_->end()) {
    in_positive_ask_ = true;
    accept(node->get_child());
    in_positive_ask_ = false;
  } else {
    in_negative_ask_ = true;
    accept(node->get_child());
    in_negative_ask_ = false;
  }
}

// Tell制約
void TellCollector::visit(boost::shared_ptr<hydla::parse_tree::Tell> node)
{
  if(!in_negative_ask_){
    // tell制約の登録
    if(collect_all_tells_ || 
       collected_tells_.find(node) == collected_tells_.end()) 
    {
      tells_->push_back(node);
      collected_tells_.insert(node);
    }
  }
}

// 論理積
void TellCollector::visit(boost::shared_ptr<hydla::parse_tree::LogicalAnd> node)
{
  accept(node->get_lhs());
  accept(node->get_rhs());
}

// 時相演算子
void TellCollector::visit(boost::shared_ptr<hydla::parse_tree::Always> node)
{
  if(in_expanded_always_) {
    if(visited_always_.find(node) != visited_always_.end()) {
      accept(node->get_child());
    }
  } else {
    accept(node->get_child());
    visited_always_.insert(node);
  }
}

// モジュールの弱合成
void TellCollector::visit(boost::shared_ptr<hydla::parse_tree::Weaker> node)
{
  accept(node->get_lhs());
  accept(node->get_rhs());
}

// モジュールの並列合成
void TellCollector::visit(boost::shared_ptr<hydla::parse_tree::Parallel> node)
{
  accept(node->get_lhs());
  accept(node->get_rhs());
}

// 制約呼び出し
void TellCollector::visit(boost::shared_ptr<hydla::parse_tree::ConstraintCaller> node)
{
  accept(node->get_child());
}

// プログラム呼び出し
void TellCollector::visit(boost::shared_ptr<hydla::parse_tree::ProgramCaller> node)
{
  accept(node->get_child());
}





} //namespace simulator
} //namespace hydla 

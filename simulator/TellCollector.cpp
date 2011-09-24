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
  
TellCollector::TellCollector(const module_set_sptr& module_set, bool in_IP) :
  module_set_(module_set), in_interval_(in_IP)
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
  //variables_.clear();
  differential_count_ = 0;


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
  // askがテンテール可能であったら子ノードも探索する
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
      accept(node->get_child());
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

// 変数
void TellCollector::visit(boost::shared_ptr<hydla::parse_tree::Variable> node)
{
  continuity_map_t::iterator find = variables_.find(node->get_name());
  if(find == variables_.end() || find->second < differential_count_){
    variables_[node->get_name()] = differential_count_;
  }
}


// 微分
void TellCollector::visit(boost::shared_ptr<hydla::parse_tree::Differential> node)
{
  differential_count_++;
  accept(node->get_child());
  differential_count_--;
}


// 左極限
void TellCollector::visit(boost::shared_ptr<hydla::parse_tree::Previous> node)
{
  if(in_interval_){
    accept(node->get_child());
  }
}



#define DEFINE_DEFAULT_BINARY(NODE_NAME)                       \
void TellCollector::visit(boost::shared_ptr<hydla::parse_tree::NODE_NAME> node)             \
{                                                                       \
  accept(node->get_lhs());                                              \
  accept(node->get_rhs());                                              \
}

#define DEFINE_DEFAULT_UNARY(NODE_NAME)                        \
void TellCollector::visit(boost::shared_ptr<hydla::parse_tree::NODE_NAME> node)             \
{                                                                       \
  accept(node->get_child());                                            \
}

#define DEFINE_DEFAULT_FACTOR(NODE_NAME)                       \
void TellCollector::visit(boost::shared_ptr<hydla::parse_tree::NODE_NAME> node)             \
{                                                                       \
}

DEFINE_DEFAULT_BINARY(Equal)
DEFINE_DEFAULT_BINARY(UnEqual)
DEFINE_DEFAULT_BINARY(Less)
DEFINE_DEFAULT_BINARY(LessEqual)
DEFINE_DEFAULT_BINARY(Greater)
DEFINE_DEFAULT_BINARY(GreaterEqual)
DEFINE_DEFAULT_BINARY(LogicalOr)
DEFINE_DEFAULT_BINARY(Plus)
DEFINE_DEFAULT_BINARY(Subtract)
DEFINE_DEFAULT_BINARY(Times)
DEFINE_DEFAULT_BINARY(Divide)
DEFINE_DEFAULT_BINARY(Power)
DEFINE_DEFAULT_BINARY(ArbitraryBinary)
DEFINE_DEFAULT_BINARY(Log)

DEFINE_DEFAULT_UNARY(Positive)
DEFINE_DEFAULT_UNARY(Negative)
DEFINE_DEFAULT_UNARY(Not)
DEFINE_DEFAULT_UNARY(Sin)
DEFINE_DEFAULT_UNARY(Cos)
DEFINE_DEFAULT_UNARY(Tan)
DEFINE_DEFAULT_UNARY(Asin)
DEFINE_DEFAULT_UNARY(Acos)
DEFINE_DEFAULT_UNARY(Atan)
DEFINE_DEFAULT_UNARY(ArbitraryUnary)
DEFINE_DEFAULT_UNARY(Ln)

DEFINE_DEFAULT_FACTOR(E)
DEFINE_DEFAULT_FACTOR(Pi)
DEFINE_DEFAULT_FACTOR(ArbitraryFactor)
DEFINE_DEFAULT_FACTOR(Number)
DEFINE_DEFAULT_FACTOR(Parameter)
DEFINE_DEFAULT_FACTOR(SymbolicT)




} //namespace simulator
} //namespace hydla 

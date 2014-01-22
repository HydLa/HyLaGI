#include "TellCollector.h"
#include "Logger.h"

#include <assert.h>
#include <iostream>

namespace hydla {
namespace simulator {
using namespace hydla::logger;
using namespace hydla::parse_tree;

namespace {
struct NodeDumper {

  template<typename T>
  NodeDumper(T it, T end) 
  {
    for(; it!=end; ++it) {
      ss << get_infix_string(*it) << "\n";
    }
  }

  NodeDumper(const NodeDumper& rhs)
  {
    ss << rhs.ss.rdbuf();
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
  HYDLA_LOGGER_DEBUG("#*** Begin TellCollector::collect ***\n");
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

  HYDLA_LOGGER_DEBUG(
    "#*** tell collector ***\n", 
    "--- collected tells ---\n", 
    NodeDumper(tells->begin(), tells->end()));
    
  HYDLA_LOGGER_DEBUG("#*** End TellCollector::collect ***\n");
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


void TellCollector::visit(boost::shared_ptr<hydla::parse_tree::Print> node)
{
  std::string str =  node->get_string();
  std::string args = node->get_args();

  args.erase(0,2);
  std::string sb(" ");
  std::string sa("");
  std::string::size_type n, nb = 0;
  //スペースの削除
  while ((n = args.find(sb,nb)) != std::string::npos)
  {
    args.replace(n, sb.size(), sa);
    nb = n + sa.size();
  }
  //引数の分解
  uint i = 0;
  std::string key(",");
  v_print.push_back(str);
  
  while(i < args.length())
  {
    int old_i = i;
    i = args.find(key, i);
    if( i != std::string::npos)
    {
      std::string item = args.substr(old_i, i - old_i);
      v_print.push_back(item);
    }else{
      std::string item = args.substr(old_i);
      v_print.push_back(item);
      break;
    }
    i += key.length();
  }

}
void TellCollector::visit(boost::shared_ptr<hydla::parse_tree::PrintPP> node)
{
  std::string str =  node->get_string();
  std::string args = node->get_args();

  args.erase(0,2);
  std::string sb(" ");
  std::string sa("");
  std::string::size_type n, nb = 0;
  //スペースの削除
  while ((n = args.find(sb,nb)) != std::string::npos)
  {
    args.replace(n, sb.size(), sa);
    nb = n + sa.size();
  }
  //引数の分解
  uint i = 0;
  std::string key(",");
  v_print_pp.push_back(str);
  
  while(i < args.length())
  {
    int old_i = i;
    i = args.find(key, i);
    if( i != std::string::npos)
    {
      std::string item = args.substr(old_i, i - old_i);
      v_print_pp.push_back(item);
    }else{
      std::string item = args.substr(old_i);
      v_print_pp.push_back(item);
      break;
    }
    i += key.length();
  }

}
void TellCollector::visit(boost::shared_ptr<hydla::parse_tree::PrintIP> node)
{
  std::string str =  node->get_string();
  std::string args = node->get_args();

  args.erase(0,2);
  std::string sb(" ");
  std::string sa("");
  std::string::size_type n, nb = 0;
  //スペースの削除
  while ((n = args.find(sb,nb)) != std::string::npos)
  {
    args.replace(n, sb.size(), sa);
    nb = n + sa.size();
  }
  //引数の分解
  uint i = 0;
  std::string key(",");
  v_print_ip.push_back(str);
  
  while(i < args.length())
  {
    int old_i = i;
    i = args.find(key, i);
    if( i != std::string::npos)
    {
      std::string item = args.substr(old_i, i - old_i);
      v_print_ip.push_back(item);
    }else{
      std::string item = args.substr(old_i);
      v_print_ip.push_back(item);
      break;
    }
    i += key.length();
  }

}

void TellCollector::visit(boost::shared_ptr<hydla::parse_tree::Scan> node)
{
  std::string str =  node->get_string();
  std::string args = node->get_args();

  std::string sb(" ");
  std::string sa("");
  std::string::size_type n, nb = 0;
  //スペースの削除
  while ((n = args.find(sb,nb)) != std::string::npos)
  {
    args.replace(n, sb.size(), sa);
    nb = n + sa.size();
  }
  //引数の分解
  uint i = 0;
  std::string key(",");
  
  while(i < args.length())
  {
    int old_i = i;
    i = args.find(key, i);
    if( i != std::string::npos)
    {
      std::string item = args.substr(old_i, i - old_i);
      v_scan.push_back(item);
    }else{
      std::string item = args.substr(old_i);
      v_scan.push_back(item);
      break;
    }
    i += key.length();
  }
}

void TellCollector::visit(boost::shared_ptr<hydla::parse_tree::Exit> node)
{
  std::string str =  node->get_string();
  std::string args = node->get_args();

  args.erase(0,2);
  std::string sb(" ");
  std::string sa("");
  std::string::size_type n, nb = 0;
  //スペースの削除
  while ((n = args.find(sb,nb)) != std::string::npos)
  {
    args.replace(n, sb.size(), sa);
    nb = n + sa.size();
  }
  //引数の分解
  uint i = 0;
  std::string key(",");
  v_print_ip.push_back(str);
  
  while(i < args.length())
  {
    int old_i = i;
    i = args.find(key, i);
    if( i != std::string::npos)
    {
      std::string item = args.substr(old_i, i - old_i);
      v_print_ip.push_back(item);
    }else{
      std::string item = args.substr(old_i);
      v_print_ip.push_back(item);
      break;
    }
    i += key.length();
  }

}
    
void TellCollector::visit(boost::shared_ptr<hydla::parse_tree::Abort> node)
{
  std::string str =  node->get_string();
  std::string args = node->get_args();

  args.erase(0,2);
  std::string sb(" ");
  std::string sa("");
  std::string::size_type n, nb = 0;
  //スペースの削除
  while ((n = args.find(sb,nb)) != std::string::npos)
  {
    args.replace(n, sb.size(), sa);
    nb = n + sa.size();
  }
  //引数の分解
  uint i = 0;
  std::string key(",");
  v_print_ip.push_back(str);
  
  while(i < args.length())
  {
    int old_i = i;
    i = args.find(key, i);
    if( i != std::string::npos)
    {
      std::string item = args.substr(old_i, i - old_i);
      v_print_ip.push_back(item);
    }else{
      std::string item = args.substr(old_i);
      v_print_ip.push_back(item);
      break;
    }
    i += key.length();
  }

}


} //namespace simulator
} //namespace hydla 

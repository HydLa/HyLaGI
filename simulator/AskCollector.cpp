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

  // ModuleSet�̃m�[�h�̒T��
  in_positive_ask_    = false;
  module_set_->dispatch(this);

  // �W�J�ς�always�m�[�h�̒T��
  in_positive_ask_    = false;
  expanded_always_t::const_iterator it  = expanded_always->begin();
  expanded_always_t::const_iterator end = expanded_always->end();
  for(; it!=end; ++it) {
    // �̗p���Ă��郂�W���[���W�����ɓ����Ă��邩�ǂ���
    if(visited_always_.find(*it) != visited_always_.end()) {
      accept((*it)->get_child());
    }
  }

  // �W�J�ς�always�m�[�h�̃��X�g�̍X�V
  expanded_always->insert(new_expanded_always_.begin(), 
                          new_expanded_always_.end());
}


// Ask����
void AskCollector::visit(boost::shared_ptr<hydla::parse_tree::Ask> node)
{
  
  if(positive_asks_->find(node) != positive_asks_->end()) 
  {
    // ���ɓW�J�ς݂�ask�m�[�h�ł������ꍇ
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
    // ���o�����Ƃ���������Ƃ����肳��Ă��Ȃ�ask�m�[�h�ł������ꍇ
    unknown_asks_->insert(node);
  }
}

// Tell����
void AskCollector::visit(boost::shared_ptr<hydla::parse_tree::Tell> node)
{
  // do nothing
}

// �������Z�q
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
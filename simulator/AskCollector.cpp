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

  // ModuleSet�̃m�[�h�̒T��
  in_positive_ask_    = false;
  in_negative_ask_    = false;
  in_expanded_always_ = false;
  module_set_->dispatch(this);

  // �W�J�ς�always�m�[�h�̒T��
  in_positive_ask_    = false;
  in_negative_ask_    = false;
  in_expanded_always_ = true;
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

// ����
void AskCollector::visit(boost::shared_ptr<hydla::parse_tree::Constraint> node)
{
  accept(node->get_child());
}

// Ask����
void AskCollector::visit(boost::shared_ptr<hydla::parse_tree::Ask> node)
{

  if(!in_negative_ask_){
    bool collect = false;
    if(boost::dynamic_pointer_cast<DiscreteAsk>(node)) {
      // ���U�ω�ASK
      if(collect_type_ & ENABLE_COLLECT_DISCRETE_ASK) collect = true;
    }
    else if(boost::dynamic_pointer_cast<ContinuousAsk>(node)) {
      // �A���ω�ASK
      if(collect_type_ & ENABLE_COLLECT_CONTINUOUS_ASK) collect = true;
    }
    else {
      // �^�����݂��Ȃ�ASK
      if(collect_type_ & ENABLE_COLLECT_NON_TYPED_ASK) collect = true;
    }
    
    if(collect) {
      if(positive_asks_->find(node) != positive_asks_->end()) 
      {
        // ���ɓW�J�ς݂�ask�m�[�h�ł������ꍇ
        in_positive_ask_ = true;
        accept(node->get_child());
        in_positive_ask_ = false;
      }
      else {
        // �܂��W�J����Ă��Ȃ�ask�m�[�h�ł������ꍇ
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

// Tell����
void AskCollector::visit(boost::shared_ptr<hydla::parse_tree::Tell> node)
{
  // do nothing
}

// �_����
void AskCollector::visit(boost::shared_ptr<hydla::parse_tree::LogicalAnd> node)
{
  accept(node->get_lhs());
  accept(node->get_rhs());
}

// �������Z�q
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


// ���W���[���̎㍇��
void AskCollector::visit(boost::shared_ptr<hydla::parse_tree::Weaker> node)
{
  accept(node->get_lhs());
  accept(node->get_rhs());
}

// ���W���[���̕��񍇐�
void AskCollector::visit(boost::shared_ptr<hydla::parse_tree::Parallel> node)
{
  accept(node->get_lhs());
  accept(node->get_rhs());
}

// ����Ăяo��
void AskCollector::visit(boost::shared_ptr<hydla::parse_tree::ConstraintCaller> node)
{
  accept(node->get_child());
}

// �v���O�����Ăяo��
void AskCollector::visit(boost::shared_ptr<hydla::parse_tree::ProgramCaller> node)
{
  accept(node->get_child());
}

} //namespace simulator
} //namespace hydla 

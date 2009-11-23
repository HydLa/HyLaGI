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

   // ModuleSet�̃m�[�h�̒T��
  in_expanded_always_ = false;
  ms->dispatch(this);
  
  // �W�J�ς�always�m�[�h�̒T��
  in_expanded_always_ = true;
  expanded_always_t::iterator it  = expanded_always->begin();
  expanded_always_t::iterator end = expanded_always->end();
  while(it!=end) {
    if(visited_always_.find(*it) != visited_always_.end()) {
      accept(*it);
    }
  }
}

// ����
void TellCollector::visit(boost::shared_ptr<hydla::parse_tree::Constraint> node)
{
  accept(node->get_child());
}

// Ask����
void TellCollector::visit(boost::shared_ptr<hydla::parse_tree::Ask> node)
{
  // ask���e���e�[���\�ł�������q�m�[�h���T������
  if(positive_asks_->find(node) != positive_asks_->end()) {
    in_ask_ = true;
    accept(node->get_child());
    in_ask_ = false;
  }
}

// Tell����
void TellCollector::visit(boost::shared_ptr<hydla::parse_tree::Tell> node)
{
  // tell����̓o�^
  collected_tells_->insert(node);
}

// �_����
void TellCollector::visit(boost::shared_ptr<hydla::parse_tree::LogicalAnd> node)
{
  accept(node->get_lhs());
  accept(node->get_rhs());
}

// �������Z�q
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

} //namespace simulator
} //namespace hydla 

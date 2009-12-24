#include "TellCollector.h"

#include <assert.h>

namespace hydla {
namespace simulator {
  
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


  // ModuleSet�̃m�[�h�̒T��
  in_ask_             = false;
  in_expanded_always_ = false;
  module_set_->dispatch(this);

  // �W�J�ς�always�m�[�h�̒T��
  in_ask_             = false;
  in_expanded_always_ = true;
  expanded_always_t::const_iterator it  = expanded_always->begin();
  expanded_always_t::const_iterator end = expanded_always->end();
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
  if(collect_all_tells_ || 
      collected_tells_.find(node) == collected_tells_.end()) 
  {
    tells_->push_back(node);
    collected_tells_.insert(node);
  }
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

// ���W���[���̎㍇��
void TellCollector::visit(boost::shared_ptr<hydla::parse_tree::Weaker> node)
{
  accept(node->get_lhs());
  accept(node->get_rhs());
}

// ���W���[���̕��񍇐�
void TellCollector::visit(boost::shared_ptr<hydla::parse_tree::Parallel> node)
{
  accept(node->get_lhs());
  accept(node->get_rhs());
}

// ����Ăяo��
void TellCollector::visit(boost::shared_ptr<hydla::parse_tree::ConstraintCaller> node)
{
  accept(node->get_child());
}

// �v���O�����Ăяo��
void TellCollector::visit(boost::shared_ptr<hydla::parse_tree::ProgramCaller> node)
{
  accept(node->get_child());
}

} //namespace simulator
} //namespace hydla 

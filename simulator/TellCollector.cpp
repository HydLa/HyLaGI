#include "TellCollector.h"

namespace hydla {
namespace simulator {
  
TellCollector::TellCollector()
{}

TellCollector::~TellCollector()
{}

void TellCollector::collect_tell(
  hydla::ch::module_set_sptr& ms,
  std::vector<boost::shared_ptr<hydla::parse_tree::Tell> >* new_tells,
  std::set<boost::shared_ptr<hydla::parse_tree::Tell> >*    collected_tells,
  std::set<boost::shared_ptr<hydla::parse_tree::Ask> >*     entailed_asks)
{
  new_tells_        = new_tells;
  collected_tells_  = collected_tells;
  entailed_asks_    = entailed_asks;

  new_tells->clear();
  ms->dispatch(this);
}

// ����
void TellCollector::visit(boost::shared_ptr<hydla::parse_tree::Constraint> node)
{
  node->get_child_node()->accept(node->get_child_node(), this);
}

// Ask����
void TellCollector::visit(boost::shared_ptr<hydla::parse_tree::Ask> node)
{
  // ask���e���e�[���\�ł�������q�m�[�h���T������
  if(entailed_asks_->find(node) != entailed_asks_->end()) {
      node->get_child_node()->accept(node->get_child_node(), this);
  }
}

// Tell����
void TellCollector::visit(boost::shared_ptr<hydla::parse_tree::Tell> node)
{
  // ���W�ς݂łȂ�tell����ł�������o�^����
  if(collected_tells_->find(node) == collected_tells_->end()) {
    new_tells_->push_back(node);
    collected_tells_->insert(node);
  }
}

// �_����
void TellCollector::visit(boost::shared_ptr<hydla::parse_tree::LogicalAnd> node)
{
  node->get_lhs()->accept(node->get_lhs(), this);
  node->get_rhs()->accept(node->get_rhs(), this);
}

// �������Z�q
void TellCollector::visit(boost::shared_ptr<hydla::parse_tree::Always> node)
{
  node->get_child_node()->accept(node->get_child_node(), this);
}

} //namespace simulator
} //namespace hydla 

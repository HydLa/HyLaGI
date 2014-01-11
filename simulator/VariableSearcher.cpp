#include "VariableSearcher.h"
#include "Logger.h"


namespace hydla {
namespace simulator {

VariableSearcher::VariableSearcher()
{}

VariableSearcher::~VariableSearcher()
{}

bool VariableSearcher::visit_node(variable_set_t variables, boost::shared_ptr<parse_tree::Node> node, const bool& include_prev)
{
  include_prev_ = include_prev;
  in_prev_ = false;
  has_variables_ = false;
  variables_ = variables;
  accept(node);
  return has_variables_;
}

// AskêßñÒ
void VariableSearcher::visit(boost::shared_ptr<hydla::parse_tree::Ask> node)
{
  accept(node->get_guard());
  //accept(node->get_child());
}

// ïœêî
void VariableSearcher::visit(boost::shared_ptr<hydla::parse_tree::Variable> node)
{
  if(include_prev_ || !include_prev_ && !in_prev_)
    if(variables_.find(node->get_name()) != variables_.end())
      has_variables_ = true;
}

// ç∂ã…å¿
void VariableSearcher::visit(boost::shared_ptr<hydla::parse_tree::Previous> node)
{
  in_prev_ = true;
  accept(node->get_child());
  in_prev_ = false;
}

} //namespace simulator
} //namespace hydla 

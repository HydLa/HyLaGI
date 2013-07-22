#include "VariableSearcher.h"
#include "Logger.h"


namespace hydla {
namespace simulator {

VariableSearcher::VariableSearcher()
{}

VariableSearcher::~VariableSearcher()
{}

bool VariableSearcher::visit_node(variable_set_t variables, boost::shared_ptr<parse_tree::Node> node, const bool& in_IP)
{
  in_interval_ = in_IP;
  in_prev_ = false;
  has_variables_ = false;
  variables_ = variables;
  accept(node);
  return has_variables_;
}


void VariableSearcher::clear(){
  variables_.clear();
  prev_variables_.clear();
}



VariableSearcher::variable_set_t VariableSearcher::get_variable_set() const{ return variables_;}
VariableSearcher::variable_set_t VariableSearcher::get_prev_variable_set() const{return prev_variables_;}


// Ask§–ñ
void VariableSearcher::visit(boost::shared_ptr<hydla::parse_tree::Ask> node)
{
  accept(node->get_guard());
  //accept(node->get_child());
}

// ‚Æ‚è‚ ‚¦‚¸IP‚¾‚¯l‚¦‚é

// •Ï”
void VariableSearcher::visit(boost::shared_ptr<hydla::parse_tree::Variable> node)
{
 /* if(in_prev_){
    prev_variables_.insert(std::make_pair(node->get_name(), differential_count_));
  }else{*/
  //variables_.insert(node->get_name());
  if(variables_.find(node->get_name()) != variables_.end())
    has_variables_ = true;
}


} //namespace simulator
} //namespace hydla 

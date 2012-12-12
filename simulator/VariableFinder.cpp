#include "VariableFinder.h"
#include "Logger.h"


namespace hydla {
namespace simulator {

VariableFinder::VariableFinder()
{}

VariableFinder::~VariableFinder()
{}

void VariableFinder::visit_node(boost::shared_ptr<parse_tree::Node> node, const bool& in_IP)
{
  in_interval_ = in_IP;
  in_prev_ = false;
  differential_count_ = 0;
  accept(node);
}


void VariableFinder::clear(){
  variables_.clear();
  prev_variables_.clear();
}



VariableFinder::variable_set_t VariableFinder::get_variable_set() const{ return variables_;}
VariableFinder::variable_set_t VariableFinder::get_prev_variable_set() const{return prev_variables_;}


// Ask§–ñ
void VariableFinder::visit(boost::shared_ptr<hydla::parse_tree::Ask> node)
{
  accept(node->get_guard());
  accept(node->get_child());
}


// •Ï”
void VariableFinder::visit(boost::shared_ptr<hydla::parse_tree::Variable> node)
{
  if(in_prev_){
    prev_variables_.insert(std::make_pair(node->get_name(), differential_count_));
  }else{
    variables_.insert(std::make_pair(node->get_name(), differential_count_));
  }
}


// ”÷•ª
void VariableFinder::visit(boost::shared_ptr<hydla::parse_tree::Differential> node)
{
  differential_count_++;
  accept(node->get_child());
  differential_count_--;
}


// ¶‹ÉŒÀ
void VariableFinder::visit(boost::shared_ptr<hydla::parse_tree::Previous> node)
{
  if(!in_interval_){
    in_prev_ = true;
    accept(node->get_child());
    in_prev_ = false;
  }else{
    accept(node->get_child());
  }
}


} //namespace simulator
} //namespace hydla 

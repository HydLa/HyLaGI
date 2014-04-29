#include "VariableFinder.h"
#include "Variable.h"
#include "Logger.h"

using namespace std;

namespace hydla {
namespace simulator {

VariableFinder::VariableFinder()
{}

VariableFinder::~VariableFinder()
{}

void VariableFinder::visit_node(boost::shared_ptr<symbolic_expression::Node> node, bool include_guard)
{
  include_guard_ = include_guard;
  in_prev_ = false;
  differential_count_ = 0;
  accept(node);
}


void VariableFinder::clear(){
  variables_.clear();
  prev_variables_.clear();
}

bool VariableFinder::include_variables(std::set<std::string> variables) const
{
  for(auto found_var : variables_)
  {
    for(auto var : variables)
    {
      if(found_var.get_name() == var)
      {
        return true;
      }
    }
  }
  return false;
}


bool VariableFinder::include_variables_prev(std::set<std::string> variables) const
{
  for(auto found_var : prev_variables_)
  {
    for(auto var : variables)
    {
      if(found_var.get_name() == var)
      {
        return true;
      }
    }
  }
  return false;
}

VariableFinder::variable_set_t VariableFinder::get_all_variable_set() const
{
  variable_set_t merged_set(variables_);
  merged_set.insert(prev_variables_.begin(), prev_variables_.end());
  return merged_set;
}

VariableFinder::variable_set_t VariableFinder::get_variable_set() const{ return variables_;}
VariableFinder::variable_set_t VariableFinder::get_prev_variable_set() const{return prev_variables_;}


// Ask制約
void VariableFinder::visit(boost::shared_ptr<hydla::symbolic_expression::Ask> node)
{
  if(include_guard_)
  {
    accept(node->get_guard());
  }
  accept(node->get_child());
}


// 変数
void VariableFinder::visit(boost::shared_ptr<hydla::symbolic_expression::Variable> node)
{
  if(in_prev_){
    prev_variables_.insert(Variable(node->get_name(), differential_count_));
  }else{
    for(int i=0; i <= differential_count_; i++){
      variables_.insert(Variable(node->get_name(), i));
    }
  }
}


// 微分
void VariableFinder::visit(boost::shared_ptr<hydla::symbolic_expression::Differential> node)
{
  differential_count_++;
  accept(node->get_child());
  differential_count_--;
}


// 左極限
void VariableFinder::visit(boost::shared_ptr<hydla::symbolic_expression::Previous> node)
{
  in_prev_ = true;
  accept(node->get_child());
  in_prev_ = false;
}


} //namespace simulator
} //namespace hydla 

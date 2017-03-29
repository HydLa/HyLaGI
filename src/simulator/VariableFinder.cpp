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

void VariableFinder::visit_node(boost::shared_ptr<symbolic_expression::Node> node)
{
  in_prev_ = false;
  differential_count_ = 0;
  accept(node);
}

void VariableFinder::clear(){
  variables_.clear();
  prev_variables_.clear();
}


bool VariableFinder::include_variable(const Variable &variable) const
{
  for(auto found_var : variables_)
  {
    if(variable == found_var)
    {
      return true;
    }
  }
  return false;
}


bool VariableFinder::include_variable_prev(const Variable &variable) const
{
  for(auto found_var : prev_variables_)
  {
    if(variable == found_var)
    {
      return true;
    }
  }
  return false;
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


bool VariableFinder::include_variables(const boost::shared_ptr<symbolic_expression::Node> &constraint) const
{
  VariableFinder tmp_finder;
  tmp_finder.visit_node(constraint);
  for(auto found_var : variables_)
  {
    if(tmp_finder.include_variable(found_var))return true;
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

variable_set_t VariableFinder::get_all_variable_set() const
{
  variable_set_t merged_set(variables_);
  merged_set.insert(prev_variables_.begin(), prev_variables_.end());
  return merged_set;
}

variable_set_t VariableFinder::get_variable_set() const{ return variables_;}
variable_set_t VariableFinder::get_prev_variable_set() const{return prev_variables_;}


// Ask制約
void VariableFinder::visit(boost::shared_ptr<hydla::symbolic_expression::Ask> node)
{
  accept(node->get_guard());
  accept(node->get_child());
}


// 時刻
void VariableFinder::visit(boost::shared_ptr<hydla::symbolic_expression::SymbolicT> node)
{
  variables_.insert(Variable("t", 0));  //Since t is always continuous, it can be regarded as previous value (for simplicity of simulation)
}

// 変数
void VariableFinder::visit(boost::shared_ptr<hydla::symbolic_expression::Variable> node)
{
  if(in_prev_){
    prev_variables_.insert(Variable(node->get_name(), differential_count_));
  }else{
    variables_.insert(Variable(node->get_name(), differential_count_));
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

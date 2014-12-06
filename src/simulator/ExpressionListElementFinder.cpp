#include "ExpressionListElementFinder.h"
#include "Variable.h"
#include "Logger.h"

using namespace std;

namespace hydla {
namespace simulator {

ExpressionListElementFinder::ExpressionListElementFinder()
{}

ExpressionListElementFinder::~ExpressionListElementFinder()
{}

void ExpressionListElementFinder::visit_node(boost::shared_ptr<symbolic_expression::Node> node)
{
  in_prev_ = false;
  differential_count_ = 0;
  accept(node);
}

void ExpressionListElementFinder::clear(){
  variables_.clear();
  prev_variables_.clear();
}

list_element_data_set_t ExpressionListElementFinder::get_all_variable_set() const
{
  list_element_data_set_t merged_set(variables_);
  merged_set.insert(prev_variables_.begin(), prev_variables_.end());
  return merged_set;
}

list_element_data_set_t ExpressionListElementFinder::get_variable_set() const{ return variables_;}
list_element_data_set_t ExpressionListElementFinder::get_prev_variable_set() const{return prev_variables_;}


// Ask制約
void ExpressionListElementFinder::visit(boost::shared_ptr<hydla::symbolic_expression::Ask> node)
{
  accept(node->get_guard());
  accept(node->get_child());
}

void ExpressionListElementFinder::visit(boost::shared_ptr<hydla::symbolic_expression::ExpressionListElement> node)
{
  if(in_prev_){
    bool inserted = false;
    for(auto prev_var : prev_variables_)
    {
      if(prev_var->node->is_same_struct(*node,true))
      {
        inserted = true;
        break;
      }
    }
    if(!inserted)
    {
      prev_variables_.insert(boost::shared_ptr<ListElementData>(new ListElementData(node, differential_count_)));
    }
  }else{
    for(int i=0; i <= differential_count_; i++){
      bool inserted = false;
      for(auto var : variables_)
      {
        if(var->node->is_same_struct(*node,true) && i==var->differential_count)
        {
          inserted = true;
          break;
        }
      }
      if(!inserted){
        variables_.insert(boost::shared_ptr<ListElementData>(new ListElementData(node, i)));
      }
    }
  }
}


// 微分
void ExpressionListElementFinder::visit(boost::shared_ptr<hydla::symbolic_expression::Differential> node)
{
  differential_count_++;
  accept(node->get_child());
  differential_count_--;
}


// 左極限
void ExpressionListElementFinder::visit(boost::shared_ptr<hydla::symbolic_expression::Previous> node)
{
  in_prev_ = true;
  accept(node->get_child());
  in_prev_ = false;
}


} //namespace simulator
} //namespace hydla 

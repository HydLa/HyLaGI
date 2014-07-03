#include "ConstraintDifferenceCalculator.h"
#include "VariableFinder.h"

namespace hydla
{
namespace simulator
{

void ConstraintDifferenceCalculator::set_changing_constraints(const ConstraintStore& constraints)
{
  ConstraintStore tmp_constraints;
  module_set_t module_set;
  //TODO: IPでprevを区別する
  for(auto constraint : constraints){
    relation_graph_->get_related_constraints(constraint, tmp_constraints, module_set);
    changing_constraints_.add_constraint_store(tmp_constraints);
  }
}

ConstraintStore ConstraintDifferenceCalculator::get_changing_constraints(){
  return changing_constraints_;
}

bool ConstraintDifferenceCalculator::is_changing(const ConstraintStore constraint_store)
{
  set_changing_constraints(changing_constraints_);
  for(auto constraint : constraint_store){
    if(changing_constraints_.count(constraint)) return true;
  }
  return false;
}

bool ConstraintDifferenceCalculator::is_changing(const constraint_t constraint)
{
  ConstraintStore constraint_store;
  module_set_t module_set;
  relation_graph_->get_related_constraints(constraint, constraint_store, module_set);
  for(auto tmp_constraint : constraint_store){
    if(changing_constraints_.count(tmp_constraint)) return true;
  }
  return false;
}

bool ConstraintDifferenceCalculator::is_changing(const Variable& variable)
{
  VariableFinder finder;
  for(auto constraint : changing_constraints_){
    finder.visit_node(constraint);
  }
  return finder.include_variable(variable) || finder.include_variable_prev(variable);
}

void ConstraintDifferenceCalculator::clear_changing(){
  changing_constraints_.clear();
}

bool ConstraintDifferenceCalculator::is_continuous(const constraint_t constraint)
{
  /*
  VariableFinder finder;
  finder.visit_node(constraint);
  variable_set_t variables(finder.get_all_variable_set());
  for(auto variable : variables){

  }
  */
  return false;
}

void ConstraintDifferenceCalculator::set_relation_graph(const boost::shared_ptr<RelationGraph> graph){
  relation_graph_ = graph;
}


}
}

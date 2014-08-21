#include "ConstraintDifferenceCalculator.h"
#include "VariableFinder.h"

using namespace std;

namespace hydla
{
namespace simulator
{

void ConstraintDifferenceCalculator::calculate_difference_constraints(const simulation_todo_sptr_t todo, const boost::shared_ptr<RelationGraph> relation_graph){
  difference_constraints_.clear();
  
  ConstraintStore current_constraints = relation_graph->get_constraints();
  ConstraintStore difference_constraints;
  if(todo->parent->phase_type == IntervalPhase){
    set_symmetric_difference(todo->parent->current_constraints, current_constraints, difference_constraints);
  }
  else{
    set_symmetric_difference(todo->parent->current_constraints, current_constraints, difference_constraints);
    difference_constraints.add_constraint_store(todo->parent->changed_constraints);
    for(auto cause : todo->discrete_causes_map){
      if(!cause.second) difference_constraints.add_constraint(cause.first->get_child());
    }
  }
  
  ConstraintStore tmp_constraints;
  module_set_t module_set;

  //TODO: IPでprevを区別する
  for(auto constraint : difference_constraints){
    add_difference_constraints(constraint, relation_graph);
  }
}

void ConstraintDifferenceCalculator::add_difference_constraints(const constraint_t constraint, const boost::shared_ptr<RelationGraph> relation_graph){
  ConstraintStore tmp_constraints;
  module_set_t module_set;
  difference_constraints_.add_constraint(constraint);
  relation_graph->get_related_constraints(constraint, tmp_constraints, module_set);
  difference_constraints_.add_constraint_store(tmp_constraints);
}

ConstraintStore ConstraintDifferenceCalculator::get_difference_constraints(){
  return difference_constraints_;
}

bool ConstraintDifferenceCalculator::is_continuous(const simulation_todo_sptr_t todo, const ask_t ask)
{
  VariableFinder finder;
  finder.visit_node(ask->get_guard());
  variable_set_t variables(finder.get_all_variable_set());
  finder.clear();
  for(auto cause : todo->discrete_causes_map){
    if(!cause.second) finder.visit_node(cause.first->get_child());
  }
  variable_set_t changing_variables(finder.get_all_variable_set());

  for(auto variable : variables){
    auto differential_pair = todo->parent->variable_map.find(Variable(variable.get_name(), variable.get_differential_count() + 1));
    if(differential_pair == todo->parent->variable_map.end() || differential_pair->second.undefined()) return false;
    for(auto cv : changing_variables){
      if(variable.get_name() == cv.get_name()) return false;
    }
  }
  return true;
}

void ConstraintDifferenceCalculator::collect_ask( const boost::shared_ptr<AskRelationGraph> ask_relation_graph,
    const std::vector<ask_t> &discrete_causes,
    const ask_set_t &positive_asks,
    const ask_set_t &negative_asks,
    ask_set_t &unknown_asks){

  VariableFinder finder;
  for(auto constraint : difference_constraints_){
    finder.visit_node(constraint);
  }
  //TODO: prevを区別する
  variable_set_t variables = finder.get_all_variable_set();
  AskRelationGraph::asks_t asks;
  for(auto variable : variables){
    ask_relation_graph->get_adjacent_asks(variable.get_name(), asks);
    for(auto ask : asks){
      if(!positive_asks.count(ask) && !negative_asks.count(ask)) unknown_asks.insert(ask);
    }
  }
  for(auto ask : discrete_causes){
    if(!positive_asks.count(ask) && !negative_asks.count(ask)) unknown_asks.insert(ask);
  }

/*
  std::cout<<"positive_asks: "<<positive_asks<<std::endl;
  std::cout<<"negative_asks: "<<negative_asks<<std::endl;
  std::cout<<"unknown_asks: "<<unknown_asks<<std::endl;
*/
}

void ConstraintDifferenceCalculator::set_symmetric_difference(
    const ConstraintStore& parent_constraints,
    const ConstraintStore& current_constraints,
    ConstraintStore& result ){
  ConstraintStore::iterator it1 = parent_constraints.begin();
  ConstraintStore::iterator it2 = current_constraints.begin();
  while(it1 != parent_constraints.end() && it2 != current_constraints.end()){
    if(*it1 < *it2){
      result.insert(*it1);
      it1++;
    }else if(*it2 < *it1){
      result.insert(*it2);
      it2++;
    }else{
      it1++;
      it2++;
    }
  }
  while(it1 != parent_constraints.end()){
    result.insert(*it1);
    it1++;
  }
  while(it2 != current_constraints.end()){
    result.insert(*it2);
    it2++;
  }
}


}
}

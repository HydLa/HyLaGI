#include "ConstraintDifferenceCalculator.h"
#include "VariableFinder.h"

namespace hydla
{
namespace simulator
{

void ConstraintDifferenceCalculator::calculate_difference_constraints(const phase_result_sptr_t parent, const boost::shared_ptr<RelationGraph> relation_graph){
  difference_constraints_.clear();
  
  ConstraintStore current_constraints = relation_graph->get_constraints();
  ConstraintStore difference_constraints;
  if(parent->phase_type == IntervalPhase){
    set_symmetric_difference(parent->current_constraints, current_constraints, difference_constraints);
  }
  else{
    difference_constraints = parent->changed_constraints;
  }
  
  ConstraintStore tmp_constraints;
  module_set_t module_set;
  //TODO: IPでprevを区別する
  for(auto constraint : difference_constraints){
    relation_graph->get_related_constraints(constraint, tmp_constraints, module_set);
    difference_constraints_.add_constraint_store(tmp_constraints);
  }
}

void ConstraintDifferenceCalculator::add_diference_constraints(const constraint_t constraint, const boost::shared_ptr<RelationGraph> relation_graph){
  ConstraintStore tmp_constraints;
  module_set_t module_set;
  relation_graph->get_related_constraints(constraint, tmp_constraints, module_set);
  difference_constraints_.add_constraint_store(tmp_constraints);
}

ConstraintStore ConstraintDifferenceCalculator::get_difference_constraints(){
  return difference_constraints_;
}

bool ConstraintDifferenceCalculator::is_continuous(const phase_result_sptr_t parent, const constraint_t constraint)
{
  
  VariableFinder finder;
  finder.visit_node(constraint);
  variable_set_t variables(finder.get_all_variable_set());
  for(auto variable : variables){
    auto differential_pair = parent->variable_map.find(Variable(variable.get_name(), variable.get_differential_count() + 1));
    if(differential_pair == parent->variable_map.end() || differential_pair->second.undefined()) return false;
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
  //TODO: PPでprevは挿入しない
  // unknown_asks.insert(discrete_causes.begin(), discrete_causes.end());
  // std::cout << unknown_asks << std::endl;
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

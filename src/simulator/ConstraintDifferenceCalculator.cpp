#include "ConstraintDifferenceCalculator.h"
#include "VariableFinder.h"

using namespace std;

namespace hydla
{
namespace simulator
{

void ConstraintDifferenceCalculator::calculate_difference_constraints(const simulation_job_sptr_t todo, const boost::shared_ptr<RelationGraph> relation_graph){
  difference_constraints_.clear();
}

void ConstraintDifferenceCalculator::add_difference_constraints(const constraint_t constraint, const boost::shared_ptr<RelationGraph> relation_graph){
}

ConstraintStore ConstraintDifferenceCalculator::get_difference_constraints(){
  return difference_constraints_;
}

bool ConstraintDifferenceCalculator::is_continuous(const simulation_job_sptr_t todo, const symbolic_expression::node_sptr guard)
{
  return true;
}

void ConstraintDifferenceCalculator::collect_ask( const boost::shared_ptr<AskRelationGraph> ask_relation_graph,
    const ask_set_t &positive_asks,
    const ask_set_t &negative_asks,
    ask_set_t &unknown_asks){
/* TODO: Ç±ÇÍÇ™âΩÇ»ÇÃÇ©çlÇ¶ÇÈ
  for(auto ask : discrete_causes){
    if(!positive_asks.count(ask.first) && !negative_asks.count(ask.first)) unknown_asks.insert(ask.first);
  }
*/

}

void ConstraintDifferenceCalculator::set_symmetric_difference(
    const ConstraintStore& parent_constraints,
    const ConstraintStore& current_constraints,
    ConstraintStore& result ){
}


}
}

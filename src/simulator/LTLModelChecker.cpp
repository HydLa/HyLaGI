#include "LTLModelChecker.h"
#include "Automaton.h"
#include "Timer.h"
#include "SymbolicTrajPrinter.h"
#include "PhaseSimulator.h"
#include "ValueModifier.h"
#include "SignalHandler.h"
#include "TimeOutError.h"
#include "Logger.h"
#include "Utility.h"
#include <limits.h>
#include <string>
#include "Backend.h"
#include "ValueModifier.h"
#include <stdio.h>

using namespace std;

namespace hydla {
namespace simulator {

using namespace std;
using namespace symbolic_expression;

LTLModelChecker::LTLModelChecker(Opts &opts):Simulator(opts), printer(backend){}

LTLModelChecker::~LTLModelChecker(){}

phase_result_sptr_t LTLModelChecker::simulate()
{
  std::string error_str = "";
  make_initial_todo();

  try
    {
      consistency_checker.reset(new ConsistencyChecker(backend));

      //Property Automaton initialize
      int id = 0;
      //[True loop]:For making Hybrid Automaton test : if we detect a acceptance cycle, the cycle is HA
      // PropertyNode *property_init = new PropertyNode(id++,ACCEPTANCE_CYCLE);
      // node_sptr true_node = node_sptr(new True());
      // property_init->add_next_edge(property_init,true_node);

      //[bouncing ball 1]:Checking [](y<10) don't over first height
      // PropertyNode *property_init = new PropertyNode(id++,NOMAL);
      // PropertyNode *node1 = new PropertyNode(id++,ACCEPTANCE_CYCLE);
      // node_sptr true_node = node_sptr(new True());
      // node_sptr y_geq_15 = node_sptr(new GreaterEqual(node_sptr(new symbolic_expression::Variable("y")),node_sptr(new Number("15"))));
      // property_init->add_next_edge(property_init,true_node);
      // property_init->add_next_edge(node1,y_geq_15);
      // node1->add_next_edge(node1,true_node);

      //[bouncing ball 2]:Checking []<>(y=0) & []<>(y!=0) ball is bouncing repeatedly
      /* TODO: implement
      PropertyNode *property_init = new PropertyNode(id++,NOMAL);
      PropertyNode *node1 = new PropertyNode(id++,ACCEPTANCE_CYCLE);
      PropertyNode *node2 = new PropertyNode(id++,ACCEPTANCE_CYCLE);
      node_sptr true_node = node_sptr(new True());
      node_sptr y_eq_0 = node_sptr(new  Equal(node_sptr(new symbolic_expression::Variable("y")),node_sptr(new Number("0"))));
      node_sptr y_neq_0 = node_sptr(new Not(node_sptr(new Equal(node_sptr(new symbolic_expression::Variable("y")),node_sptr(new Number("0"))))));
      property_init->add_next_edge(property_init,true_node);
      property_init->add_next_edge(node1,y_eq_0);
      property_init->add_next_edge(node2,y_neq_0);
      node1->add_next_edge(node1,y_eq_0);
      node2->add_next_edge(node2,y_neq_0);
*/
      //[bouncing ball 3]:Checking [](y'<10) don't over first speed
      // PropertyNode *property_init = new PropertyNode(id++,NOMAL);
      // PropertyNode *node1 = new PropertyNode(id++,ACCEPTANCE_CYCLE);
      // node_sptr true_node = node_sptr(new True());
      // node_sptr y_geq_15 = node_sptr(new GreaterEqual(node_sptr(new symbolic_expression::Variable("y")),node_sptr(new Number("15"))));
      // property_init->add_next_edge(true_node,property_init);
      // property_init->add_next_edge(y_geq_15,node1);
      // node1->add_next_edge(true_node,node1);

      //[hot air balloon]:Checking flying [](y>0) : don't work
      // PropertyNode *property_init = new PropertyNode(id++,NOMAL);
      // PropertyNode *node1 = new PropertyNode(id++,ACCEPTANCE_CYCLE);
      // node_sptr true_node = node_sptr(new True());
      // node_sptr y_ng_0 = node_sptr(new Not(node_sptr(new Greater(node_sptr(new symbolic_expression::Variable("y")),node_sptr(new Number("0"))))));
      // property_init->add_next_edge(true_node,property_init);
      // property_init->add_next_edge(y_ng_0,node1);
      // node1->add_next_edge(true_node,node1);

      //[water tank 1]:Checking whether the tank is filled up with water <>(y=12)
      // PropertyNode *property_init = new PropertyNode(id++,ACCEPTANCE_CYCLE);
      // node_sptr y_neq_12 = node_sptr(new Not(node_sptr(new Equal(node_sptr(new symbolic_expression::Variable("y")),node_sptr(new Number("12"))))));
      // property_init->add_next_edge(y_neq_12,property_init);

      //[water tank 2]:Checking wheter water level reach certain value repeatedly []<>(y=6) : need 20 phase
      // PropertyNode *property_init = new PropertyNode(id++,NOMAL);
      // PropertyNode *node1 = new PropertyNode(id++,ACCEPTANCE_CYCLE);
      // node_sptr true_node = node_sptr(new True());
      // node_sptr y_neq_6 = node_sptr(new Not(node_sptr(new Equal(node_sptr(new symbolic_expression::Variable("y")),node_sptr(new Number("6"))))));
      // property_init->add_next_edge(true_node,property_init);
      // property_init->add_next_edge(y_neq_6,node1);
      // node1->add_next_edge(y_neq_6,node1);

      //[Artificial Example]:For testing wheter I can deal with the inclusion of phase correctly
      // PropertyNode *property_init = new PropertyNode(id++,NOMAL);
      // PropertyNode *node1 = new PropertyNode(id++,ACCEPTANCE_CYCLE);
      // node_sptr true_node = node_sptr(new True());
      // node_sptr y_geq_3 = node_sptr(new GreaterEqual(node_sptr(new symbolic_expression::Variable("y")),node_sptr(new Number("3"))));
      // property_init->add_next_edge(true_node,property_init);
      // property_init->add_next_edge(y_geq_3,node1);
      // node1->add_next_edge(true_node,node1);

      //init LTL search
      /* TODO: implement
      LTLNode *LTL_init = new LTLNode("init",result_root_,property_init);
      ltl_node_list_t ltl_start;
      ltl_start.push_back(LTL_init);

      LTLsearch(result_root_,ltl_start,LTL_init);
      cout << "~~~~~~~~~~ property automaton ~~~~~~~~~" << endl;
      property_init->dump();
      cout << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << endl;
      cout << "========== result ltl search ==========" << endl;
      (LTL_init->next_edge.begin())->first->dump();
      cout << "=======================================" << endl;
      */
    }
  catch(const std::runtime_error &se)
    {
      error_str += "error ";
      error_str += ": ";
      error_str += se.what();
      error_str += "\n";
      HYDLA_LOGGER_DEBUG_VAR(error_str);
      std::cout << error_str;
    }


  if(signal_handler::interrupted){
    // // TODO: 各未実行フェーズを適切に処理
    // while(!todo_stack_->empty())
    // {
    //   simulation_job_sptr_t todo(todo_stack_->pop_todo());
    //   todo->parent->simulation_state = INTERRUPTED;
    //   // TODO: restart simulation from each interrupted phase
    // }
  }

  HYDLA_LOGGER_DEBUG("%% simulation ended");
  return result_root_;
}

void LTLModelChecker::LTLsearch(phase_result_sptr_t current,ltl_node_list_t ltl_current,LTLNode* result_init)
{
  if(signal_handler::interrupted)
    {
      current->simulation_state = INTERRUPTED;
      return;
    }
  phase_simulator_->apply_diff(*current);
  while(!current->todo_list.empty())
    {
      phase_result_sptr_t todo = current->todo_list.front();
      current->todo_list.pop_front();
      profile_vector_->insert(todo);
      if(todo->simulation_state == NOT_SIMULATED){
        process_one_todo(todo);
        /* TODO: assertion違反が検出された場合の対応 */
        ltl_current = transition(ltl_current,todo,consistency_checker);
      }
      if(opts_->dump_in_progress){
        printer.output_one_phase(todo);
      }
      //debug print
      // result_init->dump();
      LTLsearch(todo,ltl_current,result_init);
    }
  phase_simulator_->revert_diff(*current);
}

ltl_node_list_t LTLModelChecker::transition(ltl_node_list_t current,phase_result_sptr_t phase,consistency_checker_t consistency_checker){
  /* TODO: implement
  ltl_node_list_t next_search;
  for(ltl_node_list_t::iterator current_LTL_node = current.begin();current_LTL_node != current.end();current_LTL_node++){
    for(automaton_edge_list_t::iterator property_edge = (*current_LTL_node)->property->next_edge.begin();property_edge != (*current_LTL_node)->property->next_edge.end();property_edge++){
      //phase と property_edge->second(edgeのguard条件) で成否判定する
      if(check_edge_guard(phase,property_edge->second,consistency_checker)){
        LTLNode* nextNode = new LTLNode(phase,(PropertyNode*)(property_edge->first));
        //acceptance cycleの探索
        LTLNode* loop_node = detect_acceptance_cycle(nextNode,*current_LTL_node);
        if(loop_node!=NULL){
          (*current_LTL_node)->add_next_edge(loop_node);
          (*current_LTL_node)->set_color_to_trace_path("red");
          next_search.clear();
          return next_search;
        }
        //通常ループの探索
        loop_node = detect_loop_in_path(nextNode,(*current_LTL_node)->trace_path);
        //ループの場合
        if(loop_node!=NULL){
          (*current_LTL_node)->add_next_edge(loop_node);
        }
        //ループでない場合
        if(loop_node == NULL){
          (*current_LTL_node)->add_next_edge(nextNode);
          next_search.push_back(nextNode);
          //acceptance stateの場合
          if(nextNode->acceptanceState()){
            nextNode->set_color_to_trace_path("red");
            next_search.clear();
            return next_search;
          }
          //acceptance cycleの受理状態の場合
          if(nextNode->acceptanceCycle()){
            nextNode->acceptance_pathes.push_back(nextNode->trace_path);
          }
        }
      }
    }
  }
  return next_search;
  */
}

bool LTLModelChecker::check_including(LTLNode* larger,LTLNode* smaller){
  bool include_ret;
  //property_automaton
  string larger_property = larger->property->name;
  string smaller_property = smaller->property->name;
  if(larger_property != smaller_property){
    // cout << "different property automaton :\n\t \"" << larger->id << "\" : \"" << smaller->id << "\"" << endl;
    return false;
  }
  //phase typeの比較
  if(larger->phase->phase_type != smaller->phase->phase_type){
    // cout << "different phase type :\n\t \"" << larger->id << "\" : \"" << smaller->id << "\"" << endl;
    return false;
  }
  //phase の変数表の大きさの比較
  if(larger->phase->variable_map.size() != smaller->phase->variable_map.size()){
    // cout << "different size of variable map :\n\t \"" << larger->id << "\" : \"" << smaller->id << "\"" << endl;
    return false;
  }

  ConstraintStore larger_cons = larger->phase->get_parameter_constraint();
  ConstraintStore smaller_cons = smaller->phase->get_parameter_constraint();
  //compareing set of variables
  backend->call("checkInclude", true, 6, "vlnmvtcsnvlnmvtcsn", "b",
                &(larger->phase->current_time), &(larger->phase->variable_map), &larger_cons,
                &(smaller->phase->current_time), &(smaller->phase->variable_map), &smaller_cons, &include_ret);
  if(include_ret){
    // cout << "\n\"" << larger->id << "\" includes \"" << smaller->id << "\"\n" << endl;
  }
  else{
    // cout << "not included :\n\t \"" << larger->id << "\" : \"" << smaller->id << "\"" << endl;
  }
  return include_ret;
}

LTLNode* LTLModelChecker::detect_acceptance_cycle(LTLNode* new_node,LTLNode* parent_node){
  LTLNode* ret = NULL;
  for(path_list_t::iterator acceptance_path = parent_node->acceptance_pathes.begin();acceptance_path != parent_node->acceptance_pathes.end();acceptance_path++){
    ret = detect_loop_in_path(new_node, *acceptance_path);
    if(ret!=NULL){
      return ret;
    }
  }
  return ret;
}

LTLNode* LTLModelChecker::detect_loop_in_path(LTLNode* new_node, automaton_node_list_t path){
  LTLNode* ret = NULL;
  for(automaton_node_list_t::iterator it = path.begin();it != path.end();it++){
    if(check_including((LTLNode*)*it,new_node)){
      ret = (LTLNode*)*it;
      return ret;
    }
  }
  return ret;
}

bool LTLModelChecker::check_edge_guard(phase_result_sptr_t phase,node_sptr guard,consistency_checker_t consistency_checker){
  bool ret = false;
  // cout << "come here 1" << endl;
  if(guard->get_node_type_name() == "True"){
    return true;
  }
  // cout << "come here 2" << endl;
  // ConstraintStore par_cons = phase->get_parameter_constraint();
  // cout << "come here 3" << endl;
  // backend->call("resetConstraintForParameter", true, 1, "mp", "", &par_cons);
  // cout << "come here 4" << endl;
  CheckConsistencyResult cc_result;
  // cout << "come here 5" << endl;
  HYDLA_LOGGER_DEBUG("entailed check start ===========================");
  HYDLA_LOGGER_DEBUG_VAR(get_infix_string(guard));
  // variable_map_t related_vm = get_related_vm(guard, phase->variable_map);

  switch(consistency_checker->check_entailment(phase->variable_map, cc_result, guard, phase->phase_type, phase->profile)){
  case ENTAILED:
    ret = true;
    break;
  case BRANCH_PAR:
  case CONFLICTING:
  case BRANCH_VAR:
    break;
  }
  HYDLA_LOGGER_DEBUG(ret);
  HYDLA_LOGGER_DEBUG("entailed check finish ===========================");
  return ret;
}

}//namespace hydla
}//namespace simulator

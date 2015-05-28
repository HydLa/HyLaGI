#include "LTLModelChecker.h"
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

LTLModelChecker::LTLModelChecker(Opts &opts):Simulator(opts), printer(){}

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
      //[True loop]
      PropertyNode *property_init = new PropertyNode(id++,ACCEPTANCE_CYCLE);
      node_sptr true_node = node_sptr(new True());
      property_init->addLink(true_node,property_init);

      //[bouncing ball 1]
      // PropertyNode *property_init = new PropertyNode(id++,NOMAL);
      // PropertyNode *node1 = new PropertyNode(id++,ACCEPTANCE_CYCLE);
      // node_sptr true_node = node_sptr(new True());
      // node_sptr y_geq_15 = node_sptr(new GreaterEqual(node_sptr(new symbolic_expression::Variable("y")),node_sptr(new Number("15"))));
      // property_init->addLink(true_node,property_init);
      // property_init->addLink(y_geq_15,node1);
      // node1->addLink(true_node,node1);

      //[bouncing ball 2]
      // PropertyNode *property_init = new PropertyNode(id++,NOMAL);
      // PropertyNode *node1 = new PropertyNode(id++,ACCEPTANCE_CYCLE);
      // PropertyNode *node2 = new PropertyNode(id++,ACCEPTANCE_CYCLE);
      // node_sptr true_node = node_sptr(new True());
      // node_sptr y_eq_0 = node_sptr(new  Equal(node_sptr(new symbolic_expression::Variable("y")),node_sptr(new Number("0"))));
      // node_sptr y_neq_0 = node_sptr(new Not(node_sptr(new Equal(node_sptr(new symbolic_expression::Variable("y")),node_sptr(new Number("0"))))));
      // property_init->addLink(true_node,property_init);
      // property_init->addLink(y_eq_0,node1);
      // property_init->addLink(y_neq_0,node2);
      // node1->addLink(y_eq_0,node1);
      // node2->addLink(y_neq_0,node2);

      //[hot air balloon]
      // PropertyNode *property_init = new PropertyNode(id++,NOMAL);
      // PropertyNode *node1 = new PropertyNode(id++,ACCEPTANCE_CYCLE);
      // node_sptr true_node = node_sptr(new True());
      // node_sptr y_ng_0 = node_sptr(new Not(node_sptr(new Greater(node_sptr(new symbolic_expression::Variable("y")),node_sptr(new Number("0"))))));
      // property_init->addLink(true_node,property_init);
      // property_init->addLink(y_ng_0,node1);
      // node1->addLink(true_node,node1);

      //[water tank 1]
      // PropertyNode *property_init = new PropertyNode(id++,ACCEPTANCE_CYCLE);
      // node_sptr y_neq_12 = node_sptr(new Not(node_sptr(new Equal(node_sptr(new symbolic_expression::Variable("y")),node_sptr(new Number("12"))))));
      // property_init->addLink(y_neq_12,property_init);

      //[water tank 2]
      // PropertyNode *property_init = new PropertyNode(id++,NOMAL);
      // Propertynode *node1 = new PropertyNode(id++,ACCEPTANCE_CYCLE);
      // node_sptr true_node = node_sptr(new True());
      // node_sptr y_neq_6 = node_sptr(new Not(node_sptr(new Equal(node_sptr(new symbolic_expression::Variable("y")),node_sptr(new Number("6"))))));
      // property_init->addLink(true_node,property_init);
      // property_init->addLink(y_neq_6,node1);
      // node1->addLink(y_neq_6,node1);

      //[Artificial Example]
      // PropertyNode *property_init = new PropertyNode(id++,NOMAL);
      // PropertyNode *node1 = new PropertyNode(id++,ACCEPTANCE_CYCLE);
      // node_sptr true_node = node_sptr(new True());
      // node_sptr y_geq_3 = node_sptr(new GreaterEqual(node_sptr(new symbolic_expression::Variable("y")),node_sptr(new Number("3"))));
      // property_init->addLink(true_node,property_init);
      // property_init->addLink(y_geq_3,node1);
      // node1->addLink(true_node,node1);

      //init LTL search
      PropertyNode *PropertyZero = new PropertyNode(0,ZERO);
      LTLNode *LTLZero = new LTLNode(result_root_,PropertyZero);
      ltl_node_list_t ltl_start;

      // property_init->dot();
      cout << "~~~~~~~~~~ property automaton ~~~~~~~~~" << endl;
      printer.output_property_automaton(property_init);
      cout << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << endl;
      LTLsearch(result_root_,ltl_start,LTLZero,property_init);
      // LTLZero->dot();
      cout << "========== result ltl search ==========" << endl;
      printer.output_ltl_node(LTLZero);
      cout << "=======================================" << endl;
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

void LTLModelChecker::LTLsearch(phase_result_sptr_t current,ltl_node_list_t ltl_current,LTLNode* result_init,PropertyNode* property_init)
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
        if(ltl_current.empty()){
          // cout << "come ltl search : 1" << endl;
          //初期化
          LTLNode *FirstState = new LTLNode(todo,property_init);
          result_init->addLink(FirstState);
          ltl_current.push_back(FirstState);
        }else{
          // cout << "come ltl search : n" << endl;
          ltl_current = transition(ltl_current,todo,consistency_checker);
        }
      }
      if(opts_->dump_in_progress){
        printer.output_one_phase(todo);
      }
      //確認
      // cout << "test::::::::::::::::::::" << endl;
      // printer.output_ltl_node(result_init);
      // cout << "test::::::::::::::::::::" << endl;

      LTLsearch(todo,ltl_current,result_init,property_init);
    }
  phase_simulator_->revert_diff(*current);
}

ltl_node_list_t LTLModelChecker::transition(ltl_node_list_t current,phase_result_sptr_t phase,consistency_checker_t consistency_checker){
  ltl_node_list_t next_search;
  for(ltl_node_list_t::iterator current_LTL_node = current.begin();current_LTL_node != current.end();current_LTL_node++){
    for(Property_link_t::iterator property_link = (*current_LTL_node)->property->link.begin();property_link != (*current_LTL_node)->property->link.end();property_link++){
      //phase と propertylink->first で成否判定する
      if(check_edge_guard(phase,property_link->first,consistency_checker)){
        LTLNode* nextNode = new LTLNode(phase,property_link->second);
        //acceptance cycleの探索
        LTLNode* loop_node = detect_acceptance_cycle(nextNode,*current_LTL_node);
        if(loop_node!=NULL){
          (*current_LTL_node)->addLink(loop_node);
          (*current_LTL_node)->setRed();
          next_search.clear();
          return next_search;
        }
        //通常ループの探索
        loop_node = detect_loop_in_pass(nextNode,(*current_LTL_node)->pass);
        //ループの場合
        if(loop_node!=NULL){
          (*current_LTL_node)->addLink(loop_node);
        }
        //ループでない場合
        if(loop_node == NULL){
          (*current_LTL_node)->addLink(nextNode);
          next_search.push_back(nextNode);
          //acceptance stateの場合
          if(nextNode->acceptanceState()){
            nextNode->setRed();
            next_search.clear();
            return next_search;
          }
          //acceptance cycleの受理状態の場合
          if(nextNode->acceptanceCycle()){
            nextNode->acceptance_passes.push_back(nextNode->pass);
          }
        }
      }
    }
  }
  return next_search;
}

bool LTLModelChecker::check_including(LTLNode* larger,LTLNode* smaller){
  bool include_ret;
  //property_automaton
  int larger_property = larger->property->id;
  int smaller_property = smaller->property->id;
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

  //compareing set of variables
  backend->call("checkInclude", 6, "vlnmvtmpvlnmvtmp", "b",
                &(larger->phase->current_time), &(larger->phase->variable_map), &(larger->phase->parameter_map),
                &(smaller->phase->current_time), &(smaller->phase->variable_map), &(smaller->phase->parameter_map), &include_ret);
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
  for(pass_list_t::iterator acceptance_pass = parent_node->acceptance_passes.begin();acceptance_pass != parent_node->acceptance_passes.end();acceptance_pass++){
    ret = detect_loop_in_pass(new_node, *acceptance_pass);
    if(ret!=NULL){
      return ret;
    }
  }
  return ret;
}

LTLNode* LTLModelChecker::detect_loop_in_pass(LTLNode* new_node, ltl_node_list_t pass){
  LTLNode* ret = NULL;
  for(ltl_node_list_t::iterator it = pass.begin();it != pass.end();it++){
    if(check_including(*it,new_node)){
      ret = *it;
      return ret;
    }
  }
  return ret;
}

bool LTLModelChecker::check_edge_guard(phase_result_sptr_t phase,node_sptr guard,consistency_checker_t consistency_checker){
  bool ret = false;
  if(guard->get_node_type_name() == "True"){
    return true;
  }
  backend->call("resetConstraintForParameter", 1, "mp", "", &phase->parameter_map);
  CheckConsistencyResult cc_result;
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

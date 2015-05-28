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

      property_init->dot();
      cout << "~~~~~~~~~~ property automaton ~~~~~~~~~" << endl;
      printer.output_property_automaton(property_init);
      cout << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << endl;
      PropertyNode *PropertyZero = new PropertyNode(0,ZERO);
      LTLNode *LTLZero = new LTLNode(result_root_,PropertyZero);
      ltl_node_list_t ltl_start;
      LTLsearch(result_root_,ltl_start,LTLZero,property_init);
      //LTLZero->dot();
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
  // for(auto para : phase->parameter_map) cout << "true ? : " << para.first.get_name() << ":" << para.second << endl;
  for(ltl_node_list_t::iterator current_LTL_node = current.begin();current_LTL_node != current.end();current_LTL_node++){
    for(Property_link_t::iterator property_link = (*current_LTL_node)->property->link.begin();property_link != (*current_LTL_node)->property->link.end();property_link++){
      //phase と propertylink->first で成否判定する
      if(check_edge_guard(phase,property_link->first,consistency_checker)){
        LTLNode* nextNode = new LTLNode(phase,property_link->second);
        //acceptanceCycleの探索
        LTLNode* loop_node = nextNode->detectAcceptanceCycle(*current_LTL_node,backend);
        if(loop_node!=NULL){
          (*current_LTL_node)->addLink(loop_node);
          (*current_LTL_node)->setRed();
          next_search.clear();
          return next_search;
        }
        //通常ループの探索
        loop_node = nextNode->detectLoop(*current_LTL_node,backend);
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
  // bool LTLNode::will_include(LTLNode* check,backend_sptr_t backend){
  //   // A->will_include(B) <=> A ) B
  //   bool ret;
  //   //property_automaton
  //   int old_property = property->id;
  //   int now_property = check->property->id;
  //   if(old_property != now_property){
  //     return false;
  //   }
  //   //phase
  //   // if(compare_phase_result(phase,check->phase)){
  //   //   ret = check_subset(phase,check->phase,backend);
  //   // }
  //   // cout << phase->phase_type << endl;
  //   //phase typeの比較
  //   if(phase->phase_type != check->phase->phase_type){return false;}
  //   //phase の変数表の大きさの比較
  //   if(phase->variable_map.size() != check->phase->variable_map.size()){return false;}

  //   // cout << "come here0" << endl;
  //   //変数表の時刻を戻す
  //   value_t time_old, time_now;
  //   variable_map_t old_vm, now_vm;
  //   // variable_map_t shifted_vm_now, shifted_vm_old;
  //   ValueModifier modifier(*backend);
  //   time_old = phase->current_time;
  //   time_now = check->phase->current_time;
  //   old_vm = modifier.substitute_time(time_old,phase->variable_map);
  //   now_vm = modifier.substitute_time(time_now,check->phase->variable_map);

  //   // tmp_old = node_sptr(new Number("-1"));
  //   // tmp_old = node_sptr(new Times(tmp_old, phase->current_time.get_node()));
  //   // tmp_new = node_sptr(new Number("-1"));
  //   // tmp_new = node_sptr(new Times(tmp_new, phase->current_time.get_node()));
  //   // shifted_vm_old = modifier.apply_function("exprTimeshift", tmp_old, old_vm);
  //   // shifted_vm_new = modifier.apply_function("exprTimeshift", tmp_new, new_vm);
  //   // cout << "come here1" << endl;

  //   // backend->call("resetConstraintForVariable", 0, "", "");
  //   // backend->call("addConstraint", 1, "mv0t", "", &shifted_vm);

  //   // auto var_old = phase->variable_map.begin();
  //   // auto var_new = check->phase->variable_map.begin();

  //   bool testret;
  //   cout << "testing" << endl;
  //   backend->call("aho", 2, "mvtmp", "", &old_vm, &(phase->parameter_map));
  //   cout << "end" << endl;

  //   // comparing variable
  //   auto var_old = old_vm.begin();
  //   auto var_now = now_vm.begin();
  //   cout << "compare variables" << endl;
  //   int parametercount = 0;
  //   while(var_old != old_vm.end() && var_now != now_vm.end()){
  //     ret = false;
  //     // cout<<"o" << (var_old->first) << endl;
  //     // cout<<"n" << (var_now->first) << endl;
  //     //もし変数名が違う場合
  //     if(var_old->first != var_now->first){
  //       cout << "not include : name " << endl;
  //       return false;
  //     }
  //     value_t tmp_variable_now = var_now->second.get_unique_value();
  //     value_t tmp_variable_old = var_old->second.get_unique_value();
  //     // cout << "finding parameter : now => ";
  //     // search_parameter(tmp_variable_now);
  //     // cout << "finding parameter : old => ";
  //     // search_parameter(tmp_variable_old);

  //     cout << var_old->first << "\t: " << var_old->second << "\t <=> \t" << var_now->second << endl;

  //     if(search_parameter(tmp_variable_old) || search_parameter(tmp_variable_now)){
  //       parametercount++;
  //     }
  //     if(parametercount > 1){
  //       cout << "not include : parameter " << endl;
  //       return false;
  //     }


  //     // for(auto oldpara : phase->parameter_map) cout << "parameter map old : " << oldpara.first.get_name() << ":" << oldpara.second << endl;
  //     // for(auto nowpara : check->phase->parameter_map) cout << "parameter map now : " << nowpara.first.get_name() << ":" << nowpara.second << endl;
  //     backend->call("checkInclude", 4, "vlnvlnmpmp", "b", &tmp_variable_old, &tmp_variable_now, &(phase->parameter_map), &(check->phase->parameter_map), &ret);
  //     cout << "\t:" ;
  //     if(ret) cout << "true" << endl;
  //     else  cout << "false" << endl;
  //     if(!ret){
  //       cout << "not include : value " << endl;
  //       return false;
  //     }
  //     var_old++;
  //     var_now++;
  //   }
  //   // if(var_now != check->phase->variable_map.end()){return false;}
  //   // char in;
  //   // cin >> in;
  //   // if(in == 'y'){ret = true;}
  //   // else {ret = false;}
  //   // int a = (phase->id) % 7;
  //   // int b = ((check->phase->id)) % 7;
  //   // ret = (a==b);
  //   cout << "\"" << id << "\" includes \"" << check->id << "\"" << endl;
  //   return true;
  // }


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

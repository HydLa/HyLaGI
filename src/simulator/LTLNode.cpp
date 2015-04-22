#include "PhaseResult.h"
#include "PhaseSimulator.h"
#include "PropertyNode.h"
#include "../symbolic_expression/Node.h"
#include "LTLNode.h"
#include "ConsistencyChecker.h"
#include "ValueModifier.h"
#include <iostream>
#include <vector>
#include <string>
using namespace std;
using namespace hydla;
using namespace simulator;
using namespace symbolic_expression;

LTLNode::LTLNode(phase_result_sptr_t set_phase,PropertyNode* set_property){
  id = "Property" + to_string(set_property->id) + " Phase" + to_string(set_phase->id);
  phase = set_phase;
  property = set_property;
  parent = NULL;
  red = 0;
  write = 0;
}

// LTLNode::~LTLNode(){
// }

// bool check_edge_guard(phase_result_sptr_t phase,node_sptr guard,backend_sptr_t backend,consistency_checker_t consistency_checker){
//   bool ret = false;
//   if(guard->get_node_type_name() == "True"){
//     return true;
//   }
//   backend->call("resetConstraintForParameter", 1, "mp", "", &phase->parameter_map);
//   CheckConsistencyResult cc_result;
//   HYDLA_LOGGER_DEBUG("entailed check start ===========================");
//   HYDLA_LOGGER_DEBUG_VAR(get_infix_string(guard));
//   // variable_map_t related_vm = get_related_vm(guard, phase->variable_map);
//   switch(consistency_checker->check_entailment(phase->variable_map, cc_result, guard, phase->phase_type, phase->profile)){
//     case ENTAILED:
//       ret = true;
//       break;
//     case BRANCH_PAR:
//     case CONFLICTING:
//     case BRANCH_VAR:
//       break;
//   }
//   HYDLA_LOGGER_DEBUG(ret);
//   HYDLA_LOGGER_DEBUG("entailed check finish ===========================");
//   return ret;
// }

bool LTLNode::will_include(LTLNode* check,backend_sptr_t backend){
  // A->will_include(B) <=> A ) B
  bool ret;
  //property_automaton
  int old_property = property->id;
  int now_property = check->property->id;
  if(old_property != now_property){
    return false;
  }
  //phase
  // if(compare_phase_result(phase,check->phase)){
  //   ret = check_subset(phase,check->phase,backend);
  // }
  // cout << phase->phase_type << endl;
  //phase typeの比較
  if(phase->phase_type != check->phase->phase_type){return false;}
  //phase の変数表の大きさの比較
  if(phase->variable_map.size() != check->phase->variable_map.size()){return false;}

  // cout << "come here0" << endl;
  //変数表の時刻を戻す
  value_t time_old, time_now;
  variable_map_t old_vm, now_vm;
  // variable_map_t shifted_vm_now, shifted_vm_old;
  ValueModifier modifier(*backend);
  time_old = phase->current_time;
  time_now = check->phase->current_time;
  old_vm = modifier.substitute_time(time_old,phase->variable_map);
  now_vm = modifier.substitute_time(time_now,check->phase->variable_map);

  // tmp_old = node_sptr(new Number("-1"));
  // tmp_old = node_sptr(new Times(tmp_old, phase->current_time.get_node()));
  // tmp_new = node_sptr(new Number("-1"));
  // tmp_new = node_sptr(new Times(tmp_new, phase->current_time.get_node()));
  // shifted_vm_old = modifier.apply_function("exprTimeshift", tmp_old, old_vm);
  // shifted_vm_new = modifier.apply_function("exprTimeshift", tmp_new, new_vm);
  // cout << "come here1" << endl;

  // backend->call("resetConstraintForVariable", 0, "", "");
  // backend->call("addConstraint", 1, "mv0t", "", &shifted_vm);

  // auto var_old = phase->variable_map.begin();
  // auto var_new = check->phase->variable_map.begin();

  auto var_old = old_vm.begin();
  auto var_now = now_vm.begin();
  cout << "compare variables" << endl;
  int parametercount = 0;
  while(var_old != old_vm.end() && var_now != now_vm.end()){
    ret = false;
    // cout<<"o" << (var_old->first) << endl;
    // cout<<"n" << (var_now->first) << endl;
    //もし変数名が違う場合
    if(var_old->first != var_now->first){
      cout << "not include : name " << endl;
      return false;
    }
    value_t tmp_variable_now = var_now->second.get_unique_value();
    value_t tmp_variable_old = var_old->second.get_unique_value();
    // cout << "finding parameter : now => ";
    // search_parameter(tmp_variable_now);
    // cout << "finding parameter : old => ";
    // search_parameter(tmp_variable_old);

    cout << var_old->first << "\t: " << var_old->second << "\t <=> \t" << var_now->second << endl;

    if(search_parameter(tmp_variable_old) || search_parameter(tmp_variable_now)){
      parametercount++;
    }
    if(parametercount > 1){
      cout << "not include : parameter " << endl;
      return false;
    }


    // for(auto oldpara : phase->parameter_map) cout << "parameter map old : " << oldpara.first.get_name() << ":" << oldpara.second << endl;
    // for(auto nowpara : check->phase->parameter_map) cout << "parameter map now : " << nowpara.first.get_name() << ":" << nowpara.second << endl;

    backend->call("checkInclude", 4, "vlnvlnmpmp", "b", &tmp_variable_old, &tmp_variable_now, &(phase->parameter_map), &(check->phase->parameter_map), &ret);
    cout << "\t:" ;
    if(ret) cout << "true" << endl;
    else  cout << "false" << endl;
    if(!ret){
      cout << "not include : value " << endl;
      return false;
    }
    var_old++;
    var_now++;
  }
  // if(var_now != check->phase->variable_map.end()){return false;}
  // char in;
  // cin >> in;
  // if(in == 'y'){ret = true;}
  // else {ret = false;}
  // int a = (phase->id) % 7;
  // int b = ((check->phase->id)) % 7;
  // ret = (a==b);
  cout << "\"" << id << "\" includes \"" << check->id << "\"" << endl;
  return true;
}
/*
ltl_node_list_t transition(ltl_node_list_t current,phase_result_sptr_t phase,backend_sptr_t backend,consistency_checker_t consistency_checker){
  ltl_node_list_t next_search;
  for(ltl_node_list_t::iterator current_LTL_node = current.begin();current_LTL_node != current.end();current_LTL_node++){
    for(Property_link_t::iterator property_link = (*current_LTL_node)->property->link.begin();property_link != (*current_LTL_node)->property->link.end();property_link++){
      //phase と propertylink->first で成否判定する
      if(check_edge_guard(phase,property_link->first,backend,consistency_checker)){
        LTLNode* nextNode = now LTLNode(phase,property_link->second);
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
*/

LTLNode* LTLNode::detectAcceptanceCycle(LTLNode* parent_node,backend_sptr_t backend){
  LTLNode* ret = NULL;
  for(pass_list_t::iterator acceptance_pass = parent_node->acceptance_passes.begin();acceptance_pass != parent_node->acceptance_passes.end();acceptance_pass++){
    for(ltl_node_list_t::iterator it = acceptance_pass->begin();it != acceptance_pass->end();it++){
      if((*it)->will_include(this,backend)){
        ret = *it;
      }
    }
  }
  return ret;
}

LTLNode* LTLNode::detectLoop(LTLNode* parent_node,backend_sptr_t backend){
  LTLNode* ret = NULL;
  for(ltl_node_list_t::iterator it = parent_node->pass.begin();it != parent_node->pass.end();it++){
    if((*it)->will_include(this,backend)){
      ret = *it;
    }
  }
  return ret;
}

bool LTLNode::acceptanceState(){
  return (property->type == ACCEPTANCE_STATE);
}

bool LTLNode::acceptanceCycle(){
  return (property->type == ACCEPTANCE_CYCLE);
}

bool LTLNode::search_parameter(value_t var){
  // cout << var.get_node()->get_string();
  if(var.get_node()->get_string().find("Parameter[",0) != string::npos){
    // cout << " : contain" << endl;
    return true;
  }
  // cout << " : not contain" << endl;
  return false;
}

// stack< pair<bool,int> > d
// void search(){
//   //現在の状態の保持
//   int s = now_state;
//   int toggle = search_mode;
//   //次の状態の作成全ての遷移エッジについて考える
//   iterator it = transition[s].iterator();
//   //一つずつ見ていく
//   while(it.end){
//     //遷移エッジから新しい状態をつくる
//     int sd = ;
//     //二段階目のループか比べる(新しい状態の検査)
//     if(toggle){
//     }
//     //loopなど(新しい状態の検査)
//     if(! instatespace()) {
//       addstatespace(sd+toggle);
//       //stack利用
//       search();
//     }
//     //一段階目のループ検査(現在の状態)
//     if(infinalstate(s) !toggle()){
//       //一段階目
//       toggle = true;
//       seed = s;
//       search();
//     }
//   }
// }

// phase_result_sptr_t* simulate(phase_result_sptr_t* phase){
//   phase_result_sptr_t* ret = now phase_result_sptr_t(phase->value + 1);
//   phase->set_next(ret);
//   return ret;
// }

void LTLNode::addLink(LTLNode* child){
  link.push_back(child);
  if(!(child->parent != NULL)){
    ltl_node_list_t tmp_pass = pass;
    tmp_pass.push_back(child);
    child->parent = this;
    child->pass = tmp_pass;
  }
}

void LTLNode::trace(){
  cout << " -> " << "\"" << id << "\"";
  if(parent != NULL){
    parent->trace();
  }
}

void LTLNode::dump(){
  if(write == 0){
    write++;
    cout << "\"" << id << "\"" << " ";
    if(property->type != NOMAL){
      if(red>0){
        cout << "[peripheries=2 color=red];" << endl;
      }else{
        cout << "[peripheries=2];" << endl;
      }
    }else{
      if(red>0){
        cout << "[color=red];" << endl;
      }else{
        cout << ";" << endl;
      }
    }
    for(ltl_node_list_t::iterator it = link.begin();it != link.end();it++){
      if(id == (*it)->id){
        if(red>0 && (*it)->red>0){
          cout << "\"" << id << "\"" << " -> " << "\"" << id << "\"" << "[color=red];" << endl;
        }else{
          cout << "\"" << id << "\"" << " -> " << "\"" << id << "\"" << ";" << endl;
        }
      }else{
        if(red>0 && (*it)->red>0){
          cout << "\"" << id << "\"" << " -> " << "\"" << (*it)->id << "\"" << "[color=red];" << endl;
        }else{
          cout << "\"" << id << "\"" << " -> " << "\"" << (*it)->id << "\"" << ";" << endl;
        }
        (*it)->dump();
      }
    }
  }
}

void LTLNode::dot(){
  if(property->type != ZERO){
    write_reset();
    cout << "digraph g{" << endl;
    cout << "\"init\"[shape=\"point\"];" << endl;
    cout << "\"init\"" << " -> " << "\"" << id << "\"" << ";" << endl;
    dump();
    cout << "}" << endl;
    write_reset();
  }else{
    ltl_node_list_t::iterator it = link.begin();
    (*it)->dot();
  }
}

void LTLNode::write_reset(){
  if(write > 0){
    write = 0;
    for(ltl_node_list_t::iterator it = link.begin();it != link.end();it++){
      (*it)->write_reset();
    }
  }
}

void LTLNode::setRed(){
  for(ltl_node_list_t::iterator it = pass.begin();it != pass.end();it++){
    (*it)->red = 1;
  }
}

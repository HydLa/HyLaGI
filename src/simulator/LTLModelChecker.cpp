#include "LTLModelChecker.h"
#include "Automaton.h"
#include "Timer.h"
#include "SymbolicTrajPrinter.h"
#include "PhaseSimulator.h"
#include "ValueModifier.h"
#include "SignalHandler.h"
#include "VariableFinder.h"
#include "TimeOutError.h"
#include "Logger.h"
#include "Utility.h"
#include <limits.h>
#include <string>
#include "Backend.h"
#include "ValueModifier.h"
#include <stdio.h>
#include "./never_claim/NeverClaim.h"

using namespace std;

#include "../parser/never_claim/y.tab.hpp"
#include <memory>
std::shared_ptr<hydla::simulator::Automaton> nc_parse();

namespace hydla {
namespace simulator {

using namespace std;
using namespace symbolic_expression;
using namespace never_claim;

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
      id_counter = 0;

      // [True loop]
      // : For making Hybrid Automaton test
      // : if we detect a acceptance cycle, the cycle is HA
      // PropertyNode *property_init = new PropertyNode(id++,ACCEPTANCE_CYCLE);
      // node_sptr true_node = node_sptr(new True());
      // property_init->add_edge(property_init,true_node);

      // [bouncing ball 1]
      // : Checking []<>(y=0) & []<>(y!=0)
      // : ball is bouncing repeatedly
      // PropertyNode *property_init = new PropertyNode(id++,NORMAL);
      // PropertyNode *node1 = new PropertyNode(id++,ACCEPTANCE_CYCLE);
      // PropertyNode *node2 = new PropertyNode(id++,ACCEPTANCE_CYCLE);
      // node_sptr true_node = node_sptr(new True());
      // node_sptr y_eq_0 = node_sptr(new  Equal(node_sptr(new symbolic_expression::Variable("y")),node_sptr(new Number("0"))));
      // node_sptr y_neq_0 = node_sptr(new Not(node_sptr(new Equal(node_sptr(new symbolic_expression::Variable("y")),node_sptr(new Number("0"))))));
      // property_init->add_edge(property_init,true_node);
      // property_init->add_edge(node1,y_eq_0);
      // property_init->add_edge(node2,y_neq_0);
      // node1->add_edge(node1,y_eq_0);
      // node2->add_edge(node2,y_neq_0);
      /*
      // [bouncing ball 2]
      // : Checking []<>(y>7)
      // : ball is always eventually y>7
      // : for case devide (find acceptance cycle or not find)
      property_init = new PropertyNode(id++,NORMAL);
      PropertyNode *node1 = new PropertyNode(id++,ACCEPTANCE_CYCLE);
      node_sptr true_node = node_sptr(new True());
      node_sptr y_leq_7 = node_sptr(new LessEqual(node_sptr(new symbolic_expression::Variable("y")),node_sptr(new Number("7"))));
      property_init->add_edge(property_init,true_node);
      property_init->add_edge(node1,y_leq_7);
      node1->add_edge(node1,y_leq_7);
      */

      // [bouncing ball 3]
      // : test <>y!=0
      // PropertyNode *property_init = new PropertyNode(id++,ACCEPTANCE_CYCLE);
      // node_sptr y_eq_0 = node_sptr(new  Equal(node_sptr(new symbolic_expression::Variable("y")),node_sptr(new Number("0"))));
      // property_init->add_edge(property_init,y_eq_0);

      // [bouncing ball 4]
      // : test []!(y<0)
      // PropertyNode *property_init = new PropertyNode(id++,NORMAL);
      // PropertyNode *node1 = new PropertyNode(id++,ACCEPTANCE_STATE);
      // node_sptr true_node = node_sptr(new True());
      // node_sptr y_less_0 = node_sptr(new  Less(node_sptr(new symbolic_expression::Variable("y")),node_sptr(new Number("0"))));
      // property_init->add_edge(property_init,true_node);
      // property_init->add_edge(property_init,y_less_0);
      // node1->add_edge(node1,true_node);

      // [bouncing ball hole 1]
      // : []y!=-1
      // PropertyNode *property_init = new PropertyNode(id++,NORMAL);
      // PropertyNode *node1 = new PropertyNode(id++,ACCEPTANCE_STATE);
      // node_sptr true_node = node_sptr(new True());
      // node_sptr y_eq_0 = node_sptr(new  Equal(node_sptr(new symbolic_expression::Variable("y")),node_sptr(new Number("-1"))));
      // property_init->add_edge(property_init,true_node);
      // property_init->add_edge(node1,y_eq_0);
      // node1->add_edge(node1,true_node);

      // [triangle sawtooth 1]
      // : [](y>=0 && y<=10)
      // PropertyNode *property_init = new PropertyNode(id++,NORMAL);
      // PropertyNode *node1 = new PropertyNode(id++,ACCEPTANCE_STATE);
      // node_sptr true_node = node_sptr(new True());
      // node_sptr y_less_0 = node_sptr(new  Less(node_sptr(new symbolic_expression::Variable("y")),node_sptr(new Number("0"))));
      // node_sptr y_great_10 = node_sptr(new  Greater(node_sptr(new symbolic_expression::Variable("y")),node_sptr(new Number("10"))));
      // node_sptr guard1 = node_sptr(new LogicalOr(y_less_0,y_great_10));
      // property_init->add_edge(property_init,true_node);
      // property_init->add_edge(node1,guard1);
      // node1->add_edge(node1,true_node);

      // [triangle sawtooth 2]
      // : <>[](<>f=11 || []f=-1)
      // PropertyNode *property_init = new PropertyNode(id++,NORMAL);
      // PropertyNode *node1 = new PropertyNode(id++,ACCEPTANCE_CYCLE);
      // PropertyNode *node2 = new PropertyNode(id++,NORMAL);
      // PropertyNode *node3 = new PropertyNode(id++,ACCEPTANCE_CYCLE);
      // PropertyNode *node4 = new PropertyNode(id++,NORMAL);
      // PropertyNode *node5 = new PropertyNode(id++,NORMAL);
      // node_sptr true_node = node_sptr(new True());
      // node_sptr p = node_sptr(new Not(node_sptr(new  Equal(node_sptr(new symbolic_expression::Variable("f")),node_sptr(new Number("11"))))));
      // node_sptr q = node_sptr(new Not(node_sptr(new  Equal(node_sptr(new symbolic_expression::Variable("f")),node_sptr(new Number("-1"))))));
      // node_sptr guard1 = node_sptr(new LogicalAnd(p,q));
      // property_init->add_edge(node5,true_node);
      // property_init->add_edge(node4,p);
      // property_init->add_edge(node1,guard1);
      // node1->add_edge(node4,p);
      // node1->add_edge(node2,p);
      // node1->add_edge(node1,guard1);
      // node2->add_edge(node2,p);
      // node2->add_edge(node3,p);
      // node2->add_edge(node1,guard1);
      // node3->add_edge(node4,p);
      // node3->add_edge(node1,guard1);
      // node4->add_edge(node4,p);
      // node4->add_edge(node1,guard1);
      // node5->add_edge(node5,true_node);
      // node5->add_edge(node3,p);
      // node5->add_edge(node1,guard1);

      // [bouncing auto abstract]
      // :
      // PropertyNode *property_init = new PropertyNode(id++,ACCEPTANCE_CYCLE);
      // // PropertyNode *node1 = PropertyNode(id++,ACCEPTANCE_CYCLE)
      // node_sptr true_node = node_sptr(new True());
      // property_init->add_edge(property_init,true_node);

      // [Artificial Example]
      // : For testing wheter I can deal with the inclusion of phase correctly
      // PropertyNode *property_init = new PropertyNode(id++,NORMAL);
      // PropertyNode *node1 = new PropertyNode(id++,ACCEPTANCE_STATE);
      // node_sptr true_node = node_sptr(new True());
      // node_sptr y_geq_3 = node_sptr(new GreaterEqual(node_sptr(new symbolic_expression::Variable("y")),node_sptr(new Number("3"))));
      // property_init->add_edge(property_init,true_node);
      // property_init->add_edge(node1,y_geq_3);
      // node1->add_edge(node1,true_node);

      // [water tank 1]
      // :<>p
      // PropertyNode *property_init = new PropertyNode(id++,ACCEPTANCE_CYCLE);
      // node_sptr p = node_sptr(new Not(node_sptr(new  Equal(node_sptr(new symbolic_expression::Variable("y")),node_sptr(new Number("12"))))));
      // property_init->add_edge(property_init,p);

      // [water tank 2]
      // :[]<>p
      // PropertyNode *property_init = new PropertyNode(id++,NORMAL);
      // PropertyNode *node1 = new PropertyNode(id++,ACCEPTANCE_CYCLE);
      // node_sptr true_node = node_sptr(new True());
      // node_sptr p = node_sptr(new Not(node_sptr(new  Equal(node_sptr(new symbolic_expression::Variable("y")),node_sptr(new Number("6"))))));
      // property_init->add_edge(property_init,true_node);
      // property_init->add_edge(node1,p);
      // node1->add_edge(node1,p);

      /*
      // for comparison
      auto property2 = nc_parse();
      auto property_init2 = (PropertyNode *)(property2->initial_node);

      cout << "===== Property Automaton =====" << endl;
      property2->dump(cout);
      */

      auto property = nc_parse();
      auto property_init = (PropertyNode *)(property->initial_node);

      cout << "===== Property Automaton =====" << endl;
      property->dump(cout);

      /*
      Automaton property;
      property.initial_node = property_init;

      cout << "===== Property Automaton =====" << endl;
      property.dump(cout);
      */

      LTLNode *LTL_init = new LTLNode("init",result_root_,property_init,id_counter++);
      current_automaton.initial_node = LTL_init;
      current_checking_node_t search_starting_node;
      search_starting_node.node = LTL_init;
      search_starting_node.created_nodes.push_back(LTL_init);
      current_checking_node_list_t search_list;
      search_list.push_back(search_starting_node);
      phase_list_t phase_list;
      // phase_list.push_back(result_root_);
      LTLsearch(result_root_, search_list, phase_list);
      int automaton_count = 1;
      for(auto automaton: result_automata)
        {
          cout << "===== Automaton" << automaton_count++ << " =====" << endl;
          automaton.dump(cout);
        }
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

void LTLModelChecker::LTLsearch(phase_result_sptr_t current,current_checking_node_list_t checking_list, phase_list_t phase_list)
{
  io::SymbolicTrajPrinter printer(backend);
  if(signal_handler::interrupted)
  {
    current->simulation_state = INTERRUPTED;
    return;
  }
  phase_simulator_->apply_diff(*current);
  HYDLA_LOGGER_DEBUG_VAR(*current);
  phase_list.push_back(current);
  if(current->todo_list.empty() & !checking_list.empty())
  {
    // TODO
    // The simulation for this case is terminated
    // current_checking_node_list_t next_node_list = transition(checking_list, current);
    Automaton result_automaton = current_automaton.clone();
    // for(auto checked_node : next_node_list){
    //   checked_node.node->remove();
    // }
    result_automata.push_back(result_automaton);
  }
  // if(opts_->max_phase >= 0 && current->step >= opts_->max_phase){
  //   cout << "come here " << endl;
  // }
  int count = 0;
  while(!current->todo_list.empty())
  {
    // cout << "ltlserch count" << count << endl;
    // cout << "phase list num " << phase_list.size() << endl;
    phase_result_sptr_t todo = current->todo_list.front();
    current->todo_list.pop_front();
    profile_vector_->insert(todo);
    if(todo->simulation_state == NOT_SIMULATED){
      phase_list_t result_list = process_one_todo(todo);
      if(opts_->dump_in_progress){
        printer.output_one_phase(todo);
      }
    }
    if(count >= 1){
      ConstraintStore par_cons = todo->get_parameter_constraint();
      checking_list = refresh(phase_list, par_cons);
    }

    /* TODO: assertion違反が検出された場合の対応 */
    current_checking_node_list_t next_node_list = transition(checking_list, todo);
    LTLsearch(todo, next_node_list, phase_list);
    count++ ;
  }
  phase_simulator_->revert_diff(*current);
  for(auto checked_node : checking_list){
    checked_node.node->remove();
  }
}

current_checking_node_list_t LTLModelChecker::transition(current_checking_node_list_t checking_list, phase_result_sptr_t phase){
  current_checking_node_list_t next_search;
  for(auto current : checking_list){
    for(auto property_node_edge : current.node->property->edges){
      //phase と property_edge->second(edgeのguard条件) で成否判定する
      if(check_edge_guard_wP(phase, property_node_edge.second,phase->get_parameter_constraint())){
        LTLNode* next_node;
        LTLNode* exist_node = (LTLNode*)current_automaton.exist_node("Property" + ((property_node_edge.first)->name) + " Phase" + to_string(phase->id));
        if(exist_node != nullptr) {
          next_node = exist_node;
        } else {
          next_node = new LTLNode(phase, (PropertyNode*)(property_node_edge.first), id_counter++);
        }

        //acceptance cycleの探索
        LTLNode* loop_node = detect_acceptance_cycle(next_node, current.acceptance_path_list);
        if(loop_node != NULL){ //acceptance cycle を発見した場合
          if(!current_automaton.exist_edge(current.node,loop_node)){
            current.node->add_edge(loop_node);
          }
          // current.node->set_color_to_trace_path("red");
          for(auto path : current.created_nodes){
            path->set_color("red");
          }
          loop_node->set_color("red");
          result_automata.push_back(current_automaton.clone());
          for(auto path : current.created_nodes){
            path->set_color("#000000");
          }
          next_node->set_color("#000000");
          next_search.clear();
          return next_search;
        }
        //通常ループの探索
        loop_node = detect_loop_in_path(next_node,current.created_nodes);
        if(loop_node != NULL){ //ループの場合
          if(!current_automaton.exist_edge(current.node,loop_node)){
            current.node->add_edge(loop_node);
          }
        } else { //ループでない場合
          if(!current_automaton.exist_edge(current.node,next_node)){
          current.node->add_edge(next_node);
          }
          if(next_node->acceptanceState()){ //acceptance stateの場合
            // next_node->set_color_to_trace_path("red");
            for(auto path : current.created_nodes){
              path->set_color("red");
            }
            next_node->set_color("red");
            result_automata.push_back(current_automaton.clone());
            for(auto path : current.created_nodes){
              path->set_color("#000000");
            }
            next_node->set_color("#000000");
            next_search.clear();
            return next_search;
          }
          //次の探索の準備
          current_checking_node_t next_search_candidate;
          next_search_candidate.node = next_node;
          ltl_node_list_t tmp_created_nodes = current.created_nodes;
          tmp_created_nodes.push_back(next_node);
          next_search_candidate.created_nodes = tmp_created_nodes;
          next_search_candidate.acceptance_path_list = current.acceptance_path_list;
          if(next_node->acceptanceCycle()){ //acceptance cycleの受理状態の場合
            (next_search_candidate.acceptance_path_list).push_back(next_search_candidate.created_nodes);
          }
          next_search.push_back(next_search_candidate);
        }
      }
    }
  }
  if(next_search.empty() & !checking_list.empty())
  {
    result_automata.push_back(current_automaton.clone());
  }
  return next_search;
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

LTLNode* LTLModelChecker::detect_acceptance_cycle(LTLNode* new_node,ltl_path_list_t acceptance_path_list){
  LTLNode* ret = NULL;
  for(ltl_path_list_t::iterator acceptance_path = acceptance_path_list.begin();acceptance_path != acceptance_path_list.end();acceptance_path++){
    ret = detect_loop_in_path(new_node, *acceptance_path);
    if(ret!=NULL){
      return ret;
    }
  }
  return ret;
}

LTLNode* LTLModelChecker::detect_loop_in_path(LTLNode* new_node, ltl_node_list_t path){
  LTLNode* ret = NULL;
  for(ltl_node_list_t::iterator it = path.begin();it != path.end();it++){
    if(check_including((LTLNode*)*it,new_node)){
      ret = (LTLNode*)*it;
      return ret;
    }
  }
  return ret;
}

bool LTLModelChecker::check_edge_guard(phase_result_sptr_t phase,node_sptr guard){
  bool ret;
  if(guard->get_node_type_name() == "True"){
    return true;
  }
  HYDLA_LOGGER_DEBUG("checking guard condition : ",get_infix_string(guard));
  VariableFinder var_finder;
  var_finder.visit_node(guard);
  variable_set_t related_variables = var_finder.get_all_variable_set();
  variable_map_t related_vm;
  variable_map_t vm = phase->variable_map;
  for(auto related_variable : related_variables)
    {
      auto vm_it = vm.find(related_variable);
      if(vm_it != vm.end())
        related_vm[related_variable] = vm_it->second;
    }
  HYDLA_LOGGER_DEBUG("variable map : ", related_vm);
  ConstraintStore par_cons = phase->get_parameter_constraint();
  HYDLA_LOGGER_DEBUG("parameter condition : ",par_cons);

  CheckConsistencyResult cc_result;
  switch(consistency_checker->check_entailment(related_vm, cc_result, guard, phase->phase_type, phase->profile)){
  case ENTAILED:
    ret = true;
    HYDLA_LOGGER_DEBUG("guard condition : true");
    break;
  case BRANCH_PAR:
  case CONFLICTING:
  case BRANCH_VAR:
    ret = false;
    HYDLA_LOGGER_DEBUG("guard condition : false");
    break;
  }
  return ret;
}

// phase_list_t LTLModelChecker::get_path(PhaseResult &phase){
//   phase_list_t ret;
//   if(*phase != result_root_){
//     ret = get_path(phase.parent);
//     ret.push_back(phase);
//   }
//   return ret;
// }

current_checking_node_list_t LTLModelChecker::refresh(phase_list_t phase_list,ConstraintStore par_cons){
  // cout << "reflesh start" << endl;
  //init
  LTLNode *LTL_init = new LTLNode("init",result_root_,property_init,id_counter++);
  Automaton new_automaton;
  new_automaton.initial_node = LTL_init;
  current_automaton.initial_node = LTL_init;
  current_checking_node_t search_starting_node;
  search_starting_node.node = LTL_init;
  search_starting_node.created_nodes.push_back(LTL_init);
  current_checking_node_list_t checking_list;
  current_checking_node_list_t next_search;
  checking_list.push_back(search_starting_node);
  //make automaton
  int c = 0;
  for(auto current_phase : phase_list){
    if(c == 0){
      c++;
      continue;
    }
    // cout << "current phase " << current_phase->id << endl;
    for(auto current_LTL : checking_list){
      for(auto property_node_edge : current_LTL.node->property->edges){
        //phase と property_edge->second(edgeのguard条件) で成否判定する
        if(check_edge_guard_wP(current_phase, property_node_edge.second,par_cons)){
          LTLNode* next_node;
          LTLNode* exist_node = (LTLNode*)new_automaton.exist_node("Property" + ((property_node_edge.first)->name) + " Phase" + to_string(current_phase->id));
          if(exist_node != nullptr){
            next_node = exist_node;
          } else {
            next_node = new LTLNode(current_phase, (PropertyNode*)(property_node_edge.first), id_counter++);
          }
          //acceptance cycleの探索
          LTLNode* loop_node = detect_acceptance_cycle_wP(next_node, current_LTL.acceptance_path_list,par_cons);
          if(loop_node != NULL){ //acceptance cycle を発見した場合
            if(!current_automaton.exist_edge(current_LTL.node,loop_node)){
              current_LTL.node->add_edge(loop_node);
            }
            // current.node->set_color_to_trace_path("red");
            for(auto path : current_LTL.created_nodes){
              path->set_color("red");
            }
            loop_node->set_color("red");
            result_automata.push_back(current_automaton.clone());
            for(auto path : current_LTL.created_nodes){
              path->set_color("#000000");
            }
            next_node->set_color("#000000");
            next_search.clear();
            return next_search;
          }
          //通常ループの探索
          loop_node = detect_loop_in_path_wP(next_node,current_LTL.created_nodes,par_cons);
          if(loop_node != NULL){ //ループの場合
            if(!current_automaton.exist_edge(current_LTL.node,loop_node)){
              current_LTL.node->add_edge(loop_node);
            }
          } else { //ループでない場合
            if(!current_automaton.exist_edge(current_LTL.node,next_node)){
              current_LTL.node->add_edge(next_node);
            }
            if(next_node->acceptanceState()){ //acceptance stateの場合
              // next_node->set_color_to_trace_path("red");
              for(auto path : current_LTL.created_nodes){
                path->set_color("red");
              }
              next_node->set_color("red");
              result_automata.push_back(current_automaton.clone());
              for(auto path : current_LTL.created_nodes){
                path->set_color("#000000");
              }
              next_node->set_color("#000000");
              next_search.clear();
              return next_search;
            }
            //次の探索の準備
            current_checking_node_t next_search_candidate;
            next_search_candidate.node = next_node;
            ltl_node_list_t tmp_created_nodes = current_LTL.created_nodes;
            tmp_created_nodes.push_back(next_node);
            next_search_candidate.created_nodes = tmp_created_nodes;
            next_search_candidate.acceptance_path_list = current_LTL.acceptance_path_list;
            if(next_node->acceptanceCycle()){ //acceptance cycleの受理状態の場合
              (next_search_candidate.acceptance_path_list).push_back(next_search_candidate.created_nodes);
            }
            next_search.push_back(next_search_candidate);
          }
        }
      }
    }
    if(next_search.empty() & !checking_list.empty()){
      result_automata.push_back(current_automaton.clone());
    }
  }
  // cout << "new automaton" << endl;
  // new_automaton.dump(cout);
  // cout << "new automaton" << endl;

  // cout << "reflesh end" << endl;
  return next_search;
}

bool LTLModelChecker::check_including_wP(LTLNode* larger,LTLNode* smaller,ConstraintStore par_cons){
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
                &(larger->phase->current_time), &(larger->phase->variable_map), &par_cons,
                &(smaller->phase->current_time), &(smaller->phase->variable_map), &par_cons, &include_ret);
  if(include_ret){
    // cout << "\n\"" << larger->id << "\" includes \"" << smaller->id << "\"\n" << endl;
  }
  else{
    // cout << "not included :\n\t \"" << larger->id << "\" : \"" << smaller->id << "\"" << endl;
  }
  return include_ret;
}

LTLNode* LTLModelChecker::detect_acceptance_cycle_wP(LTLNode* new_node,ltl_path_list_t acceptance_path_list,ConstraintStore par_cons){
  LTLNode* ret = NULL;
  for(ltl_path_list_t::iterator acceptance_path = acceptance_path_list.begin();acceptance_path != acceptance_path_list.end();acceptance_path++){
    ret = detect_loop_in_path_wP(new_node, *acceptance_path, par_cons);
    if(ret!=NULL){
      return ret;
    }
  }
  return ret;
}

LTLNode* LTLModelChecker::detect_loop_in_path_wP(LTLNode* new_node, ltl_node_list_t path,ConstraintStore par_cons){
  LTLNode* ret = NULL;
  for(ltl_node_list_t::iterator it = path.begin();it != path.end();it++){
    if(check_including_wP((LTLNode*)*it,new_node,par_cons)){
      ret = (LTLNode*)*it;
      return ret;
    }
  }
  return ret;
}

bool LTLModelChecker::check_edge_guard_wP(phase_result_sptr_t phase,node_sptr guard,ConstraintStore par_cons){
  // cout << "check_edge_gurad wp " << endl;
  bool ret;
  if(guard->get_node_type_name() == "True"){
    // cout << "check_edge_gurad wp end" << endl;
    return true;
  }
  HYDLA_LOGGER_DEBUG("checking guard condition : ",get_infix_string(guard));
  VariableFinder var_finder;
  var_finder.visit_node(guard);
  variable_set_t related_variables = var_finder.get_all_variable_set();
  variable_map_t related_vm;
  variable_map_t vm = phase->variable_map;
  for(auto related_variable : related_variables)
  {
    auto vm_it = vm.find(related_variable);
    if(vm_it != vm.end())
      related_vm[related_variable] = vm_it->second;
  }
  HYDLA_LOGGER_DEBUG("variable map : ", related_vm);
  // ConstraintStore par_cons = phase->get_parameter_constraint();
  HYDLA_LOGGER_DEBUG("parameter condition : ",par_cons);

  // backend->call("checkInclude", true, 6, "vlnmvtcsnvlnmvtcsn", "b",
  //               &(larger->phase->current_time), &(larger->phase->variable_map), &larger_cons,
  //               &(smaller->phase->current_time), &(smaller->phase->variable_map), &smaller_cons, &include_ret);
  if(phase->phase_type == POINT_PHASE){
    backend->call("checkEdgeGuard", true, 3, "etmvtcsn", "b",
                  &guard, &related_vm, &par_cons, &ret);
    // cout << "phase " << phase->id << " : " << get_infix_string(guard) << " : " << ret << endl;
  }else {
    backend->call("checkEdgeGuardWt", true, 5, "etmvtcsnvlnvln", "b",
                  &guard, &related_vm, &par_cons, &(phase->current_time), &(phase->end_time), &ret);
    // cout << "phase " << phase->id << " : " << get_infix_string(guard) << " : " << ret << endl;
  }
  // CheckConsistencyResult cc_result;
  // switch(consistency_checker->check_entailment(related_vm, cc_result, guard, phase->phase_type, phase->profile)){
  // case ENTAILED:
  //   ret = true;
  //   HYDLA_LOGGER_DEBUG("guard condition : true");
  //   break;
  // case BRANCH_PAR:
  // case CONFLICTING:
  // case BRANCH_VAR:
  //   ret = false;
  //   HYDLA_LOGGER_DEBUG("guard condition : false");
  //   break;
  // }
  // return ret;
  // cout << "check_edge_gurad wp end" << endl;

  return ret;
}


}//namespace hydla
}//namespace simulator

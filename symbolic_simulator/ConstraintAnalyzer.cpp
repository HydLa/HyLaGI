#include <iostream>
#include <ostream>
#include <fstream>

#include "ConstraintAnalyzer.h"
#include "TellCollector.h"
#include "AskCollector.h"
#include "ContinuityMapMaker.h"
#include "TreeInfixPrinter.h"
#include "NonPrevSearcher.h"
#include "Timer.h"

#include "../virtual_constraint_solver/mathematica/MathematicaVCS.h"

using namespace std;
using namespace hydla::ch;
using namespace hydla::simulator::symbolic;
using namespace hydla::simulator;
using namespace hydla::parse_tree;
using namespace hydla::vcs;
using namespace hydla::vcs::mathematica;
using namespace hydla::timer;

namespace hydla{
namespace simulator{
namespace symbolic {

ConstraintAnalyzer::ConstraintAnalyzer(Opts& opts):Simulator(opts){}

ConstraintAnalyzer::~ConstraintAnalyzer(){}

void ConstraintAnalyzer::print_conditions()
{
  conditions_map_t::iterator it = conditions_.begin();
  std::ofstream ofs;
  if(opts_->analysis_file != ""){
    ofs.open(opts_->analysis_file.c_str());
  }
  for(;it != conditions_.end();it++){
    if(opts_->analysis_file != ""){
      // 出力先がファイルの場合
      ofs << (*it).first << ":";
      if((*it).second != NULL){
        ofs << *((*it).second);
      }
      ofs << std::endl;
    }else{
      // 出力先が標準出力の場合
      std::cout << (*it).first << ":";
      if((*it).second != NULL){
	std::cout << TreeInfixPrinter().get_infix_string((*it).second);
      }
      std::cout << std::endl;
    }
  }
  if(!cm_list_.empty()){
    for(cm_map_list_t::iterator it = cm_list_.begin(); it != cm_list_.end(); it++){
      std::cout << *(*it) << std::endl;
    }
  }

  if(opts_->analysis_file != ""){
    ofs.close();
  }
}

void ConstraintAnalyzer::initialize(const parse_tree_sptr& parse_tree)
{
  init_module_set_container(parse_tree);

  init_variable_map(parse_tree);

  solver_.reset(new MathematicaVCS(*opts_));
  solver_->set_variable_set(*variable_set_);
  solver_->set_parameter_set(*parameter_set_);
}

void ConstraintAnalyzer::add_new_cm(const module_set_sptr& ms){
  // msに対する無矛盾の条件
  node_sptr cond = conditions_[ms->get_name()];
  cm_map_list_t parents_cm;
  // flag用変数
  bool in_m = false;

  // cm_list_を見てmsはどこに入れるべきか判断
  for(cm_map_list_t::iterator it = cm_list_.begin(); it != cm_list_.end(); it++){
    // 今回得た条件と同値な条件をキーに持つcmがあった場合
    if(solver_->equivalent((*it)->get_condition(), cond)){
      in_m = true;
      // そのcm内にmsを包含するモジュール集合があったら何もせず抜ける
      if((*it)->is_super_cm(ms)) break;
      // それ以外の場合はmsを追加
      // TODO:本来はここに包含性を考慮する分岐を書くべき
      cm_map_list_t parents = (*it)->get_parents();
      bool flag;
      for(cm_map_list_t::iterator cmv = parents_cm.begin(); cmv != parents_cm.end(); cmv++){
	flag = false;
	for(cm_map_list_t::iterator pit = parents.begin(); pit != parents.end(); pit++){
	  if(*cmv == *pit){
	    flag = true;
	    break;
	  }
	}
	if(!flag) break;
      }
      if(flag) (*it)->add_module_set(ms);
      break;
    }else{
      if((*it)->is_super_cm(ms)){
	parents_cm.push_back((*it));
      }
    }
  }
  // cm内に同値な条件をキーに持つcmがなかった場合
  if(!in_m){
    cm_map_sptr new_cm(new CMMap(cond));
    new_cm->add_module_set(ms);
    for(cm_map_list_t::iterator pit = parents_cm.begin(); pit != parents_cm.end(); pit++){
      new_cm->add_parents(*pit);
      (*pit)->add_children(new_cm);
    }
    /*
    // 実際に条件を使う時用にrootを保持しておく
    if(root_cm_ == NULL){
      root_cm_ = cm_map_sptr(new_cm);
    }
    */
    cm_list_.push_back(new_cm);
  }
}

void ConstraintAnalyzer::check_all_module_set(bool b)
{
  Timer ca_timer;
  msc_no_init_->reset();
  module_set_sptr check_ms;
  // 単にすべての解候補制約モジュール集合にfind_conditionsを適用しているだけ
  while(msc_no_init_->go_next()){
    check_ms = msc_no_init_->get_module_set();
    Timer fc_timer;
    ConstraintAnalyzer::ConditionsResult ret = find_conditions(check_ms,true);
    //    std::cout << "  find conditions:" << check_ms->get_name() << " : " << fc_timer.get_elapsed_us() << " us" << std::endl;

    if(ret != CONDITIONS_FALSE && b){
      add_new_cm(check_ms);
    }

    //    std::cout << "  " << check_ms->get_name() << " : " << fc_timer.get_elapsed_us() << " us" << std::endl;
    msc_no_init_->mark_current_node();
  }

  /*
  for(cm_map_list_t::iterator it = cm_list_.begin(); it != cm_list_.end(); it++){
    std::cout << *(*it);
  }
  std::cout<<std::endl;
  */
  std::cout << "analysis : " << ca_timer.get_elapsed_us() << std::endl;
}

  ConstraintAnalyzer::ConditionsResult ConstraintAnalyzer::find_conditions(const module_set_sptr& ms, bool b)
{
  bool non_prev;
  NonPrevSearcher searcher;
  if(opts_->analysis_mode == "debug") 
    std::cout << ms->get_name() << std::endl;
  solver_->change_mode(ConditionsMode, opts_->approx_precision);
  ConstraintAnalyzer::ConditionsResult ret = CONDITIONS_FALSE;

  positive_asks_t positive_asks;
  negative_asks_t negative_asks;
  expanded_always_t expanded_always;
  TellCollector tell_collector(ms);
  AskCollector ask_collector(ms);
  tells_t tell_list;
  constraints_t constraint_list;

  continuity_map_t continuity_map;
  ContinuityMapMaker maker;
  negative_asks_t tmp_negative;

  variable_map_t vm;
  parameter_map_t pm;
  solver_->reset(vm, pm);

  // ask制約を集める。ここではexpanded_always=null, positive_asks=null, tmp_negetive=null, negative_asks=ask制約の配列となっているはず
  ask_collector.collect_ask(&expanded_always,
			    &positive_asks,
			    &tmp_negative,
			    &negative_asks);

  // 求められた条件を入れる変数
  node_sptr condition_node;
  // ガード条件の仮定に関する条件を保持する変数
  node_sptr guard_condition;

  // ask制約のガード条件の成否のパターンを全通り考える(2^(ask制約の個数)通り)
  for(int i = 0; i < (1 << negative_asks.size()) && ret != CONDITIONS_TRUE; i++){
    non_prev = false;
    positive_asks_t tmp_positive_asks;
    solver_->start_temporary();
    maker.reset();
    constraint_list.clear();

    tmp_positive_asks.clear();

    negative_asks_t::iterator it = negative_asks.begin();

    // ガード条件の成否の仮定。i の各ビットが成否に対応
    // つまり i = 11(1011) でask制約が5つあれば否、成、否、成、成の順に仮定する
    for(int j = 0; j < (int)negative_asks.size(); j++, it++){
      if((i & (1 << j)) != 0){
	if(opts_->analysis_mode == "debug") 
	  std::cout << " +++ " << TreeInfixPrinter().get_infix_string((*it)->get_guard()) << std::endl;
        tmp_positive_asks.insert(*it);
        constraint_list.push_back((*it)->get_guard());
      }else{
	if(opts_->analysis_mode == "debug") 
	  std::cout << " --- " << TreeInfixPrinter().get_infix_string((*it)->get_guard()) << std::endl;
	// 成り立たないと仮定したガード条件の後件に関する連続性を見る
        constraint_list.push_back(node_sptr(new Not((*it)->get_guard())));
        if(searcher.judge_non_prev((*it)->get_guard())){
	  maker.visit_node((*it)->get_child(), false, true);
	  non_prev = true;
	}
      }
    }

    // tell制約とガード条件が成立するask制約の後件を集める
    tell_collector.collect_all_tells(&tell_list,
				     &expanded_always,
				     &tmp_positive_asks);  

    //    std::cout << "collected tell for guard" << std::endl;
    for(tells_t::iterator it = tell_list.begin(); it != tell_list.end(); it++){
      //      std::cout << TreeInfixPrinter().get_infix_string((*it)->get_child()) << std::endl;
      constraint_list.push_back((*it)->get_child());
      maker.visit_node((*it), false, false);
    }

    // 集めた制約をソルバに送る
    solver_->add_constraint(constraint_list);

    // デフォルト連続性を表わす制約をソルバに送る
    continuity_map = maker.get_continuity_map();
    for(continuity_map_t::const_iterator it = continuity_map.begin(); it != continuity_map.end();it++){
      if(it->second>=0){
        for(int i=0; i<it->second;i++){
          solver_->set_continuity(it->first, i);
        }
      }else{
        node_sptr lhs(new Variable(it->first));
        for(int i=0; i<=-it->second;i++){
          solver_->set_continuity(it->first, i);
          lhs = node_sptr(new Differential(lhs));
        }
        node_sptr rhs(new Number("0"));
        node_sptr cons(new Equal(lhs, rhs));
        solver_->add_constraint(cons);
      }
    }
    if(non_prev){
      //      std::cout << "exist non prev" << std::endl;
      solver_->find_conditions(guard_condition);
      
      solver_->end_temporary();
      solver_->start_temporary();
    
      maker.reset();
      
      if(guard_condition != NULL){
	constraint_list.push_back(guard_condition);
	//      std::cout << "guard condition : " << TreeInfixPrinter().get_infix_string(guard_condition) << std::endl;
      }
      // tell制約とガード条件が成立するask制約の後件を集める
      tell_collector.collect_all_tells(&tell_list,
				       &expanded_always,
				       &tmp_positive_asks);  
      
      //    std::cout << "send tell" << std::endl;
      for(tells_t::iterator it = tell_list.begin(); it != tell_list.end(); it++){
	constraint_list.push_back((*it)->get_child());
	maker.visit_node((*it), false, false);
	//      std::cout << TreeInfixPrinter().get_infix_string((*it)->get_child()) << std::endl;
      }
      
      solver_->add_constraint(constraint_list);
      
      // デフォルト連続性を表わす制約をソルバに送る
      continuity_map = maker.get_continuity_map();
      for(continuity_map_t::const_iterator it = continuity_map.begin(); it != continuity_map.end();it++){
	if(it->second>=0){
	  for(int i=0; i<it->second;i++){
	    solver_->set_continuity(it->first, i);
	  }
	}else{
	  node_sptr lhs(new Variable(it->first));
	  for(int i=0; i<=-it->second;i++){
	    solver_->set_continuity(it->first, i);
	    lhs = node_sptr(new Differential(lhs));
	  }
	  node_sptr rhs(new Number("0"));
	  node_sptr cons(new Equal(lhs, rhs));
	  solver_->add_constraint(cons);
	}
      }
    }
    {
      node_sptr tmp_node;
      //      std::cout << "    result" << std::endl;
      // あるガード条件の仮定の下に得られた条件をtmp_nodeに入れる
      switch(solver_->find_conditions(tmp_node)){
      case SymbolicVirtualConstraintSolver::CONDITIONS_TRUE:
        // もし求まった条件がTrueなら後の仮定の結果がどうであれTrueになるのでret = CONDITIONS_TRUEとおく
	if(opts_->analysis_mode == "debug") 
	  std::cout << "        True" << std::endl;
        ret = CONDITIONS_TRUE;
        break;
      case SymbolicVirtualConstraintSolver::CONDITIONS_FALSE:
        if(opts_->analysis_mode == "debug") 
	  std::cout << "        False" << std::endl;
        break;
      case SymbolicVirtualConstraintSolver::CONDITIONS_VARIABLE_CONDITIONS:
        // なんらかの条件が求まったら今まで求めた条件に論理和でその条件を追加する
	if(opts_->analysis_mode == "debug")
	  std::cout << "        " << TreeInfixPrinter().get_infix_string(tmp_node) << std::endl; 
        if(condition_node == NULL) condition_node = node_sptr(tmp_node);
        else condition_node = node_sptr(new LogicalOr(condition_node, tmp_node));
        // 何らかの条件が求まったことを表わすためにret = CONDITIONS_VARIABLE_CONDITIONSとする
        ret = CONDITIONS_VARIABLE_CONDITIONS;
        break;
      default:
        assert(0);
        break;
      }
    }
    if(ret == CONDITIONS_TRUE) break;
    solver_->end_temporary();
  }
  //  std::cout << std::endl;
  switch(ret){
  case CONDITIONS_VARIABLE_CONDITIONS:
    // 何らかの条件が求まったとしても簡約するとTrueになる可能性があるので簡約する
    // 条件を簡単にする目的もある
    // 求まっている条件は A || B || … となっていて ret がCONDITIONS_VARIABLE_CONDITIONSになっていることから簡約してもFalseには絶対にならない
    if(!b) condition_node = node_sptr(new Not(condition_node));
    solver_->node_simplify(condition_node);
    if(condition_node == NULL){
      ret = CONDITIONS_TRUE;
      conditions_[ms->get_name()] = node_sptr(new Number("1"));
    }else{
      conditions_[ms->get_name()] = condition_node;
    }
    break;
  case CONDITIONS_TRUE:
    conditions_[ms->get_name()] = node_sptr(new Number("1"));
    break;
  case CONDITIONS_FALSE:
    break;
  default:
    assert(0);
    break;
  }
  return ret;
}

phase_result_const_sptr_t ConstraintAnalyzer::simulate(){
  return phase_result_const_sptr_t();
}

}
}
}

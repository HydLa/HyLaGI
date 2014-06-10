#include <iostream>
#include <ostream>
#include <fstream>

#include "ConstraintAnalyzer.h"
#include "TellCollector.h"
#include "AskCollector.h"
#include "ContinuityMapMaker.h"
#include "Timer.h"
#include "PrevSearcher.h"

#include "../backend/Backend.h"

using namespace std;
using namespace hydla::hierarchy;
using namespace hydla::simulator;
using namespace hydla::symbolic_expression;
using namespace hydla::timer;

namespace hydla{
namespace simulator{

ConstraintAnalyzer::ConstraintAnalyzer(){}
ConstraintAnalyzer::ConstraintAnalyzer(backend_sptr_t back):backend_(back){}

ConstraintAnalyzer::~ConstraintAnalyzer(){}

void ConstraintAnalyzer::set_backend(backend_sptr_t back)
{
  backend_ = back;
}

void ConstraintAnalyzer::print_conditions()
{
  conditions_map_t::iterator it = conditions_.begin();
  std::ofstream ofs;
  /*
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
    */
      // 出力先が標準出力の場合
      std::cout << (*it).first << ":";
      if((*it).second != NULL){
	std::cout << get_infix_string((*it).second);
      }
      std::cout << std::endl;
   // }
 // }
  if(!cm_list_.empty()){
    for(cm_map_list_t::iterator it = cm_list_.begin(); it != cm_list_.end(); it++){
      std::cout << *(*it) << std::endl;
    }
  }
/*
  if(opts_->analysis_file != ""){
    ofs.close();
  }
  */
}

void ConstraintAnalyzer::add_new_cm(const module_set_sptr& ms){
  // msに対する無矛盾の条件
  symbolic_expression::node_sptr cond = conditions_[ms->get_name()];
  cm_map_list_t parents_cm;
  // flag用変数
  bool in_m = false;

  // cm_list_を見てmsはどこに入れるべきか判断
  for(cm_map_list_t::iterator it = cm_list_.begin(); it != cm_list_.end(); it++){
    // 今回得た条件と同値な条件をキーに持つcmがあった場合
    int equal;
    backend_->call("equivalent", 2, "epep", "i", &((*it)->get_condition()), &cond, &equal);
    if(equal){
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
    
    // 実際に条件を使う時用にrootを保持しておく
    //if(root_cm_ == NULL){
    //  root_cm_ = cm_map_sptr(new_cm);
    //}
    
    cm_list_.push_back(new_cm);
  }
}

void ConstraintAnalyzer::check_all_module_set(bool b)
{
  /*
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

  
  //for(cm_map_list_t::iterator it = cm_list_.begin(); it != cm_list_.end(); it++){
  //  std::cout << *(*it);
  //}
  //std::cout<<std::endl;
  std::cout << "analysis : " << ca_timer.get_elapsed_us() << std::endl;
  */
}

void ConstraintAnalyzer::add_continuity(const continuity_map_t& continuity_map, const PhaseType &phase){
  for(continuity_map_t::const_iterator it = continuity_map.begin(); it != continuity_map.end();it++){

    std::string fmt = "v";
    if(phase == PointPhase)
    {
      fmt += "n";
    }
    else
    {
      fmt += "z";
    }
    fmt += "vp";
    if(it->second>=0){
      for(int i=0; i<it->second;i++){
        variable_t var(it->first, i);
        backend_->call("addInitEquation", 2, fmt.c_str(), "", &var, &var);
      }
    }else{
      for(int i=0; i<=-it->second;i++){
        variable_t var(it->first, i);
        backend_->call("addInitEquation", 2, fmt.c_str(), "", &var, &var);
      }
      if(phase == IntervalPhase)
      {
        symbolic_expression::node_sptr rhs(new Number("0"));
        fmt = phase == PointPhase?"vn":"vt";
        fmt += "en";
        variable_t var(it->first, -it->second + 1);
        backend_->call("addEquation", 2, fmt.c_str(), "", &var, &rhs);
      }
    }
  }
}


  ConstraintAnalyzer::ConditionsResult ConstraintAnalyzer::find_conditions(const module_set_sptr& ms, bool b)
{
/* TODO: implement
  bool non_prev;
  PrevSearcher searcher;
 // if(opts_->analysis_mode == "debug") 
    std::cout << ms->get_name() << std::endl;
  ConstraintAnalyzer::ConditionsResult ret = CONDITIONS_FALSE;

  positive_asks_t positive_asks;
  negative_asks_t negative_asks;
  always_set_t expanded_always;
  TellCollector tell_collector(ms);
  AskCollector ask_collector(ms);
  tells_t tell_list;
  ConstraintStore constraint_list;

  continuity_map_t continuity_map;
  ContinuityMapMaker maker;
  negative_asks_t tmp_negative;

  variable_map_t vm;
  parameter_map_t pm;
  backend_->call("resetConstraint", 0, "", "");
  backend_->call("addParameterConstraint", 1, "mp", "", &pm);
  backend_->call("addPrevConstraint", 1, "mvp", "", &vm);
  // ask制約を集める。ここではexpanded_always=null, positive_asks=null, tmp_negetive=null, negative_asks=ask制約の配列となっているはず
  ask_collector.collect_ask(&expanded_always,
			    &positive_asks,
			    &tmp_negative,
			    &negative_asks);

  // 求められた条件を入れる変数
  symbolic_expression::node_sptr condition_node;
  // ガード条件の仮定に関する条件を保持する変数
  symbolic_expression::node_sptr guard_condition;

  // ask制約のガード条件の成否のパターンを全通り考える(2^(ask制約の個数)通り)
  for(int i = 0; i < (1 << negative_asks.size()) && ret != CONDITIONS_TRUE; i++){
    non_prev = false;
    positive_asks_t tmp_positive_asks;
    backend_->call("startTemporary", 0, "", "");
    maker.reset();
    constraint_list.clear();

    tmp_positive_asks.clear();

    negative_asks_t::iterator it = negative_asks.begin();

    // ガード条件の成否の仮定。i の各ビットが成否に対応
    // つまり i = 11(1011) でask制約が5つあれば否、成、否、成、成の順に仮定する
    for(int j = 0; j < (int)negative_asks.size(); j++, it++){
      if((i & (1 << j)) != 0){
	  std::cout << " +++ " << get_infix_string((*it)->get_guard()) << std::endl;
        tmp_positive_asks.insert(*it);
        constraint_list.add_constraint((*it)->get_guard());
      }else{
	  std::cout << " --- " << get_infix_string((*it)->get_guard()) << std::endl;
	// 成り立たないと仮定したガード条件の後件に関する連続性を見る
        constraint_list.add_constraint(symbolic_expression::node_sptr(new Not((*it)->get_guard())));
        if(!searcher.search_prev((*it)->get_guard())){
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
      //      std::cout << get_infix_string((*it)->get_child()) << std::endl;
      constraint_list.add_constraint((*it)->get_child());
      maker.visit_node((*it), false, false);
    }

    // 集めた制約をソルバに送る
    // TODO? : IP にも対応する
    const char* fmt = (true)?"csn":"cst";
    backend_->call("addConstraint", 1, fmt, "", &constraint_list);

    // デフォルト連続性を表わす制約をソルバに送る
    continuity_map = maker.get_continuity_map();
    add_continuity(continuity_map, PointPhase);

    if(non_prev){
      //      std::cout << "exist non prev" << std::endl;
      backend_->call("findConditions", 0, "", "ep", &guard_condition);
      
      backend_->call("endTemporary", 0, "", "");
      backend_->call("startTemporary", 0, "", "");
    
      maker.reset();
      
      if(guard_condition != NULL){
        constraint_list.add_constraint(guard_condition);
        //      std::cout << "guard condition : " << get_infix_string(guard_condition) << std::endl;
      }
      // tell制約とガード条件が成立するask制約の後件を集める
      tell_collector.collect_all_tells(&tell_list,
				       &expanded_always,
				       &tmp_positive_asks);  
      
      //    std::cout << "send tell" << std::endl;
      for(tells_t::iterator it = tell_list.begin(); it != tell_list.end(); it++){
	constraint_list.add_constraint((*it)->get_child());
// TODO? : IPにも対応する
	maker.visit_node((*it), false, false);
	//      std::cout << get_infix_string((*it)->get_child()) << std::endl;
      }
      
      // TODO? : IP にも対応する
      const char* fmt = (true)?"csn":"cst";
      backend_->call("addConstraint", 1, fmt, "", &constraint_list);
      
      // デフォルト連続性を表わす制約をソルバに送る
      continuity_map = maker.get_continuity_map();
      add_continuity(continuity_map, PointPhase);
    }
    {
      symbolic_expression::node_sptr tmp_node;
      //      std::cout << "    result" << std::endl;
      // あるガード条件の仮定の下に得られた条件をtmp_nodeに入れる
      backend_->call("findConditions", 0, "", "ep", &tmp_node);

      if(tmp_node != NULL){
        if((new True())->is_same_struct(*tmp_node,false)){
          ret = CONDITIONS_TRUE;
          backend_->call("endTemporary",0,"","");
	  break;
	}
        if(condition_node == NULL) condition_node = symbolic_expression::node_sptr(tmp_node);
        else condition_node = symbolic_expression::node_sptr(new LogicalOr(condition_node, tmp_node));
        ret = CONDITIONS_VARIABLE_CONDITIONS;  
      }
    }
    backend_->call("endTemporary",0,"","");
    if(ret == CONDITIONS_TRUE) break;
  }
  //  std::cout << std::endl;
  switch(ret){
  case CONDITIONS_VARIABLE_CONDITIONS:
    // 何らかの条件が求まったとしても簡約するとTrueになる可能性があるので簡約する
    // 条件を簡単にする目的もある
    // 求まっている条件は A || B || … となっていて ret がCONDITIONS_VARIABLE_CONDITIONSになっていることから簡約してもFalseには絶対にならない
    if(!b) condition_node = symbolic_expression::node_sptr(new Not(condition_node));
 //   backend_->call("Simplify", 1, "ep", "ep", &condition_node, &condition_node);
    if((new True())->is_same_struct(*condition_node, false)){
      ret = CONDITIONS_TRUE;
    }
    conditions_[ms->get_name()] = condition_node;
    break;
  case CONDITIONS_TRUE:
    conditions_[ms->get_name()] = symbolic_expression::node_sptr(new True());
    break;
  case CONDITIONS_FALSE:
    break;
  default:
    assert(0);
    break;
  }
  return ret;
*/

}
}
}

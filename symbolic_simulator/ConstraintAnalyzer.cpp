#include <iostream>
#include <ostream>
#include <fstream>

#include "ConstraintAnalyzer.h"
#include "TellCollector.h"
#include "AskCollector.h"
#include "ContinuityMapMaker.h"
#include "TreeInfixPrinter.h"

#include "../virtual_constraint_solver/mathematica/MathematicaVCS.h"

using namespace std;
using namespace hydla::simulator::symbolic;
using namespace hydla::simulator;
using namespace hydla::parse_tree;
using namespace hydla::vcs;
using namespace hydla::vcs::mathematica;


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

void ConstraintAnalyzer::check_all_module_set(bool b)
{
  msc_no_init_->reset();
  // 単にすべての解候補制約モジュール集合にfind_conditionsを適用しているだけ
  while(msc_no_init_->go_next()){
    find_conditions(msc_no_init_->get_module_set(),b);
    msc_no_init_->mark_current_node();
  }
}

  ConstraintAnalyzer::ConditionsResult ConstraintAnalyzer::find_conditions(const module_set_sptr& ms, bool b)
{

  //  std::cout << ms->get_name() << std::endl;
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
	//	std::cout << " +++ " << TreeInfixPrinter().get_infix_string((*it)->get_guard()) << std::endl;
        tmp_positive_asks.insert(*it);
        constraint_list.push_back((*it)->get_guard());
      }else{
	//	std::cout << " --- " << TreeInfixPrinter().get_infix_string((*it)->get_guard()) << std::endl;
	// 成り立たないと仮定したガード条件の後件に関する連続性を見る
        constraint_list.push_back(node_sptr(new Not((*it)->get_guard())));
	maker.visit_node((*it)->get_child(), false, true);
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

    {
      node_sptr tmp_node;
      //      std::cout << "    result" << std::endl;
      // あるガード条件の仮定の下に得られた条件をtmp_nodeに入れる
      switch(solver_->find_conditions(tmp_node)){
      case SymbolicVirtualConstraintSolver::CONDITIONS_TRUE:
        // もし求まった条件がTrueなら後の仮定の結果がどうであれTrueになるのでret = CONDITIONS_TRUEとおく
	//	std::cout << "        True" << std::endl;
        ret = CONDITIONS_TRUE;
        break;
      case SymbolicVirtualConstraintSolver::CONDITIONS_FALSE:
	//	std::cout << "        False" << std::endl;
        break;
      case SymbolicVirtualConstraintSolver::CONDITIONS_VARIABLE_CONDITIONS:
        // なんらかの条件が求まったら今まで求めた条件に論理和でその条件を追加する
	//	std::cout << "        " << TreeInfixPrinter().get_infix_string(tmp_node) << std::endl; 
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
      conditions_[ms->get_name()] = node_sptr();
    }else{
      conditions_[ms->get_name()] = condition_node;
    }
    break;
  case CONDITIONS_TRUE:
    conditions_[ms->get_name()] = node_sptr();
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

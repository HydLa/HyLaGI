#include "SymbolicSimulator.h"

#include <iostream>
#include <fstream>
#include <stack>
#include <boost/xpressive/xpressive.hpp>
//#include <boost/thread.hpp>
#include <boost/iterator/indirect_iterator.hpp>
#include <boost/bind.hpp>
#include <boost/foreach.hpp>
#include <boost/make_shared.hpp>

#include "Logger.h"
#include "Timer.h"

//仮追加
#include "RTreeVisitor.h"
#include "TellCollector.h"
#include "AskCollector.h"

#include "ModuleSetList.h"
#include "ModuleSetGraph.h"
#include "ModuleSetContainerCreator.h"
#include "InitNodeRemover.h"
#include "AskDisjunctionSplitter.h"
#include "AskDisjunctionFormatter.h"
#include "../virtual_constraint_solver/mathematica/MathematicaVCS.h"
//#include "../virtual_constraint_solver/reduce/REDUCEVCS.h"
#include "ContinuityMapMaker.h"

#include "SimulateError.h"

using namespace hydla::vcs;
using namespace hydla::vcs::mathematica;

//using namespace hydla::vcs::reduce;

using namespace std;
using namespace boost;
using namespace boost::xpressive;

using namespace hydla::ch;
using namespace hydla::symbolic_simulator;
using namespace hydla::simulator;
using namespace hydla::parse_tree;
using namespace hydla::logger;
using namespace hydla::timer;

using hydla::simulator::TellCollector;
using hydla::simulator::AskCollector;
using hydla::simulator::ContinuityMapMaker;
using hydla::simulator::IntervalPhase;
using hydla::simulator::PointPhase;

namespace hydla {
  namespace symbolic_simulator {

    SymbolicSimulator::SymbolicSimulator(const Opts& opts) :
      simulator_t(opts), is_safe_(true)
    {
    }

    SymbolicSimulator::~SymbolicSimulator()
    {
    }

    void SymbolicSimulator::initialize(const parse_tree_sptr& parse_tree, variable_set_t &v, parameter_set_t &p, variable_map_t &m)
    {
      simulator_t::initialize(parse_tree, v, p, m);
      variable_derivative_map_ = parse_tree->get_variable_map();

      // 使用するソルバの決定
      /*  if(opts_->solver == "r" || opts_->solver == "Reduce") {
          solver_.reset(new REDUCEVCS(opts_, variable_map_));
          }else{*/
      solver_.reset(new MathematicaVCS(*opts_));
      //}
      solver_->set_variable_set(*variable_set_);
      solver_->set_parameter_set(*parameter_set_);
    }

    
    void SymbolicSimulator::push_branch_states(phase_state_sptr &original, SymbolicVirtualConstraintSolver::check_consistency_result_t &result, CalculateClosureResult &dst){
      for(int i=0; i<(int)result.true_parameter_maps.size();i++){
        phase_state_sptr branch_state(create_new_phase_state(original));
        branch_state->parameter_map = result.true_parameter_maps[i];
        dst.push_back(branch_state);
      }
      for(int i=0; i<(int)result.false_parameter_maps.size();i++){
        phase_state_sptr branch_state(create_new_phase_state(original));
        branch_state->parameter_map = result.false_parameter_maps[i];
        dst.push_back(branch_state);
      }
    }
    

    void SymbolicSimulator::add_continuity(const continuity_map_t& continuity_map){
      HYDLA_LOGGER_CLOSURE("#*** Begin SymbolicSimulator::add_continuity ***\n");
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
      HYDLA_LOGGER_CLOSURE("#*** End SymbolicSimulator::add_continuity ***\n");
    }


    CalculateClosureResult SymbolicSimulator::calculate_closure(phase_state_sptr& state,
        const module_set_sptr& ms, expanded_always_t &expanded_always,
        positive_asks_t &positive_asks, negative_asks_t &negative_asks){    
      HYDLA_LOGGER_CLOSURE("#*** Begin SymbolicSimulator::calculate_closure ***\n");

      //前準備
      TellCollector tell_collector(ms);
      AskCollector  ask_collector(ms);
      tells_t         tell_list;
      constraints_t   constraint_list;
      boost::shared_ptr<hydla::parse_tree::Ask>  const *branched_ask;

      continuity_map_t continuity_map;
      ContinuityMapMaker maker;

      bool expanded;
      do{
        // tell制約を集める
        tell_collector.collect_new_tells(&tell_list,
            &expanded_always, 
            &positive_asks);

        HYDLA_LOGGER_CLOSURE("%% SymbolicSimulator::check_consistency in calculate_closure\n");
        //tellじゃなくて制約部分のみ送る
        constraint_list.clear();
        
        maker.reset();

        for(tells_t::iterator it = tell_list.begin(); it != tell_list.end(); it++){
          constraint_list.push_back((*it)->get_child());
          if(opts_->default_continuity > CONT_NONE){
            maker.visit_node((*it), state->phase == IntervalPhase, false);
          }
        }
        if(opts_->default_continuity > CONT_NONE){
          continuity_map = maker.get_continuity_map();
        }
        for(constraints_t::const_iterator it = state->temporary_constraints.begin(); it != state->temporary_constraints.end(); it++){
          constraint_list.push_back(*it);
        }

        // 制約を追加し，制約ストアが矛盾をおこしていないかどうか調べる

        solver_->add_constraint(constraint_list);

        add_continuity(continuity_map);


        {
          SymbolicVirtualConstraintSolver::check_consistency_result_t check_consistency_result = solver_->check_consistency();
          if(check_consistency_result.true_parameter_maps.empty()){
            // 必ず矛盾する場合
            return CalculateClosureResult();
          }else if (check_consistency_result.false_parameter_maps.empty()){
            // 必ず充足可能な場合
            // 何もしない
          }else{
            // 記号定数の条件によって充足可能性が変化する場合
            HYDLA_LOGGER_CLOSURE("%% consistency depends on conditions of parameters\n");
            CalculateClosureResult result;
            // 自分以外の場合は別に作ってスタックに入れる
            push_branch_states(state, check_consistency_result, result);
            return result;
          }
        }

        // ask制約を集める
        ask_collector.collect_ask(&expanded_always, 
            &positive_asks, 
            &negative_asks);

        // ask制約のエンテール処理
        HYDLA_LOGGER_CLOSURE("%% SymbolicSimulator::check_entailment in calculate_closure\n");

        //デフォルト連続性の処理

        bool strong_continuity = (opts_->default_continuity >= CONT_STRONG ||
            (opts_->default_continuity == CONT_STRONG_IP && state->phase == IntervalPhase));
        if(strong_continuity){
          for(continuity_map_t::const_iterator it  = variable_derivative_map_.begin(); it!=variable_derivative_map_.end(); ++it) {
            continuity_map_t::iterator find_result = continuity_map.find(it->first);
            if( find_result == continuity_map.end()){
              continuity_map.insert(std::make_pair(it->first, -(it->second)));
            }
          }
        }
        if(opts_->default_continuity != CONT_GUARD){
          add_continuity(continuity_map);
        }

        {
          expanded = false;
          branched_ask=NULL;
          negative_asks_t::iterator it  = negative_asks.begin();
          negative_asks_t::iterator end = negative_asks.end();
          while(it!=end) {
            solver_->start_temporary();
            if(opts_->default_continuity == CONT_GUARD){
              maker.visit_node((*it)->get_child(), state->phase == IntervalPhase, true);
              add_continuity(maker.get_continuity_map());
            }
            solver_->add_guard((*it)->get_guard());

            SymbolicVirtualConstraintSolver::check_consistency_result_t check_consistency_result = solver_->check_consistency();
            if(!check_consistency_result.true_parameter_maps.empty()){
              HYDLA_LOGGER_CLOSURE("%% entailable");
              if(!check_consistency_result.false_parameter_maps.empty()){
                HYDLA_LOGGER_CLOSURE("%% entailablity depends on conditions of parameters\n");
                // 自分以外の場合は別に作ってスタックに入れる
                CalculateClosureResult result;
                push_branch_states(state, check_consistency_result, result);
                // 自分は導出可能な場合のうちの1つとする
                return result;
              }
              solver_->end_temporary();
              solver_->start_temporary();
              add_continuity(maker.get_continuity_map());
              solver_->add_guard(node_sptr(new Not((*it)->get_guard())));
              check_consistency_result = solver_->check_consistency();
              if(!check_consistency_result.true_parameter_maps.empty()){
                HYDLA_LOGGER_CLOSURE("%% entailment branches");
                // ガード条件による分岐が発生する可能性あり
                if(!check_consistency_result.false_parameter_maps.empty()){
                  HYDLA_LOGGER_CLOSURE("%% inevitable entailment depends on conditions of parameters");
                  CalculateClosureResult ret;
                  push_branch_states(state, check_consistency_result, ret);
                  return ret;
                }
                HYDLA_LOGGER_CLOSURE("--- branched ask ---\n", *((*it)->get_guard()));
                branched_ask = &(*it);
                it++;
                solver_->end_temporary();
                continue;
              }
              HYDLA_LOGGER_CLOSURE("--- entailed ask ---\n", *((*it)->get_guard()));
              positive_asks.insert(*it);
              //eraseと後置インクリメントは同時にやらないとイテレータが壊れるので，注意
              negative_asks.erase(it++);
              expanded = true;
            }else{
              it++;
            }

            if(opts_->default_continuity == CONT_GUARD){
              maker.set_continuity_map(continuity_map);
            }
            solver_->end_temporary();
          }
        }
      }while(expanded);

      if(opts_->default_continuity == CONT_GUARD){
        add_continuity(continuity_map);
      }

      if(branched_ask!=NULL){
        HYDLA_LOGGER_CLOSURE("%% branched_ask:", **branched_ask);
        CalculateClosureResult result;
        {
          // 分岐先を生成（導出されない方）
          phase_state_sptr new_state(create_new_phase_state(state));
          new_state->temporary_constraints.push_back(node_sptr(new Not((*branched_ask)->get_guard())));
          result.push_back(new_state);
        }
        {
          // 分岐先を生成（導出される方）
          phase_state_sptr new_state(create_new_phase_state(state));
          new_state->temporary_constraints.push_back((*branched_ask)->get_guard());
          result.push_back(new_state);
        }
        return result;
      }

      if(opts_->assertion){
        HYDLA_LOGGER_CLOSURE("%% SymbolicSimulator::check_assertion");
        solver_->start_temporary();
        solver_->add_constraint(node_sptr(new Not(opts_->assertion)));
        SymbolicVirtualConstraintSolver::check_consistency_result_t result = solver_->check_consistency();
        solver_->end_temporary();
        if(!result.true_parameter_maps.empty()){
          if(!result.false_parameter_maps.empty()){
            // TODO:assertion失敗なので，本来ならやり直す必要はない．trueがassertion failedで，falseはこのまま続行すべき
            // ガード条件による分岐が発生する可能性あり
            HYDLA_LOGGER_CLOSURE("%% failure of assertion depends on conditions of parameters");
            CalculateClosureResult ret;
            push_branch_states(state, result, ret);
            return ret;
          }else{
            std::cout << "Assertion Failed!" << std::endl;
            is_safe_ = false;
            CalculateClosureResult ret;
            ret.push_back(state);
            return ret;
          }
        }
      }
      HYDLA_LOGGER_CLOSURE("#*** End SymbolicSimulator::calculate_closure ***\n");
      return CalculateClosureResult(1, state);
    }

    SymbolicSimulator::Phases SymbolicSimulator::point_phase(const module_set_sptr& ms, 
        phase_state_sptr& state, bool& consistent)
    {
      HYDLA_LOGGER_PHASE("#*** Begin SymbolicSimulator::point_phase***");
      //前準備
      Timer pp_timer;
      expanded_always_t expanded_always;
      expanded_always_id2sptr(state->expanded_always_id, expanded_always);
      solver_->change_mode(DiscreteMode, opts_->approx_precision);

      positive_asks_t positive_asks(state->positive_asks);
      negative_asks_t negative_asks;

      variable_map_t time_applied_map;
      //TODO:毎回時間適用してるけど，前のフェーズが決まってれば同じになるわけだから1フェーズにつき一回でいいはず．
      solver_->apply_time_to_vm(state->parent->variable_map, time_applied_map, state->current_time);
      HYDLA_LOGGER_PHASE("--- time_applied_variable_map ---\n", time_applied_map);
      solver_->reset(time_applied_map, state->parameter_map);

      Timer pp_cc_timer;

      //閉包計算
      CalculateClosureResult result = calculate_closure(state,ms,expanded_always,positive_asks,negative_asks);
      
      if(result.size() != 1){
        consistent = false;
        return result;
      }
      
      
      if(opts_->time_measurement){
      	pp_cc_timer.push_time("PP-CalculateClosure");
      }
      // Interval Phaseへ移行（次状態の生成）
      HYDLA_LOGGER_PHASE("%% SymbolicSimulator::create new phase states\n");  
      SymbolicVirtualConstraintSolver::create_result_t create_result = solver_->create_maps();

      assert(create_result.result_maps.size()>0);


      phase_state_sptr new_state_original(create_new_phase_state());
      new_state_original->step         = state->step;
      new_state_original->phase        = IntervalPhase;
      new_state_original->current_time = state->current_time;
      expanded_always_sptr2id(expanded_always, new_state_original->expanded_always_id);

      Phases phases;
      

      for(unsigned int create_it = 0; create_it < create_result.result_maps.size(); create_it++)
      {
        phase_state_sptr new_state(create_new_phase_state(new_state_original)), branch_state(create_new_phase_state(state));
        
        branch_state->variable_map = range_map_to_value_map(branch_state, create_result.result_maps[create_it], branch_state->parameter_map);
        new_state->parameter_map = branch_state->parameter_map;
        
        branch_state->parent->children.push_back(branch_state);
        new_state->parent = branch_state;

        if(is_safe_){
          phases.push_back(new_state);
        }else{
          branch_state->cause_of_termination = simulator::ASSERTION;
        }
        
      }

      if(opts_->time_measurement){
	      pp_timer.push_time("PointPhase");
      }
      HYDLA_LOGGER_PHASE("#*** End SymbolicSimulator::point_phase***\n");
      consistent = true;
      return phases;
    }

    SymbolicSimulator::Phases SymbolicSimulator::interval_phase(const module_set_sptr& ms, 
        phase_state_sptr& state, bool& consistent)
    {
      HYDLA_LOGGER_PHASE("#*** Begin SymbolicSimulator::interval_phase***");
      //前準備
      Timer ip_timer;
      expanded_always_t expanded_always;
      expanded_always_id2sptr(state->expanded_always_id, expanded_always);

      solver_->change_mode(ContinuousMode, opts_->approx_precision);
      negative_asks_t negative_asks;
      positive_asks_t positive_asks(state->positive_asks);
      solver_->reset(state->parent->variable_map, state->parameter_map);

      Timer ip_cc_timer;

      //閉包計算
      CalculateClosureResult result = calculate_closure(state,ms,expanded_always,positive_asks,negative_asks);

      if(result.size() != 1){
        consistent = false;
        return result;
      }

      if(opts_->time_measurement){
	      ip_cc_timer.push_time("IP-CalculateClosure");
      }


      SymbolicVirtualConstraintSolver::create_result_t create_result = solver_->create_maps();
      SymbolicVirtualConstraintSolver::create_result_t::result_maps_t& results = create_result.result_maps;

      if(results.size() > 1){
        // 区分的に連続で無い解軌道を含む．中断．
        phase_state_sptr phase(create_new_phase_state(state));
        phase->cause_of_termination = simulator::NOT_UNIQUE_IN_INTERVAL;
        phase->parent->children.push_back(phase);
        consistent = true;
        return Phases();
      }

      phase_state_sptr new_state_original(create_new_phase_state());

      new_state_original->step         = state->step+1;
      new_state_original->phase        = PointPhase;
      
      // TODO: expanded_alwaysが大丈夫かどうか考える．変換が必要なのはなぜ？
      // expanded_always_sptr2id(expanded_always, new_state_original->expanded_always_id);
      state->variable_map = range_map_to_value_map(state, results[0], state->parameter_map);
      state->variable_map = shift_variable_map_time(state->variable_map, state->current_time);
      
      Phases phases;

      if(is_safe_){

        /*
        // MaxModuleの導出
        module_set_sptr max_module_set = (*msc_no_init_).get_max_module_set();
        HYDLA_LOGGER_DEBUG("#** interval_phase: ms: **\n",
         *ms,
         "\n#** interval_phase: max_module_set: ##\n",
         *max_module_set);


        // 採用していないモジュールのリスト導出
        hydla::ch::ModuleSet::module_list_t diff_module_list(max_module_set->size() - ms->size());

        std::set_difference(
        max_module_set->begin(),
        max_module_set->end(),
        ms->begin(),
        ms->end(),
        diff_module_list.begin());


        // それぞれのモジュールをsingletonなモジュール集合とする
        std::vector<module_set_sptr> diff_module_set_list;

        hydla::ch::ModuleSet::module_list_const_iterator diff_it = diff_module_list.begin();
        hydla::ch::ModuleSet::module_list_const_iterator diff_end = diff_module_list.end();
        for(; diff_it!=diff_end; ++diff_it){
        module_set_sptr diff_ms(new ModuleSet((*diff_it).first, (*diff_it).second));
        diff_module_set_list.push_back(diff_ms);
        }

        assert(diff_module_list.size() == diff_module_set_list.size());


        // diff_module_set_list内の各モジュール集合内にある条件なし制約をそれぞれ得る
        not_adopted_tells_list_t not_adopted_tells_list;

        std::vector<module_set_sptr>::const_iterator diff_ms_list_it = diff_module_set_list.begin();
        std::vector<module_set_sptr>::const_iterator diff_ms_list_end = diff_module_set_list.end();
        for(; diff_ms_list_it!=diff_ms_list_end; ++diff_ms_list_it){
        TellCollector not_adopted_tells_collector(*diff_ms_list_it);
        tells_t       not_adopted_tells;
        not_adopted_tells_collector.collect_all_tells(&not_adopted_tells,
        &expanded_always, 
        &positive_asks);
        not_adopted_tells_list.push_back(not_adopted_tells);
        }


        //現在採用されていない制約を離散変化条件として追加
        for(not_adopted_tells_list_t::const_iterator it = not_adopted_tells_list.begin(); it != not_adopted_tells_list.end(); it++){
        tells_t::const_iterator na_it = it -> begin();
        tells_t::const_iterator na_end = it -> end();
        for(; na_it != na_end; na_it++){
        disc_cause.push_back((*na_it)->get_child());
        }
        }
        */

        constraints_t disc_cause;
        //現在導出されているガード条件にNotをつけたものを離散変化条件として追加
        for(positive_asks_t::const_iterator it = positive_asks.begin(); it != positive_asks.end(); it++){
          disc_cause.push_back(node_sptr(new Not((*it)->get_guard() ) ) );
        }
        //現在導出されていないガード条件を離散変化条件として追加
        for(negative_asks_t::const_iterator it = negative_asks.begin(); it != negative_asks.end(); it++){
          disc_cause.push_back((*it)->get_guard());
        }

        //assertionの否定を追加
        if(opts_->assertion){
          disc_cause.push_back(node_sptr(new Not(opts_->assertion)));
        }

        SymbolicVirtualConstraintSolver::PPTimeResult time_result = solver_->calculate_next_PP_time(disc_cause,
            state->current_time,time_t(node_sptr(new hydla::parse_tree::Number(opts_->max_time))));

        for(unsigned int time_it=0; time_it<time_result.candidates.size(); time_it++){
          phase_state_sptr branch_state(create_new_phase_state(state));
          branch_state->parameter_map = time_result.candidates[time_it].parameter_map;
          branch_state->parent->children.push_back(branch_state);
          branch_state->end_time = time_result.candidates[time_it].time;
          phase_state_sptr new_state(create_new_phase_state(new_state_original));
          
          if(!time_result.candidates[time_it].is_max_time ) {
            new_state->current_time = time_result.candidates[time_it].time;
            solver_->simplify(new_state->current_time);
            new_state->parameter_map = branch_state->parameter_map;
            new_state->parent = branch_state;
            phases.push_back(new_state);
          }else{
            branch_state->cause_of_termination = simulator::TIME_LIMIT;
          }
        }
      }else{
        state->parent->children.push_back(state);
        state->cause_of_termination = simulator::ASSERTION;
      }

      if(opts_->time_measurement){
	      ip_timer.push_time("IntervalPhase");
      }
      HYDLA_LOGGER_PHASE("#*** End SymbolicSimulator::interval_phase***");
      consistent = true;
      return phases;
  }

  SymbolicSimulator::variable_map_t SymbolicSimulator::range_map_to_value_map(
      const phase_state_sptr& state,
      const hydla::vcs::SymbolicVirtualConstraintSolver::variable_range_map_t& rm,
      parameter_map_t &parameter_map){
    variable_map_t ret = *variable_map_;
    for(vcs::SymbolicVirtualConstraintSolver::variable_range_map_t::const_iterator r_it = rm.begin(); r_it != rm.end(); r_it++){
      variable_t* variable = get_variable(r_it->first->get_name(), r_it->first->get_derivative_count());
      if(r_it->second.is_unique()){
        ret.set_variable(variable, r_it->second.get_lower_bound().value);
      }else{
        parameter_t param(r_it->first, state);
        parameter_set_->push_front(param);
        parameter_map.set_variable(&(parameter_set_->front()), r_it->second);
        ret.set_variable(variable, value_t(node_sptr(new Parameter(variable->get_name(), variable->get_derivative_count(), state->id))));
        // TODO:記号定数導入後，各変数の数式に出現する変数を記号定数に置き換える
      }
    }
    return ret;
  }


  SymbolicSimulator::variable_map_t SymbolicSimulator::shift_variable_map_time(const variable_map_t& vm, const time_t &time){
    variable_map_t shifted_vm;
    variable_map_t::const_iterator it  = vm.begin();
    variable_map_t::const_iterator end = vm.end();
    for(; it!=end; ++it) {
      if(it->second.is_undefined())
        shifted_vm.set_variable(it->first, it->second);
      else
        shifted_vm.set_variable(it->first, solver_->shift_expr_time(it->second, time));
    }
    return shifted_vm;
  }



} //namespace symbolic_simulator
} //namespace hydla

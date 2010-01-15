#include "BPSimulator.h"
#include "EntailmentChecker.h"
#include "ConstraintStore.h"
#include "ConstraintStoreInterval.h"
#include "ConsistencyChecker.h"
#include "ConsistencyCheckerInterval.h"

#include "Logger.h"

#include "InitNodeRemover.h"
#include "AskDisjunctionSplitter.h"
#include "AskDisjunctionFormatter.h"
#include "DiscreteAskRemover.h"


#include <iostream>
#include <boost/foreach.hpp>

// constraint_hierarchy
#include "ModuleSet.h"
//#include "ModuleSetList.h"
#include "ModuleSetGraph.h"
#include "ModuleSetContainerCreator.h"

using namespace hydla::ch;
using namespace hydla::simulator;

namespace hydla {
namespace bp_simulator {

BPSimulator::BPSimulator(const Opts& opts) :
  opts_(opts)
{}

BPSimulator::~BPSimulator()
{}

/* ダンプ用 */
namespace {
  class NodeDump {
  public:
    template<typename T>
    void operator()(T& it) 
    {
      std::cout << *it << "\n";
    }
  };
}

void BPSimulator::do_initialize(const parse_tree_sptr& parse_tree)
{
  ModuleSetContainerCreator<ModuleSetGraph> mcc;

  {
    // askのformatのみ
    parse_tree_sptr pt_original(boost::make_shared<ParseTree>(*parse_tree));
    AskDisjunctionFormatter().format(pt_original.get());
    AskDisjunctionSplitter().split(pt_original.get());
    msc_original_ = mcc.create(pt_original);
  }

  {
    // initialな制約を除いた + ask format
    parse_tree_sptr pt_no_init(boost::make_shared<ParseTree>(*parse_tree));
    InitNodeRemover().apply(pt_no_init.get());
    AskDisjunctionFormatter().format(pt_no_init.get());
    AskDisjunctionSplitter().split(pt_no_init.get());
    msc_no_init_ = mcc.create(pt_no_init);
  }

  {
    // no initial + 離散変化askを除いた + ask format
    // 離散変化askとは…条件にprev変数のあるask
    parse_tree_sptr pt_no_init_discreteask(boost::make_shared<ParseTree>(*parse_tree));
    InitNodeRemover().apply(pt_no_init_discreteask.get());
    DiscreteAskRemover().apply(pt_no_init_discreteask.get());
    AskDisjunctionFormatter().format(pt_no_init_discreteask.get());
    AskDisjunctionSplitter().split(pt_no_init_discreteask.get());
    msc_no_init_discreteask_ = mcc.create(pt_no_init_discreteask);
  }

  // 積む
  phase_state_sptr state(create_new_phase_state());
  state->phase        = PointPhase;
  state->time         = BPTime();
  state->variable_map = variable_map_;
  state->module_set_container = msc_original_;

  push_phase_state(state);

  // MathLink初期化
  //TODO: 例外を投げるようにする
  if(!ml_.init(opts_.mathlink.c_str())) {
    std::cerr << "can not link" << std::endl;
    exit(-1);
  }

}

/**
 * Point Phaseの処理
 * TODO: どっかにフレーム公理な処理を入れる！
 */
bool BPSimulator::point_phase(const module_set_sptr& ms, 
                              const phase_state_const_sptr& state)
{
  positive_asks_t positive_asks;
  negative_asks_t negative_asks;
  ConstraintStore constraint_store;
  // TODO: stateから制約ストアを作る
  constraint_store.build(state->variable_map);
  HYDLA_LOGGER_DEBUG(constraint_store);
  TellCollector tell_collector(ms);

  return this->do_point_phase(ms, state, constraint_store,
                              tell_collector, positive_asks, negative_asks);
}

/**
 * Point Phaseの実質的な処理
 * askのエンテールに基づき分割再帰される可能性がある．
 *
 * @param ms モジュール集合
 * @param state Point Phase開始時の状態
 * @param constraint_store 制約ストア
 * @param positive_asks エンテールされたask制約リスト
 * @param negative_asks エンテールされないask制約リスト
 *
 * @return Point Phaseを満たす解が存在するか
 */
bool BPSimulator::do_point_phase(const module_set_sptr& ms,
                    const phase_state_const_sptr& state,
                    ConstraintStore& constraint_store,
                    TellCollector& tell_collector,
                    positive_asks_t& positive_asks,
                    negative_asks_t& negative_asks)
{
  HYDLA_LOGGER_DEBUG("#** do_point_phase: BEGIN **\n");
  negative_asks_t unknown_asks; // Symbolicではnegative_asks
  AskCollector  ask_collector(ms);
  ConsistencyChecker consistency_checker;
  EntailmentChecker entailment_checker;

  //TODO: do_point_phaseの引数でpositive_asksみたいに引き回す必要あり
  expanded_always_t expanded_always;
  //expanded_always_id2sptr(state->expanded_always_id, expanded_always);

  bool expanded   = true;
  while(expanded) {
    // tell制約を集める
    tells_t tell_list;
    tell_collector.collect_new_tells(&tell_list,
                                     &expanded_always, &positive_asks);

    // 制約が充足しているかどうかの確認
    // 充足していればストアを更新
    if(!consistency_checker.is_consistent(tell_list,
                                          constraint_store)) return false;

    // ask制約を集める
    ask_collector.collect_ask(&expanded_always, 
                              &positive_asks, &unknown_asks);

    //ask制約のエンテール処理
    expanded = false;
    {
      negative_asks_t::iterator it  = unknown_asks.begin();
      negative_asks_t::iterator end = unknown_asks.end();
      while(it!=end) {
        if(negative_asks.find(*it)!=negative_asks.end()) {
          HYDLA_LOGGER_DEBUG("#*** do_point_phase: skip un-entailed ask ***");
          unknown_asks.erase(it++);
          continue;
        }
        Trivalent res = entailment_checker.check_entailment(*it, constraint_store);
        switch(res) {
          case Tri_FALSE:
            negative_asks.insert(*it);
            unknown_asks.erase(it++);
            break;
          case Tri_UNKNOWN:
            {
              // Ask条件に基づいて分岐
              // エンテールされる場合
              HYDLA_LOGGER_DEBUG("#*** do_point_phase: BRANCH ***\n");
              std::set<rp_constraint> guards, not_guards;
              var_name_map_t vars;
              GuardConstraintBuilder gcb;
              gcb.create_guard_expr(*it, guards, not_guards, vars);
              ConstraintStore cs_t(constraint_store);
              cs_t.add_constraint(guards.begin(), guards.end(), vars);
              TellCollector tc_t(tell_collector);
              positive_asks_t pa_t(positive_asks);
              pa_t.insert(*it);
              negative_asks_t na_t(negative_asks);
              bool result = this->do_point_phase(ms, state, cs_t, tc_t, pa_t, na_t);
              // エンテールされない場合
              std::set<rp_constraint>::iterator ctr_it;
              for(ctr_it=not_guards.begin(); ctr_it!=not_guards.end(); ctr_it++) {
                ConstraintStore cs_f(constraint_store);
                TellCollector tc_f(tell_collector);
                cs_f.add_constraint(*ctr_it, vars);
                positive_asks_t pa_f(positive_asks);
                negative_asks_t na_f(negative_asks);
                na_f.insert(*it);
                result = this->do_point_phase(ms, state, cs_f, tc_f, pa_f, na_f) || result;
              }
              return result;
            }
            assert(false);
            break;
          case Tri_TRUE:
            expanded = true;
            positive_asks.insert(*it);
            unknown_asks.erase(it++);
            break;
        }
      }
    }
  } // while(expanded)

  // IntervalPhaseへ
  phase_state_sptr new_state(create_new_phase_state(/*state*/));
  new_state->module_set_container = msc_no_init_discreteask_;
  new_state->phase = IntervalPhase;
  // ConstraintStoreからvariable_mapを作成
  constraint_store.build_variable_map(new_state->variable_map);
  //expanded_always_sptr2id(expanded_always, new_state->expanded_always_id);
  push_phase_state(new_state);

  std::cout << new_state->variable_map;

  return true;
}

/**
 * Interval Phaseの処理
 */
bool BPSimulator::interval_phase(const module_set_sptr& ms, 
                                 const phase_state_const_sptr& state)
{
  // 初期値はコピーしておく必要があるかも？
  TellCollector tell_collector(ms);
  AskCollector ask_collector(ms);
  ConstraintStoreInterval constraint_store;
  constraint_store.build(state->variable_map);

  HYDLA_LOGGER_DEBUG(constraint_store);

  ConsistencyCheckerInterval consistency_checker(this->ml_);
  tells_t tell_list;
  positive_asks_t positive_asks;
  negative_asks_t negative_asks;
  //TODO: do_point_phaseの引数でpositive_asksみたいに引き回す必要あり
  expanded_always_t expanded_always;
  //expanded_always_id2sptr(state->expanded_always_id, expanded_always);
  bool expanded = true;
  while(expanded) {
    // tell制約を集める
    tell_collector.collect_all_tells(&tell_list,
                                     &expanded_always, 
                                     &positive_asks);

    // 充足確認とストアへの追加, ODE求解
    if(!consistency_checker.is_consistent(tell_list, constraint_store)) {
      return false;
    }

    // ask制約を集める
    ask_collector.collect_ask(&expanded_always,
                              &positive_asks,
                              &negative_asks);

    expanded = false;
    {
      // ask条件のエンテール判定
      // IPでask条件にprevは入ってない => たぶんUNKNOWNにはならない…
      // ただし，数学的にT/FなのにUになるものがあるので対策
      // for all unknown_asks do
      //   Trivalent res = EntailmentCheckerInterval.check_entailment(ask, constraint_store);
      //   if(res==Tri_TRUE) positive_asks + ask
      //   if(res==Tri_FALSE) negative_asks + ask
      //   if(res==Tri_UNKNOWN) assert(false)かなぁ？
    }
  }

  // (list({t, vm}),list({ask,p?n})) = 積分処理(integrate)
  // integrate(cs, p_a, n_a, time, max_time);
  // time, vm, c_askからphase_stateを作る
  // 積む
  // ↓正確にはfor文で囲むと思われる
  /*phase_state_sptr new_state(create_new_phase_state(state));
  new_state->phase        = phase_state_t::PointPhase;
  new_state->initial_time = false;
  new_state->variable_map = vm;
  push_phase_state(new_state);*/

  return true;
}

/**
 * Interval Phaseの実質的な処理
 * askのエンテールに基づき分割再帰される可能性がある．
 *
 * @param ms モジュール集合
 * @param state Point Phase開始時の状態
 * @param constraint_store 制約ストア
 * @param positive_asks エンテールされたask制約リスト
 * @param negative_asks エンテールされないask制約リスト
 *
 * @return Interval Phaseを満たす解が存在するか
 */
bool BPSimulator::do_interval_phase(const module_set_sptr &ms,
                                    const phase_state_const_sptr &state,
                                    ConstraintStoreInterval &constraitn_store,
                                    TellCollector &tell_collector,
                                    positive_asks_t &positive_asks,
                                    negative_asks_t &negative_asks)
{
  return true;
}

} // bp_simulator
} // hydla

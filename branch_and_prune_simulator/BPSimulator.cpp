#include "BPSimulator.h"
#include "ConstraintStore.h"
#include "ConsistencyChecker.h"
#include "EntailmentChecker.h"

#include <iostream>
#include <boost/foreach.hpp>

// constraint_hierarchy
#include "ModuleSet.h"

using namespace hydla::simulator;

namespace hydla {
namespace bp_simulator {

BPSimulator::BPSimulator(const Opts& opts) :
  simulator_t(opts.debug_mode),
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

void BPSimulator::do_initialize()
{}

/**
 * Point Phaseの処理
 */
bool BPSimulator::point_phase(const module_set_sptr& ms, 
                              const phase_state_const_sptr& state)
{
  positive_asks_t positive_asks;
  ConstraintStore constraint_store(is_debug_mode());
  // TODO: stateから制約ストアを作る
  constraint_store.build(state->variable_map);

  return this->do_point_phase(ms, state, constraint_store, positive_asks);
}

/**
 * Point Phaseの実質的な処理
 * askのエンテールに基づき分割再帰される可能性がある．
 *
 * @param ms モジュール集合
 * @param state Point Phase開始時の状態
 * @param constraint_store 制約ストア
 * @param positive_asks エンテールされたask制約リスト
 *
 * @return Point Phaseを満たす解が存在するか
 */
bool BPSimulator::do_point_phase(const module_set_sptr& ms,
                    const phase_state_const_sptr& state,
                    ConstraintStore& constraint_store,
                    positive_asks_t& positive_asks)
{
  negative_asks_t unknown_asks; // Symbolicではnegative_asks
  TellCollector tell_collector(ms, is_debug_mode());
  AskCollector  ask_collector(ms, is_debug_mode());
  ConsistencyChecker consistency_checker(is_debug_mode());
  EntailmentChecker entailment_checker(is_debug_mode());

  bool expanded   = true;
  while(expanded) {
    // tell制約を集める
    tells_t tell_list;
    tell_collector.collect_new_tells(&tell_list,
                                     &state->expanded_always, &positive_asks);

    // 制約が充足しているかどうかの確認
    // 充足していればストアを更新
    if(!consistency_checker.is_consistent(tell_list,
                                          constraint_store)) return false;

    // ask制約を集める
    ask_collector.collect_ask(&state->expanded_always, 
                              &positive_asks, &unknown_asks);

    //ask制約のエンテール処理
    expanded = false;
    {
      negative_asks_t::iterator it  = unknown_asks.begin();
      negative_asks_t::iterator end = unknown_asks.end();
      while(it!=end) {
        Trivalent res = entailment_checker.check_entailment(*it, constraint_store);
        switch(res) {
          case TRUE:
            expanded = true;
            positive_asks.insert(*it);
            unknown_asks.erase(it++);
            break;
          case UNKNOWN:
            // UNKNOWNの処理はexpandが全て終わってから
            it++;
            break;
          case FALSE:
            // FALSEになったaskは以降エンテールされることはないので消す
            unknown_asks.erase(it++);
            break;
        }
      }
    }
  } // while(expanded)
  // negative_asksがtrueの場合とfalseの場合全ての組み合わせを考え分割

  // unknown_asksがあれば

  // ?
  //if(!csbp.build_constraint_store(&new_tells, &state->constraint_store)) {
  //  return false;
  //}

  //// IntervalPhaseへ
  //state->phase = IntervalPhase;
  //state_queue_.push(*state);

  return true;
}

/**
 * Interval Phaseの処理
 */
bool BPSimulator::interval_phase(const module_set_sptr& ms, 
                                 const phase_state_const_sptr& state)
{
  return true;
}

} // bp_simulator
} // hydla

#include "BPSimulator.h"
#include "GuardLister.h"
//#include "EntailmentChecker.h"
//#include "ConstraintStore.h"
//#include "ConstraintStoreInterval.h"
//#include "ConsistencyChecker.h"
//#include "ConsistencyCheckerInterval.h"

#include "Logger.h"

#include "InitNodeRemover.h"
#include "AskDisjunctionSplitter.h"
#include "AskDisjunctionFormatter.h"
#include "DiscreteAskRemover.h"
#include "AskTypeAnalyzer.h"
#include "TypedAsk.h"

#include "../virtual_constraint_solver/realpaver/RealPaverVCS.h"
#include "../virtual_constraint_solver/mathematica/vcs_math_source.h"


#include <iostream>

// constraint_hierarchy
#include "ModuleSet.h"
//#include "ModuleSetList.h"
#include "ModuleSetGraph.h"
#include "ModuleSetContainerCreator.h"

using namespace hydla::ch;
using namespace hydla::simulator;

using namespace hydla::parse_tree;

using namespace hydla::vcs::realpaver;

typedef std::set<hydla::parse_tree::node_sptr> node_list_t;

namespace hydla {
namespace bp_simulator {

BPSimulator::BPSimulator(const Opts& opts) :
  precision_(0.5),
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
    AskTypeAnalyzer().analyze(pt_original.get());
    AskDisjunctionFormatter().format(pt_original.get());
    AskDisjunctionSplitter().split(pt_original.get());
    msc_original_ = mcc.create(pt_original);
    HYDLA_LOGGER_DEBUG("#* original module set * \n",*pt_original); // デバッグ
  }

  {
    // initialな制約を除いた + ask format
    parse_tree_sptr pt_no_init(boost::make_shared<ParseTree>(*parse_tree));
    InitNodeRemover().apply(pt_no_init.get());
    AskTypeAnalyzer().analyze(pt_no_init.get());
    AskDisjunctionFormatter().format(pt_no_init.get());
    AskDisjunctionSplitter().split(pt_no_init.get());
    msc_no_init_ = mcc.create(pt_no_init);
    HYDLA_LOGGER_DEBUG("#* no init module set *\n", *pt_no_init); 
  }

  // TODO: 廃止
  //{
  //  // no initial + 離散変化askを除いた + ask format
  //  // 離散変化askとは…条件にprev変数のあるask
  //  parse_tree_sptr pt_no_init_discreteask(boost::make_shared<ParseTree>(*parse_tree));
  //  InitNodeRemover().apply(pt_no_init_discreteask.get());
  //  DiscreteAskRemover().apply(pt_no_init_discreteask.get());
  //  AskDisjunctionFormatter().format(pt_no_init_discreteask.get());
  //  AskDisjunctionSplitter().split(pt_no_init_discreteask.get());
  //  msc_no_init_discreteask_ = mcc.create(pt_no_init_discreteask);
  //}

  // 積む
  phase_state_sptr state(create_new_phase_state());
  state->phase        = PointPhase;
  state->current_time = bp_time_t();
  state->variable_map = variable_map_;
  state->module_set_container = msc_original_;

  push_phase_state(state);

  // MathLink初期化
  //TODO: 例外を投げるようにする
  if(!ml_.init(opts_.mathlink.c_str())) {
    std::cerr << "can not link" << std::endl;
    exit(-1);
  }
  // HydLa.mの内容送信
  //   ml_.MLPutFunction("Get", 1);
  //   ml_.MLPutString("symbolic_simulator/HydLa.m");
  ml_.MLPutFunction("ToExpression", 1);
  ml_.MLPutString(vcs_math_source());  
  ml_.MLEndPacket();
  ml_.skip_pkt_until(RETURNPKT);
  ml_.MLNewPacket();
}

/**
 * Point Phaseの処理
 * TODO: どっかにフレーム公理な処理を入れる！
 */
bool BPSimulator::point_phase(const module_set_sptr& ms, 
                              const phase_state_const_sptr& state)
{
  expanded_always_t expanded_always;
  positive_asks_t positive_asks;
  negative_asks_t negative_asks;

  if(state->changed_asks.size() != 0) {
    HYDLA_LOGGER_DEBUG("#** point_phase: changed_ask_id: ",
      state->changed_asks.at(0).second,
      " **");
  }

  RealPaverVCS vcs(RealPaverVCS::DiscreteMode);
  vcs.set_precision(this->precision_);
  vcs.reset(state->variable_map);

  TellCollector tell_collector(ms);

  return this->do_point_phase(ms, state, vcs, tell_collector,
    expanded_always, positive_asks, negative_asks);
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
                    RealPaverVCS& vcs,
                    TellCollector& tell_collector,
                    expanded_always_t& expanded_always,
                    positive_asks_t& positive_asks,
                    negative_asks_t& negative_asks)
{
  HYDLA_LOGGER_DEBUG("#** do_point_phase: BEGIN **\n");
  negative_asks_t unknown_asks; // Symbolicではnegative_asks
  AskCollector  ask_collector(ms, AskCollector::ENABLE_COLLECT_NON_TYPED_ASK | 
    AskCollector::ENABLE_COLLECT_DISCRETE_ASK |
    AskCollector::ENABLE_COLLECT_CONTINUOUS_ASK);
  //ConsistencyChecker consistency_checker;
  //EntailmentChecker entailment_checker;

  //expanded_always_id2sptr(state->expanded_always_id, expanded_always);

  bool expanded   = true;
  while(expanded) {
    // tell制約を集める
    tells_t tell_list;
    tell_collector.collect_new_tells(&tell_list,
                                     &expanded_always, &positive_asks);

    // 制約が充足しているかどうかの確認
    // 充足していればストアを更新
    switch(vcs.add_constraint(tell_list)) {
    case hydla::vcs::VCSR_TRUE:
      break;
    case hydla::vcs::VCSR_FALSE:
      return false;
    case hydla::vcs::VCSR_UNKNOWN:
      assert(false);
      break;
    case hydla::vcs::VCSR_SOLVER_ERROR:
      assert(false); // TODO: 暫定的に
      break;
    }

    // ask制約を集める
    ask_collector.collect_ask(&expanded_always,
                              &positive_asks, &unknown_asks);

    //ask制約のエンテール処理
    expanded = false;
    {
      negative_asks_t::iterator it  = unknown_asks.begin();
      negative_asks_t::iterator end = unknown_asks.end();
      while(it!=end) {
        // 以前にエンテールされないと判断されたask制約はスキップ
        if(negative_asks.find(*it)!=negative_asks.end()) {
          HYDLA_LOGGER_DEBUG("#*** do_point_phase: skip un-entailed ask ***");
          unknown_asks.erase(it++);
          continue;
        }
        HYDLA_LOGGER_DEBUG("#*** do_point_phase: ask_node_id: ",
          (*it)->get_id(),
          " ***");
        if(state->changed_asks.size() != 0 &&
          (*it)->get_id() == state->changed_asks.at(0).second) {
            if(state->changed_asks.at(0).first == Negative2Positive) {
              HYDLA_LOGGER_DEBUG("#*** do_point_phase: previous changed ask TRUE ***");
              expanded = true;
              positive_asks.insert(*it);
              unknown_asks.erase(it++);
            } else {
              HYDLA_LOGGER_DEBUG("#*** do_point_phase: previous changed ask FALSE ***");
              negative_asks.insert(*it);
              unknown_asks.erase(it++);
            }
            continue;
        }
        switch(vcs.check_entailment(*it)) {
        case hydla::vcs::VCSR_TRUE:
          expanded = true;
          positive_asks.insert(*it);
          unknown_asks.erase(it++);
          break;
        case hydla::vcs::VCSR_FALSE:
          negative_asks.insert(*it);
          unknown_asks.erase(it++);
          break;
        case hydla::vcs::VCSR_UNKNOWN:
          {
            // Ask条件に基づいて分岐
            HYDLA_LOGGER_DEBUG("#*** do_point_phase: BRANCH ***\n");
            // エンテールされる場合
            HYDLA_LOGGER_DEBUG("#*** do_point_phase: TRUE CASE ***\n");
            // Askから式nodeのリストを得る
            GuardLister lister;
            node_list_t g_list = lister.get_guard_list(*it);
            RealPaverVCS vcs_t(vcs);
            for(node_list_t::iterator nit=g_list.begin(); nit!=g_list.end(); nit++) {
              // ガード条件をストア(VCS)に加える
              vcs_t.add_single_constraint(*nit);
            }
            TellCollector tc_t(tell_collector);
            expanded_always_t ea_t(expanded_always);
            positive_asks_t pa_t(positive_asks); pa_t.insert(*it);
            negative_asks_t na_t(negative_asks);
            bool result = this->do_point_phase(ms, state, vcs_t, tc_t, ea_t, pa_t, na_t);
            // エンテールされない場合
            for(node_list_t::iterator nit=g_list.begin(); nit!=g_list.end(); nit++) {
              HYDLA_LOGGER_DEBUG("#*** do_point_phase: FALSE CASE(S) ***\n");
              RealPaverVCS vcs_f(vcs);
              vcs_f.add_single_constraint(*nit, true);
              TellCollector tc_f(tell_collector);
              expanded_always_t ea_f(expanded_always);
              positive_asks_t pa_f(positive_asks);
              negative_asks_t na_f(negative_asks); na_f.insert(*it);
              result = this->do_point_phase(ms, state, vcs_f, tc_f, ea_f, pa_f, na_f) || result;
            }
            return result;
          }
          assert(false);
          break;
        case hydla::vcs::VCSR_SOLVER_ERROR:
          assert(false);
          break;
        }
      }
    }
  } // while(expanded)

  // IntervalPhaseへ
  phase_state_sptr new_state(create_new_phase_state(/*state*/));
  new_state->module_set_container = msc_no_init_;
  new_state->phase = IntervalPhase;
  new_state->current_time = state->current_time;
  // ConstraintStoreからvariable_mapを作成
  vcs.create_variable_map(new_state->variable_map);
  //expanded_always_sptr2id(expanded_always, new_state->expanded_always_id);
  push_phase_state(new_state);

  // ここはシミュレーション結果の出力として必要なんじゃない？
  //std::cout << new_state->variable_map;

  return true;
}

/**
 * Interval Phaseの処理
 */
bool BPSimulator::interval_phase(const module_set_sptr& ms, 
                                 const phase_state_const_sptr& state)
{
  HYDLA_LOGGER_DEBUG("#** interval_phase: BEGIN **\n");
  // 初期値はコピーしておく必要があるかも？
  expanded_always_t expanded_always;
  //expanded_always_id2sptr(state->expanded_always_id, expanded_always);
  positive_asks_t positive_asks;
  negative_asks_t negative_asks;
  RealPaverVCS vcs(RealPaverVCS::ContinuousMode, &ml_);
  vcs.set_precision(this->precision_);
  vcs.reset(state->variable_map);
  TellCollector tell_collector(ms);
  AskCollector ask_collector(ms);

  tells_t tell_list;

  bool expanded = true;
  while(expanded) {
    // tell制約を集める
    tell_collector.collect_all_tells(&tell_list,
                                     &expanded_always, 
                                     &positive_asks);

    // 充足確認とストアへの追加, ODE求解
    switch(vcs.add_constraint(tell_list)) {
    case hydla::vcs::VCSR_TRUE:
      break;
    case hydla::vcs::VCSR_FALSE:
      return false;
    case hydla::vcs::VCSR_UNKNOWN:
      assert(false);
      break;
    case hydla::vcs::VCSR_SOLVER_ERROR:
      assert(false);
      break;
    }

    // ask制約を集める
    ask_collector.collect_ask(&expanded_always,
                              &positive_asks,
                              &negative_asks);

    expanded = false;
    {
      // ask条件のエンテール判定
      // IPでask条件にprevは入ってない => たぶんUNKNOWNにはならない…
      // TODO: ただし，数学的にT/FなのにUになるものがあるので対策
      // IPでは連続を仮定 => 時刻0であるこのときは全ての変数の値が初期値変数と同じ
      negative_asks_t::iterator it  = negative_asks.begin();
      negative_asks_t::iterator end = negative_asks.end();
      while(it!=end) {
        // 離散変化askはIPではチェックしない
        if(boost::dynamic_pointer_cast<hydla::simulator::DiscreteAsk>(*it)) {
          it++;
        } else {
          switch(vcs.check_entailment(*it)) {
          case hydla::vcs::VCSR_TRUE:
            expanded = true;
            positive_asks.insert(*it);
            negative_asks.erase(it++);
            break;
         case hydla::vcs::VCSR_FALSE:
           it++;
           break;
         case hydla::vcs::VCSR_UNKNOWN:
           // IPでは起こらない(ようにする)
           assert(false);
           break;
         case hydla::vcs::VCSR_SOLVER_ERROR:
           assert(false);
           break;
          }
        }
      }
    }
  } // while

  virtual_constraint_solver_t::IntegrateResult integrate_result;
  bool all_proof = false, finish_simulate = false;
  while(!all_proof) {
    switch(vcs.integrate(integrate_result, positive_asks,
      negative_asks, state->current_time, bp_time_t(opts_.max_time))) {
    case hydla::vcs::VCSR_TRUE:
      // 全ての初期状態について次のポイントフェーズ時刻が見つかった
      all_proof = true;
      break;
    case hydla::vcs::VCSR_FALSE:
      // 次のポイントフェーズがもうなかった
      // resultのis_max_timeはtrue？
      finish_simulate = true;
      all_proof = true;
      break;
    case hydla::vcs::VCSR_UNKNOWN:
      // ポイントフェーズはあったが，全ての初期状態についてではなかった
      // TODO: 現状，全ての初期状態について引っかかるaskがないと大変なことになる
      if(integrate_result.changed_asks.at(0).first == Negative2Positive) {
        for(negative_asks_t::iterator nit=negative_asks.begin(); nit!=negative_asks.end(); ++nit) {
          if((*nit)->get_id() == integrate_result.changed_asks.at(0).second) {
            negative_asks.erase(nit);
            break;
          }
        }
      } else {
        for(positive_asks_t::iterator pit=positive_asks.begin(); pit!=positive_asks.end(); ++pit) {
          if((*pit)->get_id() == integrate_result.changed_asks.at(0).second) {
            positive_asks.erase(pit);
            break;
          }
        }
      }
      break;
    case hydla::vcs::VCSR_SOLVER_ERROR:
      assert(false);
      break;
    }
    // 積む処理はここでやる
    typedef virtual_constraint_solver_t::IntegrateResult::next_phase_state_list_t next_phase_state_list_t;
    if(!finish_simulate) {
      for(next_phase_state_list_t::const_iterator npit=integrate_result.states.begin();
        npit!=integrate_result.states.end(); ++npit) {
          phase_state_sptr new_state(create_new_phase_state());
          new_state->phase        = PointPhase;
          new_state->module_set_container = msc_no_init_;
          new_state->variable_map = (*npit).variable_map;
          new_state->current_time = (*npit).time;
          new_state->changed_asks = integrate_result.changed_asks;
          push_phase_state(new_state);
      }
    }
  } // while

  return true;
}

} // bp_simulator
} // hydla

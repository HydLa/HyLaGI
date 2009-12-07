#include "BPSimulator.h"
#include "ConsistencyChecker.h"
#include "EntailmentChecker.h"

#include <iostream>
#include <boost/foreach.hpp>

// constraint_hierarchy
#include "ModuleSet.h"

// simulator
#include "TellCollector.h"
#include "AskCollector.h"

using namespace hydla::simulator;

namespace hydla {
namespace bp_simulator {

  BPSimulator::BPSimulator(){}

  BPSimulator::~BPSimulator(){}

  /**
   * シミュレーションメイン
   */
  bool BPSimulator::simulate(boost::shared_ptr<hydla::ch::ModuleSetContainer> msc, Opts& opts)
  {
    this->debug_mode_ = opts.debug_mode;
    this->max_time_   = opts.max_time;

    // 初期stateをスタックに積む
    phase_state_sptr init_state_sptr(new phase_state_t);
    init_state_sptr->phase = phase_state_t::PointPhase;
    this->push_phase_state(init_state_sptr);

    // 自動的に時刻0のBPSimulator::point_phaseがはじまる
    simulator_t::simulate(msc);

    return true;
  }

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

  /**
   * Point Phaseの処理
   */
  bool BPSimulator::point_phase(hydla::ch::module_set_sptr& ms, phase_state_sptr& state)
  {
    if(debug_mode_) {
      std::cout << "#***** begin point phase *****\n";
      std::cout << "#** module set **\n";
      std::cout << ms->get_name() << std::endl;
      ms->dump_tree(std::cout) << std::endl;
    }

    TellCollector tell_collector(ms);
    AskCollector  ask_collector;
    //ConstraintStoreBuilderPoint csbp(ml_);       //TODO: kenshiroが作成
    ConsistencyChecker consistency_checker(this->debug_mode_);
    EntailmentChecker entailment_checker;   //TODO: kenshiroが作成

    TellCollector::tells_t tell_list;
    positive_asks_t   positive_asks;
    negative_asks_t   negative_asks;

    bool expanded   = true;
    while(expanded) {
      // tell制約を集める
      tell_collector.collect_all_tells(&tell_list, &state->expanded_always, &positive_asks);
      if(debug_mode_) {
        std::cout << "#** collected tells **\n";  
        std::for_each(tell_list.begin(), tell_list.end(), NodeDump());
      }

      // 制約が充足しているかどうかの確認
      if(!consistency_checker.is_consistent(tell_list)){
        if(debug_mode_) std::cout << "#*** inconsistent\n";
        return false;
      }
      if(debug_mode_) std::cout << "#*** consistent\n";

      // ask制約を集める
      ask_collector.collect_ask(ms.get(), &state->expanded_always, 
        &positive_asks, &negative_asks);
      if(debug_mode_) {
        std::cout << "#** positive asks **\n";  
        std::for_each(positive_asks.begin(), positive_asks.end(), NodeDump());

        std::cout << "#** negative asks **\n";  
        std::for_each(negative_asks.begin(), negative_asks.end(), NodeDump());
      }

      //ask制約のエンテール処理
      expanded = false;
      {
        negative_asks_t::iterator it  = negative_asks.begin();
        negative_asks_t::iterator end = negative_asks.end();
        while(it!=end) {
          // 他に値boxが必要では？
          Trivalent res = entailment_checker.check_entailment(*it, tell_list);
          switch(res) {
            case TRUE:
              expanded = true;
              positive_asks.insert(*it);
              negative_asks.erase(it++);
              break;
            case UNKNOWN:
              // phase_state_sptr state_include_true;
              // phase_state_sptr state_include_false;
              // それぞれ値boxを更新
              // positive_asksとnegative_asksを持ち越す必要がある
              // PointPhase(ms, state_include_true);
              // PointPhase(ms, state_include_false);
              // 後始末？
              return true;
            case FALSE:
              it++;
              break;
          }
        }
      }
    } // while(expanded)

    ////   if(!csbp.build_constraint_store(&new_tells, &state->constraint_store)) {
    ////     return false;
    ////   }

    ////  state->phase = IntervalPhase;
    ////state_queue_.push(*state);
    return true;
  }
  
  /**
   * Interval Phaseの処理
   */
  bool BPSimulator::interval_phase(hydla::ch::module_set_sptr& ms, phase_state_sptr& state)
  {
    return true;
  }

} // bp_simulator
} // hydla

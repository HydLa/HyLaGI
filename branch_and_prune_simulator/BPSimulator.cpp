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

BPSimulator::BPSimulator(const Opts& opts) :
  opts_(opts)
{}

BPSimulator::~BPSimulator()
{}

/* �_���v�p */
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
{
}

/**
 * Point Phase�̏���
 */
bool BPSimulator::point_phase(const module_set_sptr& ms, 
                              const phase_state_const_sptr& state)
{
  if(is_debug_mode()) {
    std::cout << "#***** begin point phase *****\n"
              << "#** module set **\n"
              << ms->get_name()
              << "\n"
              << *ms 
              << std::endl;
  }

  TellCollector tell_collector(ms);
  AskCollector  ask_collector(ms);
  //ConstraintStoreBuilderPoint csbp(ml_);       //TODO: kenshiro���쐬
  ConsistencyChecker consistency_checker(is_debug_mode());
  EntailmentChecker entailment_checker;   //TODO: kenshiro���쐬

  tells_t         tell_list;
  positive_asks_t positive_asks;
  negative_asks_t negative_asks;

  bool expanded   = true;
  while(expanded) {
    // tell������W�߂�
    tell_collector.collect_all_tells(&tell_list, &state->expanded_always, &positive_asks);
    if(is_debug_mode()) {
      std::cout << "#** collected tells **\n";  
      std::for_each(tell_list.begin(), tell_list.end(), NodeDump());
    }

    // ���񂪏[�����Ă��邩�ǂ����̊m�F
    if(!consistency_checker.is_consistent(tell_list)){
      if(is_debug_mode()) std::cout << "#*** inconsistent\n";
      return false;
    }
    if(is_debug_mode()) std::cout << "#*** consistent\n";

    // ask������W�߂�
    ask_collector.collect_ask(&state->expanded_always, 
                              &positive_asks, &negative_asks);
    if(is_debug_mode()) {
      std::cout << "#** positive asks **\n";  
      std::for_each(positive_asks.begin(), positive_asks.end(), NodeDump());

      std::cout << "#** negative asks **\n";  
      std::for_each(negative_asks.begin(), negative_asks.end(), NodeDump());
    }

    //ask����̃G���e�[������
    expanded = false;
    {
      negative_asks_t::iterator it  = negative_asks.begin();
      negative_asks_t::iterator end = negative_asks.end();
      while(it!=end) {
        // ���ɒlbox���K�v�ł́H
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
            // ���ꂼ��lbox���X�V
            // positive_asks��negative_asks�������z���K�v������
            // PointPhase(ms, state_include_true);
            // PointPhase(ms, state_include_false);
            // ��n���H
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
 * Interval Phase�̏���
 */
bool BPSimulator::interval_phase(const module_set_sptr& ms, 
                                 const phase_state_const_sptr& state)
{
  return true;
}

} // bp_simulator
} // hydla

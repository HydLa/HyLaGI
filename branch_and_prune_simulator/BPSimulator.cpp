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
{}

/**
 * Point Phase�̏���
 */
bool BPSimulator::point_phase(const module_set_sptr& ms, 
                              const phase_state_const_sptr& state)
{
  positive_asks_t positive_asks;
  ConstraintStore constraint_store(is_debug_mode());
  // TODO: state���琧��X�g�A�����
  constraint_store.build(state->variable_map);

  return this->do_point_phase(ms, state, constraint_store, positive_asks);
}

/**
 * Point Phase�̎����I�ȏ���
 * ask�̃G���e�[���Ɋ�Â������ċA�����\��������D
 *
 * @param ms ���W���[���W��
 * @param state Point Phase�J�n���̏��
 * @param constraint_store ����X�g�A
 * @param positive_asks �G���e�[�����ꂽask���񃊃X�g
 *
 * @return Point Phase�𖞂����������݂��邩
 */
bool BPSimulator::do_point_phase(const module_set_sptr& ms,
                    const phase_state_const_sptr& state,
                    ConstraintStore& constraint_store,
                    positive_asks_t& positive_asks)
{
  negative_asks_t unknown_asks; // Symbolic�ł�negative_asks
  TellCollector tell_collector(ms, is_debug_mode());
  AskCollector  ask_collector(ms, is_debug_mode());
  ConsistencyChecker consistency_checker(is_debug_mode());
  EntailmentChecker entailment_checker(is_debug_mode());

  bool expanded   = true;
  while(expanded) {
    // tell������W�߂�
    tells_t tell_list;
    tell_collector.collect_new_tells(&tell_list,
                                     &state->expanded_always, &positive_asks);

    // ���񂪏[�����Ă��邩�ǂ����̊m�F
    // �[�����Ă���΃X�g�A���X�V
    if(!consistency_checker.is_consistent(tell_list,
                                          constraint_store)) return false;

    // ask������W�߂�
    ask_collector.collect_ask(&state->expanded_always, 
                              &positive_asks, &unknown_asks);

    //ask����̃G���e�[������
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
            // UNKNOWN�̏�����expand���S�ďI����Ă���
            it++;
            break;
          case FALSE:
            // FALSE�ɂȂ���ask�͈ȍ~�G���e�[������邱�Ƃ͂Ȃ��̂ŏ���
            unknown_asks.erase(it++);
            break;
        }
      }
    }
  } // while(expanded)
  // negative_asks��true�̏ꍇ��false�̏ꍇ�S�Ă̑g�ݍ��킹���l������

  // unknown_asks�������

  // ?
  //if(!csbp.build_constraint_store(&new_tells, &state->constraint_store)) {
  //  return false;
  //}

  //// IntervalPhase��
  //state->phase = IntervalPhase;
  //state_queue_.push(*state);

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

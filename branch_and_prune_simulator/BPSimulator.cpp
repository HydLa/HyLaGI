#include "BPSimulator.h"
#include "ConstraintStoreBuilderPoint.h"
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
  // TODO: ���ʂȐ錾
  tells_t         tell_list;
  positive_asks_t positive_asks;
  negative_asks_t negative_asks;

  return this->do_point_phase(ms, state, tell_list, positive_asks, negative_asks);
}

/**
 * Point Phase�̎����I�ȏ���
 * ask�̃G���e�[���Ɋ�Â������ċA�����\��������D
 * ����X�g�A�͍Ō�܂ōX�V�����C�L����tell�����tell_list�ɕێ������D
 * �Ō�ɃX�g�A��tell_list�����킹�ĐV��Ԃ𐶐�����D
 *
 * @param ms ���W���[���W��
 * @param state Point Phase�J�n���̏��
 * @param tell_list tell���񃊃X�g
 * @param positive_asks �G���e�[�����ꂽask���񃊃X�g
 * @param negative_asks �G���e�[������Ă��Ȃ�ask���񃊃X�g
 *
 * @return Point Phase�𖞂����������݂��邩
 */
bool BPSimulator::do_point_phase(const module_set_sptr& ms,
                    const phase_state_const_sptr& state,
                    tells_t tell_list,
                    positive_asks_t positive_asks,
                    negative_asks_t negative_asks)
{
  TellCollector tell_collector(ms, is_debug_mode());
  AskCollector  ask_collector(ms, is_debug_mode());
  ConstraintStoreBuilderPoint csbp(is_debug_mode()); //TODO: kenshiro���쐬�H
  ConsistencyChecker consistency_checker(is_debug_mode());
  EntailmentChecker entailment_checker(is_debug_mode());   //TODO: kenshiro���쐬

  // TODO: state���琧��X�g�A�����
  csbp.build_constraint_store(state->variable_map);

  bool expanded   = true;
  while(expanded) {
    // tell������W�߂�
    tell_collector.collect_all_tells(&tell_list, &state->expanded_always, &positive_asks);

    // ���񂪏[�����Ă��邩�ǂ����̊m�F
    if(!consistency_checker.is_consistent(tell_list, csbp.getcs())){
      return false;
    }

    // ask������W�߂�
    ask_collector.collect_ask(&state->expanded_always, 
                              &positive_asks, &negative_asks);

    //ask����̃G���e�[������
    expanded = false;
    {
      negative_asks_t::iterator it  = negative_asks.begin();
      negative_asks_t::iterator end = negative_asks.end();
      while(it!=end) {
        Trivalent res = entailment_checker.check_entailment(*it, tell_list, csbp.getcs());
        switch(res) {
          case TRUE:
            expanded = true;
            positive_asks.insert(*it);
            negative_asks.erase(it++);
            break;
          case UNKNOWN:
            // UNKNOWN�̏�����expand���I����Ă���
            it++;
            break;
          case FALSE:
            // ask����
            negative_asks.erase(it++);
            break;
        }
      }
    }
  } // while(expanded)
  // negative_asks��true�̏ꍇ��false�̏ꍇ�S�Ă̑g�ݍ��킹���l������

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

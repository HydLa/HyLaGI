#include "BPSimulator.h"
#include "ConstraintStore.h"
#include "ConsistencyChecker.h"
#include "EntailmentChecker.h"

#include "Logger.h"

#include <iostream>
#include <boost/foreach.hpp>

// constraint_hierarchy
#include "ModuleSet.h"

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

void BPSimulator::do_initialize(const parse_tree_sptr& parse_tree)
{}

/**
 * Point Phase�̏���
 * TODO: �ǂ����Ƀt���[�������ȏ���������I
 */
bool BPSimulator::point_phase(const module_set_sptr& ms, 
                              const phase_state_const_sptr& state)
{
  positive_asks_t positive_asks;
  negative_asks_t negative_asks;
  ConstraintStore constraint_store;
  // TODO: state���琧��X�g�A�����
  constraint_store.build(state->variable_map);
  HYDLA_LOGGER_DEBUG(constraint_store);
  TellCollector tell_collector(ms);

  return this->do_point_phase(ms, state, constraint_store,
                              tell_collector, positive_asks, negative_asks);
}

/**
 * Point Phase�̎����I�ȏ���
 * ask�̃G���e�[���Ɋ�Â������ċA�����\��������D
 *
 * @param ms ���W���[���W��
 * @param state Point Phase�J�n���̏��
 * @param constraint_store ����X�g�A
 * @param positive_asks �G���e�[�����ꂽask���񃊃X�g
 * @param negative_asks �G���e�[������Ȃ�ask���񃊃X�g
 *
 * @return Point Phase�𖞂����������݂��邩
 */
bool BPSimulator::do_point_phase(const module_set_sptr& ms,
                    const phase_state_const_sptr& state,
                    ConstraintStore& constraint_store,
                    TellCollector& tell_collector,
                    positive_asks_t& positive_asks,
                    negative_asks_t& negative_asks)
{
  HYDLA_LOGGER_DEBUG("#** do_point_phase: BEGIN **\n");
  negative_asks_t unknown_asks; // Symbolic�ł�negative_asks
  AskCollector  ask_collector(ms);
  ConsistencyChecker consistency_checker;
  EntailmentChecker entailment_checker;

  //TODO: do_point_phase�̈�����positive_asks�݂����Ɉ����񂷕K�v����
  expanded_always_t expanded_always;
  expanded_always_id2sptr(state->expanded_always_id, expanded_always);

  bool expanded   = true;
  while(expanded) {
    // tell������W�߂�
    tells_t tell_list;
    tell_collector.collect_new_tells(&tell_list,
                                     &expanded_always, &positive_asks);

    // ���񂪏[�����Ă��邩�ǂ����̊m�F
    // �[�����Ă���΃X�g�A���X�V
    if(!consistency_checker.is_consistent(tell_list,
                                          constraint_store)) return false;

    // ask������W�߂�
    ask_collector.collect_ask(&expanded_always, 
                              &positive_asks, &unknown_asks);

    //ask����̃G���e�[������
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
          case TRUE:
            expanded = true;
            positive_asks.insert(*it);
            unknown_asks.erase(it++);
            break;
          case UNKNOWN:
            {
              // Ask�����Ɋ�Â��ĕ���
              // �G���e�[�������ꍇ
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
              // �G���e�[������Ȃ��ꍇ
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
          case FALSE:
            negative_asks.insert(*it);
            unknown_asks.erase(it++);
            break;
        }
      }
    }
  } // while(expanded)

  // IntervalPhase��
  phase_state_sptr new_state(create_new_phase_state(state));
  new_state->phase = IntervalPhase;
  // ConstraintStore����variable_map���쐬
  constraint_store.build_variable_map(new_state->variable_map);
  expanded_always_sptr2id(expanded_always, new_state->expanded_always_id);
  push_phase_state(new_state);

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

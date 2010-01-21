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

#include "../virtual_constraint_solver/realpaver/RealPaverVCS.h"

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
{
  ModuleSetContainerCreator<ModuleSetGraph> mcc;

  {
    // ask��format�̂�
    parse_tree_sptr pt_original(boost::make_shared<ParseTree>(*parse_tree));
    AskDisjunctionFormatter().format(pt_original.get());
    AskDisjunctionSplitter().split(pt_original.get());
    msc_original_ = mcc.create(pt_original);
  }

  {
    // initial�Ȑ���������� + ask format
    parse_tree_sptr pt_no_init(boost::make_shared<ParseTree>(*parse_tree));
    InitNodeRemover().apply(pt_no_init.get());
    AskDisjunctionFormatter().format(pt_no_init.get());
    AskDisjunctionSplitter().split(pt_no_init.get());
    msc_no_init_ = mcc.create(pt_no_init);
  }

  {
    // no initial + ���U�ω�ask�������� + ask format
    // ���U�ω�ask�Ƃ́c������prev�ϐ��̂���ask
    parse_tree_sptr pt_no_init_discreteask(boost::make_shared<ParseTree>(*parse_tree));
    InitNodeRemover().apply(pt_no_init_discreteask.get());
    DiscreteAskRemover().apply(pt_no_init_discreteask.get());
    AskDisjunctionFormatter().format(pt_no_init_discreteask.get());
    AskDisjunctionSplitter().split(pt_no_init_discreteask.get());
    msc_no_init_discreteask_ = mcc.create(pt_no_init_discreteask);
  }

  // �ς�
  phase_state_sptr state(create_new_phase_state());
  state->phase        = PointPhase;
  state->current_time = bp_time_t();
  state->variable_map = variable_map_;
  state->module_set_container = msc_original_;

  push_phase_state(state);

  // MathLink������
  //TODO: ��O�𓊂���悤�ɂ���
  if(!ml_.init(opts_.mathlink.c_str())) {
    std::cerr << "can not link" << std::endl;
    exit(-1);
  }

}

/**
 * Point Phase�̏���
 * TODO: �ǂ����Ƀt���[�������ȏ���������I
 */
bool BPSimulator::point_phase(const module_set_sptr& ms, 
                              const phase_state_const_sptr& state)
{
  expanded_always_t expanded_always;
  positive_asks_t positive_asks;
  negative_asks_t negative_asks;
  //ConstraintStore constraint_store;
  //// TODO: state���琧��X�g�A�����
  //constraint_store.build(state->variable_map);
  RealPaverVCS vcs(RealPaverVCS::DiscreteMode);
  vcs.reset(state->variable_map);

  TellCollector tell_collector(ms);

  return this->do_point_phase(ms, state, vcs, tell_collector,
    expanded_always, positive_asks, negative_asks);
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
                    RealPaverVCS& vcs,
                    TellCollector& tell_collector,
                    expanded_always_t& expanded_always,
                    positive_asks_t& positive_asks,
                    negative_asks_t& negative_asks)
{
  HYDLA_LOGGER_DEBUG("#** do_point_phase: BEGIN **\n");
  negative_asks_t unknown_asks; // Symbolic�ł�negative_asks
  AskCollector  ask_collector(ms, AskCollector::ENABLE_COLLECT_NON_TYPED_ASK | 
    AskCollector::ENABLE_COLLECT_DISCRETE_ASK |
    AskCollector::ENABLE_COLLECT_CONTINUOUS_ASK);
  //ConsistencyChecker consistency_checker;
  //EntailmentChecker entailment_checker;

  //expanded_always_id2sptr(state->expanded_always_id, expanded_always);

  bool expanded   = true;
  while(expanded) {
    // tell������W�߂�
    tells_t tell_list;
    tell_collector.collect_new_tells(&tell_list,
                                     &expanded_always, &positive_asks);

    // ���񂪏[�����Ă��邩�ǂ����̊m�F
    // �[�����Ă���΃X�g�A���X�V
    switch(vcs.add_constraint(tell_list)) {
    case hydla::vcs::VCSR_TRUE:
      break;
    case hydla::vcs::VCSR_FALSE:
      return false;
    case hydla::vcs::VCSR_UNKNOWN:
      assert(false);
      break;
    case hydla::vcs::VCSR_SOLVER_ERROR:
      assert(false); // TODO: �b��I��
      break;
    }

    // ask������W�߂�
    ask_collector.collect_ask(&expanded_always,
                              &positive_asks, &unknown_asks);

    //ask����̃G���e�[������
    expanded = false;
    {
      negative_asks_t::iterator it  = unknown_asks.begin();
      negative_asks_t::iterator end = unknown_asks.end();
      while(it!=end) {
        // �ȑO�ɃG���e�[������Ȃ��Ɣ��f���ꂽask����̓X�L�b�v
        if(negative_asks.find(*it)!=negative_asks.end()) {
          HYDLA_LOGGER_DEBUG("#*** do_point_phase: skip un-entailed ask ***");
          unknown_asks.erase(it++);
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
            // Ask�����Ɋ�Â��ĕ���
            HYDLA_LOGGER_DEBUG("#*** do_point_phase: BRANCH ***\n");
            // �G���e�[�������ꍇ
            HYDLA_LOGGER_DEBUG("#*** do_point_phase: TRUE CASE ***\n");
            // Ask���玮node�̃��X�g�𓾂�
            GuardLister lister;
            node_list_t g_list = lister.get_guard_list(*it);
            RealPaverVCS vcs_t(vcs);
            for(node_list_t::iterator nit=g_list.begin(); nit!=g_list.end(); nit++) {
              // �K�[�h�������X�g�A(VCS)�ɉ�����
              vcs_t.add_single_constraint(*nit);
            }
            TellCollector tc_t(tell_collector);
            expanded_always_t ea_t(expanded_always);
            positive_asks_t pa_t(positive_asks); pa_t.insert(*it);
            negative_asks_t na_t(negative_asks);
            bool result = this->do_point_phase(ms, state, vcs_t, tc_t, ea_t, pa_t, na_t);
            // �G���e�[������Ȃ��ꍇ
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

  // IntervalPhase��
  phase_state_sptr new_state(create_new_phase_state(/*state*/));
  new_state->module_set_container = msc_no_init_;
  new_state->phase = IntervalPhase;
  // ConstraintStore����variable_map���쐬
  vcs.create_variable_map(new_state->variable_map);
  //expanded_always_sptr2id(expanded_always, new_state->expanded_always_id);
  push_phase_state(new_state);

  // �����̓V�~�����[�V�������ʂ̏o�͂Ƃ��ĕK�v�Ȃ񂶂�Ȃ��H
  //std::cout << new_state->variable_map;

  return true;
}

/**
 * Interval Phase�̏���
 */
bool BPSimulator::interval_phase(const module_set_sptr& ms, 
                                 const phase_state_const_sptr& state)
{
  HYDLA_LOGGER_DEBUG("#** interval_phase: BEGIN **\n");
  // �����l�̓R�s�[���Ă����K�v�����邩���H
  expanded_always_t expanded_always;
  //expanded_always_id2sptr(state->expanded_always_id, expanded_always);
  positive_asks_t positive_asks;
  negative_asks_t negative_asks;
  RealPaverVCS vcs(RealPaverVCS::ContinuousMode, &ml_);
  vcs.reset(state->variable_map);
  TellCollector tell_collector(ms);
  AskCollector ask_collector(ms, AskCollector::ENABLE_COLLECT_NON_TYPED_ASK | 
    AskCollector::ENABLE_COLLECT_CONTINUOUS_ASK);

  tells_t tell_list;

  bool expanded = true;
  while(expanded) {
    // tell������W�߂�
    tell_collector.collect_all_tells(&tell_list,
                                     &expanded_always, 
                                     &positive_asks);

    // �[���m�F�ƃX�g�A�ւ̒ǉ�, ODE����
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

    // ask������W�߂�
    ask_collector.collect_ask(&expanded_always,
                              &positive_asks,
                              &negative_asks);

    expanded = false;
    {
      // ask�����̃G���e�[������
      // IP��ask������prev�͓����ĂȂ� => ���Ԃ�UNKNOWN�ɂ͂Ȃ�Ȃ��c
      // �������C���w�I��T/F�Ȃ̂�U�ɂȂ���̂�����̂ő΍�
      // IP�ł͘A�������� => ����0�ł��邱�̂Ƃ��͑S�Ă̕ϐ��̒l�������l�ϐ��Ɠ���
      // ���̏󋵂�FALSE�ł�����̂ɓ�x�ڂ̃`�����X��^����K�v�����邩�H
      // �܂���������
      negative_asks_t::iterator it  = negative_asks.begin();
      negative_asks_t::iterator end = negative_asks.end();
      while(it!=end) {
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
          // IP�ł͋N����Ȃ�(�悤�ɂ���)
          assert(false);
          break;
        case hydla::vcs::VCSR_SOLVER_ERROR:
          assert(false);
          break;
        }
      }
    }
  } // while

  virtual_constraint_solver_t::IntegrateResult integrate_result;
  // (list({t, vm}),list({ask,p?n})) = �ϕ�����(integrate)
  // integrate(cs, p_a, n_a, time, max_time);
  // time, vm, c_ask����phase_state�����
  // �ς�
  // �����m�ɂ�for���ň͂ނƎv����
  /*phase_state_sptr new_state(create_new_phase_state(state));
  new_state->phase        = phase_state_t::PointPhase;
  new_state->initial_time = false;
  new_state->variable_map = vm;
  push_phase_state(new_state);
*/
  return true;
}

/**
 * Interval Phase�̎����I�ȏ���
 * ask�̃G���e�[���Ɋ�Â������ċA�����\��������D
 *
 * @param ms ���W���[���W��
 * @param state Point Phase�J�n���̏��
 * @param constraint_store ����X�g�A
 * @param positive_asks �G���e�[�����ꂽask���񃊃X�g
 * @param negative_asks �G���e�[������Ȃ�ask���񃊃X�g
 *
 * @return Interval Phase�𖞂����������݂��邩
 */
bool BPSimulator::do_interval_phase(const module_set_sptr &ms,
                                    const phase_state_const_sptr &state,
                                    hydla::vcs::realpaver::RealPaverVCS& vcs,
                                    TellCollector &tell_collector,
                                    expanded_always_t& expanded_always,
                                    positive_asks_t &positive_asks,
                                    negative_asks_t &negative_asks)
{
  return true;
}

} // bp_simulator
} // hydla

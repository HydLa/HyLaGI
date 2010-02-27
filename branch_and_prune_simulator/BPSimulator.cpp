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
    AskTypeAnalyzer().analyze(pt_original.get());
    AskDisjunctionFormatter().format(pt_original.get());
    AskDisjunctionSplitter().split(pt_original.get());
    msc_original_ = mcc.create(pt_original);
    HYDLA_LOGGER_DEBUG("#* original module set * \n",*pt_original); // �f�o�b�O
  }

  {
    // initial�Ȑ���������� + ask format
    parse_tree_sptr pt_no_init(boost::make_shared<ParseTree>(*parse_tree));
    InitNodeRemover().apply(pt_no_init.get());
    AskTypeAnalyzer().analyze(pt_no_init.get());
    AskDisjunctionFormatter().format(pt_no_init.get());
    AskDisjunctionSplitter().split(pt_no_init.get());
    msc_no_init_ = mcc.create(pt_no_init);
    HYDLA_LOGGER_DEBUG("#* no init module set *\n", *pt_no_init); 
  }

  // TODO: �p�~
  //{
  //  // no initial + ���U�ω�ask�������� + ask format
  //  // ���U�ω�ask�Ƃ́c������prev�ϐ��̂���ask
  //  parse_tree_sptr pt_no_init_discreteask(boost::make_shared<ParseTree>(*parse_tree));
  //  InitNodeRemover().apply(pt_no_init_discreteask.get());
  //  DiscreteAskRemover().apply(pt_no_init_discreteask.get());
  //  AskDisjunctionFormatter().format(pt_no_init_discreteask.get());
  //  AskDisjunctionSplitter().split(pt_no_init_discreteask.get());
  //  msc_no_init_discreteask_ = mcc.create(pt_no_init_discreteask);
  //}

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
  // HydLa.m�̓��e���M
  //   ml_.MLPutFunction("Get", 1);
  //   ml_.MLPutString("symbolic_simulator/HydLa.m");
  ml_.MLPutFunction("ToExpression", 1);
  ml_.MLPutString(vcs_math_source());  
  ml_.MLEndPacket();
  ml_.skip_pkt_until(RETURNPKT);
  ml_.MLNewPacket();
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
  new_state->current_time = state->current_time;
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
  vcs.set_precision(this->precision_);
  vcs.reset(state->variable_map);
  TellCollector tell_collector(ms);
  AskCollector ask_collector(ms);

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
      // TODO: �������C���w�I��T/F�Ȃ̂�U�ɂȂ���̂�����̂ő΍�
      // IP�ł͘A�������� => ����0�ł��邱�̂Ƃ��͑S�Ă̕ϐ��̒l�������l�ϐ��Ɠ���
      negative_asks_t::iterator it  = negative_asks.begin();
      negative_asks_t::iterator end = negative_asks.end();
      while(it!=end) {
        // ���U�ω�ask��IP�ł̓`�F�b�N���Ȃ�
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
           // IP�ł͋N����Ȃ�(�悤�ɂ���)
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
      // �S�Ă̏�����Ԃɂ��Ď��̃|�C���g�t�F�[�Y��������������
      all_proof = true;
      break;
    case hydla::vcs::VCSR_FALSE:
      // ���̃|�C���g�t�F�[�Y�������Ȃ�����
      // result��is_max_time��true�H
      finish_simulate = true;
      all_proof = true;
      break;
    case hydla::vcs::VCSR_UNKNOWN:
      // �|�C���g�t�F�[�Y�͂��������C�S�Ă̏�����Ԃɂ��Ăł͂Ȃ�����
      // TODO: ����C�S�Ă̏�����Ԃɂ��Ĉ���������ask���Ȃ��Ƒ�ςȂ��ƂɂȂ�
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
    // �ςޏ����͂����ł��
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

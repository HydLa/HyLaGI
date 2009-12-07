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
   * �V�~�����[�V�������C��
   */
  bool BPSimulator::simulate(boost::shared_ptr<hydla::ch::ModuleSetContainer> msc, Opts& opts)
  {
    this->debug_mode_ = opts.debug_mode;
    this->max_time_   = opts.max_time;

    // ����state���X�^�b�N�ɐς�
    phase_state_sptr init_state_sptr(new phase_state_t);
    init_state_sptr->phase = phase_state_t::PointPhase;
    this->push_phase_state(init_state_sptr);

    // �����I�Ɏ���0��BPSimulator::point_phase���͂��܂�
    simulator_t::simulate(msc);

    return true;
  }

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

  /**
   * Point Phase�̏���
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
    //ConstraintStoreBuilderPoint csbp(ml_);       //TODO: kenshiro���쐬
    ConsistencyChecker consistency_checker(this->debug_mode_);
    EntailmentChecker entailment_checker;   //TODO: kenshiro���쐬

    TellCollector::tells_t tell_list;
    positive_asks_t   positive_asks;
    negative_asks_t   negative_asks;

    bool expanded   = true;
    while(expanded) {
      // tell������W�߂�
      tell_collector.collect_all_tells(&tell_list, &state->expanded_always, &positive_asks);
      if(debug_mode_) {
        std::cout << "#** collected tells **\n";  
        std::for_each(tell_list.begin(), tell_list.end(), NodeDump());
      }

      // ���񂪏[�����Ă��邩�ǂ����̊m�F
      if(!consistency_checker.is_consistent(tell_list)){
        if(debug_mode_) std::cout << "#*** inconsistent\n";
        return false;
      }
      if(debug_mode_) std::cout << "#*** consistent\n";

      // ask������W�߂�
      ask_collector.collect_ask(ms.get(), &state->expanded_always, 
        &positive_asks, &negative_asks);
      if(debug_mode_) {
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
  bool BPSimulator::interval_phase(hydla::ch::module_set_sptr& ms, phase_state_sptr& state)
  {
    return true;
  }

} // bp_simulator
} // hydla

#ifndef _INCLUDED_HYDLA_SIMULATOR_H_
#define _INCLUDED_HYDLA_SIMULATOR_H_

#include <iostream>

#include <string>
#include <map>
#include <set>
#include <queue>
#include <vector>

#include <assert.h>

#include <boost/bind.hpp>

#include "Node.h"
#include "ModuleSetContainer.h"

namespace hydla {
namespace simulator {

template<typename VariableType, typename ValueType, typename TimeType>
class Simulator
{
public:
  /**
   * ����X�g�A
   */
  typedef struct ConstraintStore_ {
    typedef typename std::map<VariableType, ValueType> variable_list_t;
    typedef typename variable_list_t::const_iterator variable_list_const_iterator_t;

    variable_list_t variable_list_;

    /**
     * �f�[�^���_���v����
     */
    std::ostream& dump(std::ostream& s) const
    {
      variable_list_const_iterator_t it  = variable_list_.begin();
      variable_list_const_iterator_t end = variable_list_.end();
      while(it!=end) {
        s << it->first.dump(s) << " : " << it->second.dump(s) << "\n";
      }
      return s;
    }
  } ConstraintStore;

  /**
   * �����̃t�F�[�Y
   */
  typedef enum Phase_ {
    PointPhase,
    IntervalPhase,
  } Phase;

  /**
   * �e�����̏��
   */
  typedef struct State_ {
    typedef boost::shared_ptr<hydla::parse_tree::Always> always_sptr;
    typedef std::vector<always_sptr>                     always_sptr_list_t;

    Phase               phase;
    TimeType            time;
    ConstraintStore     constraint_store;
    always_sptr_list_t  expanded_always;
    bool                initial_time;
  } State;

  Simulator()
  {}
  
  virtual ~Simulator()
  {}

  /**
   * �^����ꂽ����⃂�W���[���W�������ɃV�~�����[�V�������s�������Ȃ�
   */
  void simulate(boost::shared_ptr<hydla::ch::ModuleSetContainer> msc)//,
//                const hydla::parse_tree::variable_map_t& variable_map)
  {
    State init_state;
    init_state.phase = PointPhase;
    state_queue_.push(init_state);

    while(!state_queue_.empty()) {
      msc->dispatch(
        boost::bind(&Simulator<VariableType, ValueType, TimeType>::execute_module_set, 
                    this, 
                    _1,
                    &state_queue_.front()));
      state_queue_.pop();
    }
  }

  /**
   * �ɑ�Ȑ��񃂃W���[���W���̎��s�������Ȃ�
   */
  bool execute_module_set(hydla::ch::module_set_sptr ms, State* state)
  {
    bool ret;
    switch(state->phase) 
    {
    case PointPhase:
      ret = point_phase(ms, state);
      break;

    case IntervalPhase:
      ret = interval_phase(ms, state);
      break;

    default:
      assert(0);
    }

    return ret;
  }

protected:

  /**
   * Point Phase�̏���
   */
  virtual bool point_phase(hydla::ch::module_set_sptr& ms, State* state) = 0;
  
  /**
   * Interval Phase�̏���
   */
  virtual bool interval_phase(hydla::ch::module_set_sptr& ms, State* state) = 0;

  // �e��Ԃ�ۑ����Ă������߂̃L���[
  std::queue<State> state_queue_;
};

} //namespace simulator
} //namespace hydla 

#endif // _INCLUDED_HYDLA_SIMULATOR_H_

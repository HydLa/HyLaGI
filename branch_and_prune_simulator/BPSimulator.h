#ifndef _INCLUDED_HYDLA_BP_SIMULATOR_H_
#define _INCLUDED_HYDLA_BP_SIMULATOR_H_

#include "Simulator.h"

namespace hydla {
namespace bp_simulator {

/**
 * �ϐ����E����
 */
typedef struct BPVaribale_ {
  std::string name;
  bool previous;
  bool initial;

  /* �_���v */
  std::ostream& dump(std::ostream& s) const
  {
    s << name;
    if(previous) s << "-";
    if(initial) s << "_0";
    return s;
  }
} BPVariable;

/**
 * �ϐ��l
 */
typedef struct BPValue_
{
  // rp_interval?

  /* �_���v */
  std::ostream& dump(std::ostream& s) const
  {
    s << "value";
    return s;
  }
} BPValue;

/**
 * ����
 * �����ƌo�ߎ��ԁH
 */
typedef struct BPTime_
{
} BPTime;

typedef simulator::PhaseState<BPVariable, BPValue, BPTime> phase_state_t;
typedef boost::shared_ptr<phase_state_t> phase_state_sptr;
typedef simulator::Simulator<phase_state_t> simulator_t;

class BPSimulator : public simulator_t
{
public:

  typedef struct Opts_ {
    bool debug_mode;
    std::string max_time;
    bool profile_mode;    // �Ȃ�
    bool parallel_mode;   // �Ȃ�
  } Opts;

  BPSimulator();
  virtual ~BPSimulator();

  bool simulate(boost::shared_ptr<hydla::ch::ModuleSetContainer> msc, Opts& opts);

//  virtual bool test_module_set(hydla::ch::module_set_sptr ms);


  /**
   * Point Phase�̏���
   */
  virtual bool point_phase(hydla::ch::module_set_sptr& ms, phase_state_sptr& state);
  
  /**
   * Interval Phase�̏���
   */
  virtual bool interval_phase(hydla::ch::module_set_sptr& ms, phase_state_sptr& state);

private:
  bool debug_mode_;
  std::string max_time_;

};

} // namespace bp_simulator
} // namespace hydla

#endif //_INCLUDED_HYDLA_BP_SIMULATOR_H_
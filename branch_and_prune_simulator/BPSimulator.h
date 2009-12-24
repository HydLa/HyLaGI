#ifndef _INCLUDED_HYDLA_BP_SIMULATOR_H_
#define _INCLUDED_HYDLA_BP_SIMULATOR_H_


#include "Simulator.h"
#include "DefaultVariable.h"

namespace hydla {
namespace bp_simulator {

/**
 * �ϐ����E����
 */
  /*
struct BPVariable {
  std::string name;
  unsigned int derivative_count;

  bool previous;
  bool initial;

  std::ostream& dump(std::ostream& s) const
  {
    s << name;
    if(previous) s << "-";
    if(initial) s << "_0";
    return s;
  }

   
  friend std::ostream& operator<<(std::ostream& s, 
                                  const BPVariable & v)
  {
    return v.dump(s);
  }
};
*/
typedef simulator::DefaultVariable BPVariable;

/**
 * �ϐ��l
 */
struct BPValue
{
  // rp_interval?

  /* �_���v */
  std::ostream& dump(std::ostream& s) const
  {
    s << "value";
    return s;
  }
    
  friend std::ostream& operator<<(std::ostream& s, 
                                  const BPValue & v)
  {
    return v.dump(s);
  }
};

/**
 * ����
 * �����ƌo�ߎ��ԁH
 */
struct BPTime
{
};

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

  BPSimulator(const Opts& opts);
  virtual ~BPSimulator();


  /**
   * Point Phase�̏���
   */
  virtual bool point_phase(const module_set_sptr& ms, 
                           const phase_state_const_sptr& state);
  
  /**
   * Interval Phase�̏���
   */
  virtual bool interval_phase(const module_set_sptr& ms, 
                              const phase_state_const_sptr& state);

private:  
  /**
   * ����������
   */
  virtual void do_initialize();

  Opts opts_;
  std::string max_time_;
};

} // namespace bp_simulator
} // namespace hydla

#endif //_INCLUDED_HYDLA_BP_SIMULATOR_H_
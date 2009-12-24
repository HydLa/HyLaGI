#ifndef _INCLUDED_HYDLA_BP_SIMULATOR_H_
#define _INCLUDED_HYDLA_BP_SIMULATOR_H_


#include "Simulator.h"
#include "DefaultVariable.h"

namespace hydla {
namespace bp_simulator {

/**
 * 変数名・属性
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
 * 変数値
 */
struct BPValue
{
  // rp_interval?

  /* ダンプ */
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
 * 時刻
 * 時刻と経過時間？
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
    bool profile_mode;    // ない
    bool parallel_mode;   // ない
  } Opts;

  BPSimulator(const Opts& opts);
  virtual ~BPSimulator();


  /**
   * Point Phaseの処理
   */
  virtual bool point_phase(const module_set_sptr& ms, 
                           const phase_state_const_sptr& state);
  
  /**
   * Interval Phaseの処理
   */
  virtual bool interval_phase(const module_set_sptr& ms, 
                              const phase_state_const_sptr& state);

private:  
  /**
   * 初期化処理
   */
  virtual void do_initialize();

  Opts opts_;
  std::string max_time_;
};

} // namespace bp_simulator
} // namespace hydla

#endif //_INCLUDED_HYDLA_BP_SIMULATOR_H_
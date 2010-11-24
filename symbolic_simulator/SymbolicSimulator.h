#ifndef _INCLUDED_SYMBOLIC_SIMULATOR_H_
#define _INCLUDED_SYMBOLIC_SIMULATOR_H_

#include <string>
#include <stack>

#include "ParseTree.h"

#include "Simulator.h"
#include "mathlink_helper.h"

#include "../virtual_constraint_solver/mathematica/MathematicaVCS.h"
#include "../virtual_constraint_solver/mathematica/MathVariable.h"
#include "../virtual_constraint_solver/mathematica/MathValue.h"
#include "../virtual_constraint_solver/mathematica/MathTime.h"

#include "Types.h"
using namespace hydla::simulator;

namespace hydla {
namespace symbolic_simulator {

typedef hydla::vcs::mathematica::MathVariable symbolic_variable_t;
typedef hydla::vcs::mathematica::MathValue    symbolic_value_t;
typedef hydla::vcs::mathematica::MathTime     symbolic_time_t;

typedef hydla::simulator::VariableMap<symbolic_variable_t, 
                                      symbolic_value_t> variable_map_t;
typedef hydla::simulator::PhaseState<symbolic_variable_t, 
                                     symbolic_value_t, 
                                     symbolic_time_t> phase_state_t;
typedef boost::shared_ptr<phase_state_t> phase_state_sptr;
typedef hydla::simulator::Simulator<phase_state_t> simulator_t;

class SymbolicSimulator : public simulator_t
{
public:

  typedef enum OutputFormat_ {
    fmtTFunction,
    fmtNumeric,
  } OutputFormat;

  typedef struct Opts_ {
    std::string mathlink;
    std::string output;
    bool debug_mode;
    std::string max_time; //TODO: symbolic_time_t�ɂ���
    bool nd_mode;
    bool interactive_mode;
    bool profile_mode;
    bool parallel_mode;
    OutputFormat output_format;
    symbolic_time_t output_interval;
    int             output_precision;
    int approx_precision;
  } Opts;

  SymbolicSimulator(const Opts& opts);
  virtual ~SymbolicSimulator();

  
  /**
   * �^����ꂽ����⃂�W���[���W�������ɃV�~�����[�V�������s�������Ȃ�
   */
  virtual void simulate();

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
  virtual void do_initialize(const parse_tree_sptr& parse_tree);

  void init_module_set_container(const parse_tree_sptr& parse_tree);
  void init_mathlink();

  void output(const symbolic_time_t& time, 
              const variable_map_t& vm);
              
              

                              
  module_set_container_sptr msc_original_;

  module_set_container_sptr msc_no_init_;
  module_set_container_sptr msc_no_init_discreteask_;

  Opts     opts_;
  MathLink ml_; 
  
  

  
  /*
  * �C���^���N�e�B�u���[�h�p�B�ePhase�ɂ�����A���W���[���W���̖�����������s��
  */
  virtual bool judge_phase_state(const module_set_sptr& ms, 
                                    const phase_state_const_sptr& state);

  /**
   �C���^���N�e�B�u���[�h�p�BPoint Phase�̏o�́�����Ԑ���
   */
  bool do_point_phase(const module_set_sptr& ms, 
                              const phase_state_const_sptr& state, tells_t tell_list, const expanded_always_t& expanded_always);

  /**
   �C���^���N�e�B�u���[�h�p�BInterval Phase�̏o�́�����Ԑ���
   */
  bool do_interval_phase(const module_set_sptr& ms, 
                              const phase_state_const_sptr& state, const tells_t& tell_list,const positive_asks_t& positive_asks, const negative_asks_t& negative_asks, const expanded_always_t& expanded_always);
                                
  
  std::vector<module_set_sptr> module_set_vector; //�C���^���N�e�B�u���[�h�p�́A�������ɑ�W����vector
  std::vector<tells_t> tell_vector;         //�C���^���N�e�B�u���[�h�p�́Atell��vector
  std::vector<hydla::simulator::positive_asks_t> positive_asks_vector; //�C���^���N�e�B�u���[�h�p�́Anegative ask��vector
  std::vector<hydla::simulator::negative_asks_t> negative_asks_vector; //�C���^���N�e�B�u���[�h�p�́Apositive ask��vector
  std::vector<hydla::simulator::expanded_always_t> expanded_always_vector; //�C���^���N�e�B�u���[�h�p�́Apositive ask��vector
  int step;                                       //�C���^���N�e�B�u���[�h�p�́A�i�߂�X�e�b�v��
  std::ostream *output_dest;                       //�V�~�����[�V�������ʂ̏o�͐�B���[�U�[�ւ̃��b�Z�[�W������
  std::stack<phase_state_sptr> track_back_stack;
  //std::vector<phase_state_sptr> state_vector; 
//  hydla::vcs::mathematica::MathematicaVCS vcs_;
};

} // namespace symbolic_simulator
} // namespace hydla

#endif //_INCLUDED_SYMBOLIC_SIMULATOR_H_

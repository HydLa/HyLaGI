#ifndef _INCLUDED_SYMBOLIC_SIMULATOR_H_
#define _INCLUDED_SYMBOLIC_SIMULATOR_H_

#include <string>
#include <sstream>
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
  
  typedef enum OutputStyle_ {
    styleTree,
    styleList,
  } OutputStyle;

  typedef struct Opts_ {
    std::string mathlink;
    bool debug_mode;
    std::string max_time; //TODO: symbolic_time_t�ɂ���
    bool nd_mode;
    OutputStyle output_style;
    bool interactive_mode;
    bool profile_mode;
    bool parallel_mode;
    OutputFormat output_format;
    symbolic_time_t output_interval;
    int             output_precision;
    int approx_precision;
    std::string solver;
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

  /*
  * reduce�o�͗p�֐��B���݂�point_phase�֐����番��
  */
  
  virtual bool reduce_output(const module_set_sptr& ms, 
                           const phase_state_const_sptr& state);

private:
  /**
   * ����������
   */
  virtual void do_initialize(const parse_tree_sptr& parse_tree);

  void init_module_set_container(const parse_tree_sptr& parse_tree);
  void init_mathlink();
  
  bool calculate_closure(const module_set_sptr& ms, hydla::vcs::mathematica::MathematicaVCS &vcs, expanded_always_t &expanded_always,
                         positive_asks_t &positive_asks, negative_asks_t &negative_asks);

  void output(const symbolic_time_t& time, 
              const variable_map_t& vm);


void output_interval(hydla::vcs::mathematica::MathematicaVCS &vcs, const symbolic_time_t& current_time, const symbolic_time_t& limit_time,
                     const variable_map_t& variable_map);
              
                              
  module_set_container_sptr msc_original_;

  module_set_container_sptr msc_no_init_;
  module_set_container_sptr msc_no_init_discreteask_;

  Opts     opts_;
  MathLink ml_;
  
  
  virtual void push_phase_state(const phase_state_sptr& state) 
  {
    branch_++;
    state_stack_.push(state);
  }
 
                               
  
  std::vector<std::string> output_vector_;           //�S��T���ŁC�S�o�H�o�͂��s�����߂̔z��
  std::stack<int> branch_stack_;                   //���O�ɕ��򂵂������L�����邽�߂̃X�^�b�N
  int branch_;                                     //���t�F�[�Y�ŉ���Ԃɕ��򂵂����������ϐ�
  std::ostringstream output_buffer_;                      //�X�^�b�N�ɓ���邽�߂́C���O�̕��򂩂猻�݂܂ł̏o�͕ێ�
};

} // namespace symbolic_simulator
} // namespace hydla

#endif //_INCLUDED_SYMBOLIC_SIMULATOR_H_

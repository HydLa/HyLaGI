#ifndef _INCLUDED_SYMBOLIC_SIMULATOR_H_
#define _INCLUDED_SYMBOLIC_SIMULATOR_H_

#include <string>
#include <iostream>
#include <fstream>

#include <sstream>
#include <stack>

#include "ParseTree.h"

#include "Simulator.h"

#include <boost/scoped_ptr.hpp>

#include "Types.h"
#include "SymbolicTypes.h"
#include "../virtual_constraint_solver/SymbolicVirtualConstraintSolver.h"

using namespace hydla::simulator;

namespace hydla {
namespace symbolic_simulator {


class SymbolicSimulator : public simulator_t
{
public:

  typedef hydla::vcs::SymbolicVirtualConstraintSolver solver_t;


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
  * �^����ꂽ����⃂�W���[���W���̃��X�g��reduce�̂��߂Ƀt�@�C���o�͂���֐��Binitialize�O�ŕ���
  */
    virtual bool reduce_simulate();

  /*
  * reduce�o�͗p�֐��B���݂�point_phase�֐����番��
  * svn Rev: 1062
  */
    virtual bool reduce_output(const module_set_sptr& ms, 
                             const phase_state_const_sptr& state);
  
  /*
  * dispatch�֐��ɓn����ModuleSet��pair<name,node>���擾
  */

  virtual bool reduce_simulate_phase_state(const module_set_sptr& ms);

private:
  /**
   * ����������
   */
  virtual void do_initialize(const parse_tree_sptr& parse_tree);
  
  variable_map_t shift_variable_map_time(const variable_map_t& vm,const time_t &time);

  void init_module_set_container(const parse_tree_sptr& parse_tree);
  
  CalculateClosureResult calculate_closure(const phase_state_const_sptr& state,
                        const module_set_sptr& ms, expanded_always_t &expanded_always,
                         positive_asks_t &positive_asks, negative_asks_t &negative_asks);

  void output(const time_t& time, 
              const variable_map_t& vm);
  
  void output(const time_t& time, 
              const variable_map_t& vm,
              const parameter_map_t& pm);
  

  void output_interval(const time_t& current_time, const time_t& limit_time,
                     const variable_map_t& variable_map);

  void output_point(const time_t& time, const variable_map_t& variable_map, const parameter_map_t& parameter_map);
              
                              
  module_set_container_sptr msc_original_;

  module_set_container_sptr msc_no_init_;
  module_set_container_sptr msc_no_init_discreteask_;

  Opts     opts_;
  
  
  virtual void push_phase_state(const phase_state_sptr& state) 
  {
    branch_++;
    state_stack_.push(state);
  }
  
  std::string value_to_string(const value_t& val);


  boost::scoped_ptr<solver_t> solver_;               //�g�p����\���o
  
  std::vector<std::string> output_vector_;           //�S��T���ŁC�S�o�H�o�͂��s�����߂̔z��
  std::stack<int> branch_stack_;                     //���O�ɕ��򂵂������L�����邽�߂̃X�^�b�N
  int branch_;                                       //���t�F�[�Y�ŉ���Ԃɕ��򂵂����������ϐ�
  std::ostringstream output_buffer_;                 //�X�^�b�N�ɓ���邽�߂́C���O�̕��򂩂猻�݂܂ł̏o�͕ێ�
};

} // namespace symbolic_simulator
} // namespace hydla

#endif //_INCLUDED_SYMBOLIC_SIMULATOR_H_

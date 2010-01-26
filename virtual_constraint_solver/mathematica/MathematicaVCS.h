#ifndef _INCLUDED_HYDLA_VCS_MATHEMATICA_VCS_H_
#define _INCLUDED_HYDLA_VCS_MATHEMATICA_VCS_H_

/**
 * �v���O�����Ԃ̈ˑ����̖�肩��C
 * ���̃w�b�_�[����т��̃w�b�_�[����C���N���[�h�����w�b�_�[�ɂ�����
 * �\���o�[�ˑ��̃w�b�_�[(mathematica��realpaver���̌ŗL�̃w�b�_�[)��
 * �C���N���[�h���Ă͂Ȃ�Ȃ�
 */

#include <boost/scoped_ptr.hpp>

#include "MathVCSType.h"

class MathLink;

namespace hydla {
namespace vcs {
namespace mathematica {

class MathematicaVCS : 
    public virtual_constraint_solver_t
{
public:
  enum Mode {
    ContinuousMode,
    DiscreteMode,
  };

  MathematicaVCS(Mode m, MathLink* ml, int approx_precision);

  virtual ~MathematicaVCS();

  /**
   * ����X�g�A�̏������������Ȃ�
   */
  virtual bool reset();

  /**
   * �^����ꂽ�ϐ��\�����ɁC����X�g�A�̏������������Ȃ�
   */
  virtual bool reset(const variable_map_t& vm);

  /**
   * ���݂̐���X�g�A����ϐ��\���쐬����
   */
  virtual bool create_variable_map(variable_map_t& vm);

  /**
   * �����ǉ�����
   */
  virtual VCSResult add_constraint(const tells_t& collected_tells);
  
  /**
   * ���݂̐���X�g�A����^����ask�����o�\���ǂ���
   */
  virtual VCSResult check_entailment(const ask_node_sptr& negative_ask);

  /**
   * ask�̓��o��Ԃ��ω�����܂Őϕ��������Ȃ�
   */
  virtual VCSResult integrate(
    integrate_result_t& integrate_result,
    const positive_asks_t& positive_asks,
    const negative_asks_t& negative_asks,
    const time_t& current_time,
    const time_t& max_time);

  /**
   * ���݂̃��[�h��Ԃ�
   */
  Mode get_mode() const {
    return mode_;
  }

  /**
   * ���[�h��ύX����
   * ����X�g�A���̂��ׂĂ̓�����Ԃ͂��ׂď����������
   */
//  void change_mode(Mode m);

  /**
   * ���ʂ̏o�͊֐���ݒ肷��
   */
  virtual void set_output_func(const time_t& max_interval, 
                               const output_function_t& func);

  /**
   * ���ʂ̏o�͊֐��̐ݒ�����Z�b�g���C������Ԃɖ߂�
   */
  virtual void reset_output_func();

private:
  Mode      mode_;
//  MathLink* ml_;

  boost::scoped_ptr<virtual_constraint_solver_t> vcs_;
};

} // namespace mathematica
} // namespace simulator
} // namespace hydla 

#endif // _INCLUDED_HYDLA_VCS_MATHEMATICA_VCS_H_

#ifndef _INCLUDED_HYDLA_VCS_REALPAVER_REALPAVER_VCS_H_
#define _INCLUDED_HYDLA_VCS_REALPAVER_REALPAVER_VCS_H_

/**
 * �v���O�����Ԃ̈ˑ����̖�肩��C
 * ���̃w�b�_�[����т��̃w�b�_�[����C���N���[�h�����w�b�_�[�ɂ�����
 * �\���o�[�ˑ��̃w�b�_�[(mathematica��realpaver���̌ŗL�̃w�b�_�[)��
 * �C���N���[�h���Ă͂Ȃ�Ȃ�
 */

#include <boost/scoped_ptr.hpp>

//#include "RPVCSType.h"
#include "RealPaverBaseVCS.h"

class MathLink;

namespace hydla {
namespace vcs {
namespace realpaver {

class RealPaverVCS : 
    public RealPaverBaseVCS
{
public:
  enum Mode {
    ContinuousMode,
    DiscreteMode,
  };

  RealPaverVCS(Mode m);
  RealPaverVCS(Mode m, MathLink* ml);
  RealPaverVCS(const RealPaverVCS& src);

  virtual ~RealPaverVCS();

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
   * ���[�h��Ԃ�
   */
  Mode get_mode() const {
    return mode_;
  }

/******************** realpaver only ********************/

  virtual void add_single_constraint(const node_sptr& constraint_node,
    const bool neg_expression=false);

  virtual void set_precision(const double p);

/******************** realpaver only ********************/

private:
  virtual RealPaverBaseVCS* clone(); //�R�s�[�֎~

  Mode mode_;

  boost::scoped_ptr<RealPaverBaseVCS> vcs_;
};

} // namespace realpaver
} // namespace vcs
} // namespace hydla 

#endif // _INCLUDED_HYDLA_VCS_REALPAVER_REALPAVER_VCS_H_

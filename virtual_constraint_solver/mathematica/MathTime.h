#ifndef _INCLUDED_HYDLA_VCS_MATHEMATICA_MATH_TIME_H_
#define _INCLUDED_HYDLA_VCS_MATHEMATICA_MATH_TIME_H_

#include <iostream>
#include <string>

#include <boost/operators.hpp>

class MathLink;

namespace hydla {
namespace vcs {
namespace mathematica {

class MathTime : 
    public boost::additive<MathTime>
{
public:
  typedef std::string time_t;
  
  /**
   * �f�t�H���g�R���X�g���N�^
   */
  MathTime();

  /**
   * �R���X�g���N�^
   * ����������ɐ�������
   */
  MathTime(const std::string& str);

  /**
   * �f�X�g���N�^
   */ 
  ~MathTime();

  /**
   * Mathematica�֎����f�[�^�𑗐M����
   */
  void send_time(MathLink& ml) const;

  /**
   * Mathematica���玞���f�[�^����M����
   */
  void receive_time(MathLink& ml);

  /**
   * ���������_�`���̒l���擾����
   */
  std::string get_real_val(MathLink& ml, int precision) const;

  /**
   * MathTime���m�̉��Z
   */
  MathTime& operator+=(const MathTime& rhs);

  /**
   * MathTime���m�̌��Z
   */
  MathTime& operator-=(const MathTime& rhs);

  /**
   * �f�[�^���_���v����
   */
  std::ostream& dump(std::ostream& s) const;

private:
  time_t time_; 
};

std::ostream& operator<<(std::ostream& s, const MathTime & t);


} // namespace mathematica
} // namespace simulator
} // namespace hydla

#endif // _INCLUDED_HYDLA_VCS_MATHEMATICA_MATH_TIME_H_


#ifndef _SYMBOLIC_TIME_H_
#define _SYMBOLIC_TIME_H_

#include <iostream>
#include <string>

#include <boost/operators.hpp>

namespace hydla {
namespace symbolic_simulator{

class SymbolicTime : 
    public boost::additive<SymbolicTime>
{
public:
  typedef std::string time_t;
  
  /**
   * �f�t�H���g�R���X�g���N�^
   */
  SymbolicTime();

  /**
   * �R���X�g���N�^
   * ����������ɐ�������
   */
  SymbolicTime(const std::string& str);

  /**
   * �f�X�g���N�^
   */ 
  ~SymbolicTime();

  
  std::string get_string() const;

  void set(const time_t &time);

  /**
   * SymbolicTime���m�̉��Z
   */
  SymbolicTime& operator+=(const SymbolicTime& rhs);
  

  /**
   * SymbolicTime���m�̌��Z
   */
  SymbolicTime& operator-=(const SymbolicTime& rhs);


  /**
   * �f�[�^���_���v����
   */
  std::ostream& dump(std::ostream& s) const;

private:
  time_t time_; 
};

std::ostream& operator<<(std::ostream& s, const SymbolicTime & t);


} // namespace symbolic_simulator
} // namespace hydla

#endif // _SYMBOLIC_TIME_H_


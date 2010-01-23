#ifndef _INCLUDED_HYDLA_SYMBOLIC_TIME_H_
#define _INCLUDED_HYDLA_SYMBOLIC_TIME_H_

#include <iostream>
#include <string>

#include <boost/operators.hpp>

#include "mathlink_helper.h"

namespace hydla {
namespace symbolic_simulator {

class SymbolicTime : 
    public boost::additive<SymbolicTime>
{
public:
  typedef std::string time_t;
  
  /**
   * �f�t�H���g�R���X�g���N�^
   */
  SymbolicTime() :
    time_("0")
  {}

  /**
   * �R���X�g���N�^
   * ����������ɐ�������
   */
  SymbolicTime(const std::string& str)
  {
    time_ = str;
  }

  ~SymbolicTime()
  {}

  /**
   * Mathematica�֎����f�[�^�𑗐M����
   */
  void send_time(MathLink& ml)
  {
    ml.put_function("ToExpression", 1);
    ml.put_string(time_);
  }

  /**
   * Mathematica���玞���f�[�^����M����
   */
  void receive_time(MathLink& ml)
  {
    time_ = ml.get_string();
  }

  SymbolicTime& operator+=(const SymbolicTime& rhs)
  {
    time_ += " + " + rhs.time_;
    return *this;
  }

  SymbolicTime& operator-=(const SymbolicTime& rhs)
  {
    time_ += " - " + rhs.time_;
    return *this;
  }

  /**
   * �f�[�^���_���v����
   */
  std::ostream& dump(std::ostream& s) const
  {
    s << time_;
    return s;
  }

  friend std::ostream& operator<<(std::ostream& s, 
                                  const SymbolicTime & t)
  {
    return t.dump(s);
  }

private:
  time_t time_; 
};

} //namespace symbolic_simulator
} // namespace hydla

#endif // _INCLUDED_HYDLA_SYMBOLIC_TIME_H_


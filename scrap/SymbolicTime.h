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
   * デフォルトコンストラクタ
   */
  SymbolicTime() :
    time_("0")
  {}

  /**
   * コンストラクタ
   * 文字列を元に生成する
   */
  SymbolicTime(const std::string& str)
  {
    time_ = str;
  }

  ~SymbolicTime()
  {}

  /**
   * Mathematicaへ時刻データを送信する
   */
  void send_time(MathLink& ml)
  {
    ml.put_function("ToExpression", 1);
    ml.put_string(time_);
  }

  /**
   * Mathematicaから時刻データを受信する
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
   * データをダンプする
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


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
   * デフォルトコンストラクタ
   */
  MathTime();

  /**
   * コンストラクタ
   * 文字列を元に生成する
   */
  MathTime(const std::string& str);

  /**
   * デストラクタ
   */ 
  ~MathTime();

  /**
   * Mathematicaへ時刻データを送信する
   */
  void send_time(MathLink* ml);

  /**
   * Mathematicaから時刻データを受信する
   */
  void receive_time(MathLink* ml);

  /**
   * MathTime同士の加算
   */
  MathTime& operator+=(const SymbolicTime& rhs);

  /**
   * MathTime同士の減算
   */
  MathTime& operator-=(const SymbolicTime& rhs);

  /**
   * データをダンプする
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


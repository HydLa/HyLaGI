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
   * デフォルトコンストラクタ
   */
  SymbolicTime();

  /**
   * コンストラクタ
   * 文字列を元に生成する
   */
  SymbolicTime(const std::string& str);

  /**
   * デストラクタ
   */ 
  ~SymbolicTime();

  
  std::string get_string() const;

  void set(const time_t &time);

  /**
   * SymbolicTime同士の加算
   */
  SymbolicTime& operator+=(const SymbolicTime& rhs);
  

  /**
   * SymbolicTime同士の減算
   */
  SymbolicTime& operator-=(const SymbolicTime& rhs);


  /**
   * データをダンプする
   */
  std::ostream& dump(std::ostream& s) const;

private:
  time_t time_; 
};

std::ostream& operator<<(std::ostream& s, const SymbolicTime & t);


} // namespace symbolic_simulator
} // namespace hydla

#endif // _SYMBOLIC_TIME_H_


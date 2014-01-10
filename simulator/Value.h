#ifndef _SIMULATOR_VALUE_H_
#define _SIMULATOR_VALUE_H_

#include <string>
#include <vector>
#include <boost/shared_ptr.hpp>

#include "Node.h"

namespace hydla {
namespace simulator {

class Value
{  
  public:

  typedef hydla::parse_tree::node_sptr node_sptr;
  
  virtual ~Value(){}
  
  Value();

  /**
   * 単なる文字列は数値と見なして受け取る
   */
  Value(const std::string &str);
  
  /**
   * 渡された数式を値とするValueを作る
   */
  Value(const node_sptr & node);
  
  /**
   * 未定義値かどうか
   */
  virtual bool undefined() const;

  /**
   * 文字列表現を取得する
   */
  virtual std::string get_string() const;

  /**
   * Nodeの形式にしたものを取得する
   */
  virtual node_sptr get_node() const;

  virtual void set_node(const node_sptr&);
  
  /**
   * Value同士の加算
   */
  Value& operator+=(const Value &rhs);
  Value operator+(const Value &rhs);

  /**
   * Value同士の減算
   */
  Value& operator-=(const Value &rhs);
  Value operator-(const Value &rhs);
  
  /**
   * データをダンプする
   */
  std::ostream& dump(std::ostream& s) const{
    if(undefined()) s << "UNDEF";
    else s << get_string();
    return s;
  }

  private:

  node_sptr node_;  /// symbolic expression
};

bool operator<(const Value& lhs, const Value& rhs);

std::ostream& operator<<(std::ostream& s, const Value & v);


} // namespace simulator
} // namespace hydla

#endif // _SIMULATOR_VALUE_H_

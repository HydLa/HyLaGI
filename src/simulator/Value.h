#pragma once

#include <string>
#include <vector>
#include <boost/shared_ptr.hpp>

#include "Node.h"


namespace hydla {
namespace simulator {

class Parameter;

class Value
{  
  public:

  typedef hydla::symbolic_expression::node_sptr node_sptr;
  
  virtual ~Value(){}
  
  Value();

  /**
   * 単なる文字列は数値と見なして受け取る
   */
  Value(const std::string &str);

  /**
   * construct Value from given integer
   */
  Value(int num);
  
  /**
   * 渡された数式を値とするValueを作る
   */
  Value(const symbolic_expression::node_sptr & node);
  
  /**
   * construct Value from given parameter
   */
  Value(const Parameter &param);

  /**
   * construct Value from given double value
   */
  Value(double num);
  
  virtual bool undefined() const;
  
  virtual bool infinite() const;

  /**
   * 文字列表現を取得する
   */
  virtual std::string get_string() const;

  virtual Value get_numerized_value()const;

  /**
   * Nodeの形式にしたものを取得する
   */
  virtual symbolic_expression::node_sptr get_node() const;

  virtual void set_node(const symbolic_expression::node_sptr&);
  
  /// Value同士の加算
  Value& operator+=(const Value &rhs);
  Value operator+(const Value &rhs);

  /// Value同士の減算
  Value& operator-=(const Value &rhs);
  Value operator-(const Value &rhs);

  /// Value同士の乗算
  Value& operator*=(const Value &rhs);
  Value operator*(const Value &rhs);

  /// Value同士の除算
  Value& operator/=(const Value &rhs);
  Value operator/(const Value &rhs);

  /// negative
  Value operator-();

  
  Value& operator^=(const Value &rhs);
  Value operator^(const Value &rhs);
  
  /**
   * データをダンプする
   */
  std::ostream& dump(std::ostream& s) const{
    if(undefined()) s << "UNDEF";
    else s << get_string();
    return s;
  }

  private:

  symbolic_expression::node_sptr node_;  /// symbolic expression
};

bool operator<(const Value& lhs, const Value& rhs);

std::ostream& operator<<(std::ostream& s, const Value & v);


} // namespace simulator
} // namespace hydla


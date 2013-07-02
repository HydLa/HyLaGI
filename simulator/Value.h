#ifndef _SIMULATOR_VALUE_H_
#define _SIMULATOR_VALUE_H_

#include <string>
#include <vector>
#include <boost/shared_ptr.hpp>

#include "ValueVisitor.h"
#include "Node.h"

#include <boost/operators.hpp>

namespace hydla {
namespace simulator {

class ValueVisitor;

class Value:
  public boost::additive<Value>
{  
  public:
  
  virtual ~Value(){}
  
  /**
   * 未定義値かどうか
   */
  virtual bool undefined() const = 0;
  
  /**
   * 自分自身のクローンを新たに作成し，そのポインタを返す
   * 返り値のメモリは呼び出した側で解放するように注意する．
   */
  virtual Value* clone() const = 0;

  /**
   * 文字列表現を取得する
   */
  virtual std::string get_string() const = 0;

  /**
   * Nodeの形式にしたものを取得する
   */
  virtual hydla::parse_tree::node_sptr get_node() const = 0;
  
  virtual void accept(ValueVisitor &) = 0;
  
  /**
   * Value同士の加算
   */
  Value& operator+=(const Value& rhs);

  /**
   * Value同士の減算
   */
  Value& operator-=(const Value& rhs);
  
  /**
   * データをダンプする
   */
  std::ostream& dump(std::ostream& s) const{
    if(undefined()) s << "UNDEF";
    else s << get_string();
    return s;
  }
};

bool operator<(const Value& lhs, const Value& rhs);

std::ostream& operator<<(std::ostream& s, const Value & v);


} // namespace simulator
} // namespace hydla

#endif // _SIMULATOR_VALUE_H_

#ifndef _SYMBOLIC_VALUE_H_
#define _SYMBOLIC_VALUE_H_

#include <string>
#include <vector>
#include <boost/shared_ptr.hpp>
#include "Node.h"

#include <boost/operators.hpp>

namespace hydla {
namespace symbolic_simulator {

class SymbolicValue:
    public boost::additive<SymbolicValue>
{

  typedef hydla::parse_tree::node_sptr node_sptr;
  node_sptr node_;  //値はnode_sptr
  
  bool is_unique_;  //値が一意かどうかを示す変数．とりあえず外部から設定する

  public:


  SymbolicValue(){}
  //単なる文字列は数値と見なして受け取る
  SymbolicValue(const std::string &str){node_.reset(new hydla::parse_tree::Number(str));}
  SymbolicValue(const node_sptr & node){node_ = node;}
  
  /**
   * 未定義値かどうか
   */
  bool is_undefined() const;
  
  /**
   * 値が（定量的に）一意に定まるか
   */
  bool is_unique() const;
  
  void set_unique(const bool &u){is_unique_=u;}

  /**
   * 文字列表現を取得する
   */
  std::string get_string() const;
  /**
   * ノードを取得する
   */
  node_sptr get_node() const;
  
  /**
   * 新たなノードをセット
   */
  void set(const node_sptr&);

  /**
   * SymbolicValue同士の加算. additiveを継承しているのでこれで+も使えるようになる
   */
  SymbolicValue& operator+=(const SymbolicValue& rhs);
  

  /**
   * SymbolicValue同士の減算. additiveを継承しているのでこれで-も使えるようになる
   */
  SymbolicValue& operator-=(const SymbolicValue& rhs);
  
  /**
   * データをダンプする
   */
  std::ostream& dump(std::ostream& s) const;
};

bool operator<(const SymbolicValue& lhs, const SymbolicValue& rhs);

std::ostream& operator<<(std::ostream& s, const SymbolicValue & v);


} // namespace simulator
} // namespace hydla

#endif // _SYMBOLIC_VALUE_H_

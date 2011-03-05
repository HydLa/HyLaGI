#ifndef _SYMBOLIC_VALUE_H_
#define _SYMBOLIC_VALUE_H_

#include <string>
#include <vector>
#include <boost/shared_ptr.hpp>
#include "Node.h"

namespace hydla {
namespace symbolic_simulator {

struct SymbolicValue {

  typedef hydla::parse_tree::node_sptr node_sptr;


  SymbolicValue(){}
  SymbolicValue(const std::string &str){str_=str;}
  
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
   * 新たな文字列をセット
   */
  void set(const std::string&);
  /**
   * 新たなノードをセット
   */
  void set(const node_sptr&);

  
  /**
   * データをダンプする
   */
  std::ostream& dump(std::ostream& s) const;

  private:
  std::string str_;  //とりあえず値は文字列で
  node_sptr node_;  //値をnode_sptrにしてみたい
  
  bool is_unique_;  //値が一意かどうかを示す変数．とりあえず外部から設定する
};

bool operator<(const SymbolicValue& lhs, const SymbolicValue& rhs);

std::ostream& operator<<(std::ostream& s, const SymbolicValue & v);


} // namespace simulator
} // namespace hydla

#endif // _SYMBOLIC_VALUE_H_

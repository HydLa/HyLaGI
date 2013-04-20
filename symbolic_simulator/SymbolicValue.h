#ifndef _SYMBOLIC_VALUE_H_
#define _SYMBOLIC_VALUE_H_

#include <string>
#include <vector>
#include <boost/shared_ptr.hpp>
#include "Node.h"
#include "../simulator/Value.h"

#include <boost/operators.hpp>

namespace hydla {
namespace simulator {
namespace symbolic {

class SymbolicValue: public hydla::simulator::Value
{  
  public:

  typedef hydla::parse_tree::node_sptr node_sptr;

  SymbolicValue();
  
  virtual ~SymbolicValue(){}
  
  virtual hydla::simulator::Value* clone() const
    {return new SymbolicValue(node_->clone());}
  
  /**
   * 単なる文字列は数値と見なして受け取る
   */
  SymbolicValue(const std::string &str);
  
  /**
   * 渡されたノードを参照するSymbolicValueを作る
   */
  SymbolicValue(const node_sptr & node);
  
  /**
   * 未定義値かどうか
   */
  bool is_undefined() const;

  /**
   * 文字列表現を取得する
   */
  std::string get_string() const;
  /**
   * ノードを取得する
   */
  node_sptr get_node() const;
  
  /**
   * ノードを設定する
   */
  void set_node(node_sptr);
  
  /**
   * 新たなノードをセット
   */
  void set(const node_sptr&);

  void accept(hydla::simulator::ValueVisitor& v){v.visit(*this);}
  
  private:
  
  node_sptr node_;  //値はnode_sptr
};

}
} // namespace simulator
} // namespace hydla

#endif // _SYMBOLIC_VALUE_H_
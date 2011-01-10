#ifndef _SYMBOLIC_VALUE_H_
#define _SYMBOLIC_VALUE_H_

#include <string>
#include <vector>

namespace hydla {
namespace symbolic_simulator {

struct SymbolicValue {

  typedef enum{
    EQUAL,
    NOT_EQUAL,
    LESS_EQUAL,
    LESS,
    GREATER_EQUAL,
    GREATER,
    RELATION_NUMBER
  }Relation;
  
  typedef struct{
    Relation relation;
    std::string value;
  }Element;
  
  static const std::string relation_symbol_[RELATION_NUMBER];
  /**
   * 未定義値かどうか
   */
  bool is_undefined() const;

  /**
   * 文字列表現を取得する
   */
  std::string get_string() const;
  
  /**
   * とりあえず文字列をセット
   */
  void set(std::string);

  /**
   * データをダンプする
   */
  std::ostream& dump(std::ostream& s) const;
  
  void add(const std::vector<Element> &vec);
  
  void set(const std::vector<std::vector<Element> > &vec);
  

  private:
  

  std::string visit_all(const std::vector<Element> &vec, const std::string &delimiter) const;
  //文字列配列を走査してデリミタで連結して出力
  
  std::vector<std::vector< Element > > value_; //値との関係の列の列（文字列のDNF形式になっている．∧でつながれたものを∨でつなぐ）
  std::string str_; // 文字列（任意の式を扱える）
};

bool operator<(const SymbolicValue& lhs, const SymbolicValue& rhs);

std::ostream& operator<<(std::ostream& s, const SymbolicValue & v);


} // namespace simulator
} // namespace hydla

#endif // _SYMBOLIC_VALUE_H_

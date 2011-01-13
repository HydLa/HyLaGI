#ifndef _SYMBOLIC_VALUE_H_
#define _SYMBOLIC_VALUE_H_

#include <string>
#include <vector>

namespace hydla {
namespace symbolic_simulator {

typedef std::string symbolic_element_value_t;   //simbolic_value_tの構成要素．Relationと組むもの

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
    symbolic_element_value_t value;
  }Element;
  
  SymbolicValue();
  
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
   * とりあえず最初のelement_valueを返す
   */
  symbolic_element_value_t get_first_value() const;
  
  /**
   * とりあえず最初のrelationを返す
   */
  Relation get_first_relation() const;

  /**
   * とりあえず最初のrelationの記号を返す
   */
  std::string get_first_symbol() const;
  
  /**
   * とりあえず文字列をセット
   */
  void set(std::string);

  /**
   * 新たなものをセット
   */
  void set(const std::vector<std::vector<Element> > &vec);
  
  /**
   * addの対象を次のorに．
   */
  void go_next_or();

  /**
   * 新たな要素を追加
   */
  void add(const Element &ele);

  /**
   * データをダンプする
   */
  std::ostream& dump(std::ostream& s) const;
  
  
  std::vector<std::vector< Element > > value_; //値との関係の列の列（DNF形式．∧でつながれたものを∨でつなぐ）

  private:
  

  std::vector <Element> *current_or_;
  std::string visit_all(const std::vector<Element> &vec, const std::string &delimiter) const;
  //配列を走査してデリミタで連結して出力
  
  std::string str_; // 文字列（任意の式を扱える）
};

bool operator<(const SymbolicValue& lhs, const SymbolicValue& rhs);

std::ostream& operator<<(std::ostream& s, const SymbolicValue & v);


} // namespace simulator
} // namespace hydla

#endif // _SYMBOLIC_VALUE_H_

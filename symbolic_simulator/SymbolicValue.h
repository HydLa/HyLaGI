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
  
  typedef struct Element{
    Relation relation;
    symbolic_element_value_t value;
    Element(const symbolic_element_value_t& val, const Relation& rel);
    std::string get_symbol()const;
    std::string get_value()const;
  }element_t;
  

  typedef std::vector< Element > and_vector;
  typedef std::vector< and_vector > or_vector;
  typedef and_vector::iterator         and_iterator;
  typedef and_vector::const_iterator   and_const_iterator;
  typedef or_vector::iterator         or_iterator;
  typedef or_vector::const_iterator   or_const_iterator;

  SymbolicValue();
  
  static const std::string relation_symbol_[RELATION_NUMBER];

  /**
   * 未定義値かどうか
   */
  bool is_undefined() const;
  
  /**
   * 値が（定量的に）一意に定まるか
   */
  bool is_unique() const;

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
   * 新たなものをセット
   */
  void set(const std::vector<std::vector<Element> > &vec);
  
  /**
   * 新たな要素を１つセットする（それまでのものは捨てる）
   */
  void set(const Element &ele);

  /**
   * 要素を全て捨て，初期化する
   */
  void clear();
  
  /**
   * addの対象を次のorに．
   */
  void go_next_or();

  /**
   * 新たな要素を追加
   */
  void add(const Element &ele);
  
  or_const_iterator or_begin() const {return value_.begin();}
  or_iterator or_begin() {return value_.begin();}
  or_const_iterator or_end() const {return value_.end();}
  or_iterator or_end() {return value_.end();}

  
  /**
   * データをダンプする
   */
  std::ostream& dump(std::ostream& s) const;
  
  
  or_vector value_; //値との関係の列の列（DNF形式．∧でつながれたものを∨でつなぐ）

  private:
  

  std::vector <Element> *current_or_;
  std::string visit_all(const std::vector<Element> &vec, const std::string &delimiter) const;
  //配列を走査してデリミタで連結して出力
  
};

bool operator<(const SymbolicValue& lhs, const SymbolicValue& rhs);

std::ostream& operator<<(std::ostream& s, const SymbolicValue & v);


} // namespace simulator
} // namespace hydla

#endif // _SYMBOLIC_VALUE_H_

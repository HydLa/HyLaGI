#ifndef _INCLUDED_HYDLA_SIMULATOR_VARIABLE_MAP_H_
#define _INCLUDED_HYDLA_SIMULATOR_VARIABLE_MAP_H_

#include <map>
#include <ostream>
#include <string>

namespace hydla {
namespace simulator {

/**
 * 変数表
 */
template<typename VariableType, typename ValueType>
class VariableMap {
public:
  typedef VariableType variable_t;
  typedef ValueType    value_t;
  
  typedef VariableMap<VariableType, ValueType> variable_map_t;
  typedef typename std::map<VariableType, ValueType> variable_list_t;

  // STL互換のためのtypedef (未完)
  typedef typename variable_list_t::value_type       value_type;
  typedef typename variable_list_t::iterator         iterator;
  typedef typename variable_list_t::const_iterator   const_iterator;

  /**
   * 変数と値の組を設定する
   * すでに設定済みの変数であった場合，値は上書きされる
   */
  void set_variable(const variable_t& var, const value_t& val)
  {
    iterator it = variables_.find(var);
    if(it != variables_.end()) {
      it->second = val;
    }
    else {
      variables_.insert(std::make_pair(var, val));
    }
  }

  /**
   * 要求された変数に対応する値のリファレンスを返す
   */
  value_t& get_variable(const variable_t& var)
  {
//     iterator it = variables_.find(var);
//     if(it != variables_.end()) {
//       return it->second;
//     }

    return variables_[var];
  }
  
  void clear()
  {
    variables_.clear();
  }

  iterator begin()       
  {
    return variables_.begin();
  }

  const_iterator begin() const 
  {
    return variables_.begin();
  }

  iterator end()       
  {
    return variables_.end();
  }

  const_iterator end() const 
  {
    return variables_.end();
  }

  /**
   * 変数表のサイズを返す
   */
  size_t size() const
  {
    return variables_.size();
  }

  /**
   * データをダンプする
   */
  std::ostream& dump(std::ostream& s) const
  {
    const_iterator it  = variables_.begin();
    const_iterator end = variables_.end();
    for(; it!=end; ++it) {
      s << it->first << " <=> " << it->second << "\n";
    }
    return s;
  }

private:  
  variable_list_t variables_;
};


template<typename VariableType, typename ValueType>
std::ostream& operator<<(std::ostream& s, 
                         const VariableMap<VariableType, ValueType> & v)
{
  return v.dump(s);
}

} // namespace simulator
} // namespace hydla 

#endif // _INCLUDED_HYDLA_SIMULATOR_VARIABLE_MAP_H_


#ifndef _INCLUDED_HYDLA_SIMULATOR_VARIABLE_MAP_H_
#define _INCLUDED_HYDLA_SIMULATOR_VARIABLE_MAP_H_

namespace hydla {
namespace simulator {

/**
 * 変数表
 */
template<typename VariableType, typename ValueType>
struct VariableMap {
public:
  typedef typename std::map<VariableType, ValueType> variable_list_t;
  typedef typename variable_list_t::const_iterator variable_list_const_iterator_t;

  variable_list_t variables;

  /**
   * データをダンプする
   */
  std::ostream& dump(std::ostream& s) const
  {
    variable_list_const_iterator_t it  = variables.begin();
    variable_list_const_iterator_t end = variables.end();
    while(it!=end) {
      s << it->first.dump(s) << " : " << it->second.dump(s) << "\n";
    }
    return s;
  }
};

} // namespace simulator
} // namespace hydla 

#endif // _INCLUDED_HYDLA_SIMULATOR_VARIABLE_MAP_H_


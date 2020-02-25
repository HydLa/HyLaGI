#pragma once

#include <ostream>
#include <string>

#include <cassert>

#include <memory>

#include "Node.h"

namespace hydla { 
namespace parser {

/**
 * 定義制約を収納するクラス
 */
template<typename DefinitionNodeType>
class DefinitionContainer 
{
public:
  typedef DefinitionNodeType                   definition_node_t;

  typedef std::string                          definition_name_t;
  typedef int                                  bound_variable_count_t;
  typedef std::pair<definition_name_t, 
                    bound_variable_count_t>    definition_map_key_t;

  typedef std::shared_ptr<definition_node_t> definition_map_value_t;
  typedef std::map<definition_map_key_t, 
                   definition_map_value_t>     definition_map_t;
  typedef typename definition_map_t::const_iterator definition_map_const_iterator;

  /**
   * 定義ノードを追加する際のキーを作成する
   */
  definition_map_key_t 
  create_definition_key(definition_name_t name, 
                        bound_variable_count_t count) const 
  {
    return std::make_pair(name, count);
  }

  definition_map_key_t 
  create_definition_key(
    const std::shared_ptr<hydla::symbolic_expression::Definition>& d) const 
  {
    return std::make_pair(d->get_name(), 
                          d->bound_variable_size());
  }
  
  /**
   * 定義ノードを追加する
   */
  void add_definition(const std::shared_ptr<definition_node_t>& d)
  {
    definition_map_key_t key(create_definition_key(d));
    
    assert(!is_registered(key));
    definition_map_.insert(std::make_pair(key, d));
  }

  /**
   * すでに登録済みのノードであるかどうか
   */
  bool is_registered(
    const std::shared_ptr<hydla::symbolic_expression::Definition>& d) const 
  {
    return is_registered(create_definition_key(d));
  }

  bool is_registered(const definition_map_key_t& d) const 
  {
    return definition_map_.find(d) != definition_map_.end();
  }


  /**
   * 定義ノードを返す
   *
   * @return 与えられた定義に対するノード．
   *          存在しない定義の場合は空クラスを返す
   */
  const std::shared_ptr<definition_node_t> 
  get_definition(const definition_map_key_t& d) const
  {
    definition_map_const_iterator it = definition_map_.find(d);
    if(it == definition_map_.end()) {
      return std::shared_ptr<definition_node_t>();
    }
    return it->second;
  }

  /**
   * 登録されている情報をすべて消去し，初期状態に戻す
   */
  void clear()
  {
    definition_map_.clear();
  }

  /**
   * 定義内容の出力をおこなう
   */
  std::ostream& dump(std::ostream& s) const
  {
    definition_map_const_iterator it  = definition_map_.begin();
    definition_map_const_iterator end = definition_map_.end();
    for(; it!=end; ++it) {
      s << "key: " << it->first.first << "<" << it->first.second << "> " 
        << "val: " << *it->second << "\n";
    }
    return s;
  }

  definition_map_const_iterator begin() const{return definition_map_.begin();}
  definition_map_const_iterator end() const{return definition_map_.end();}

private:

  /**
   * 定義を格納するためのmap
   */
  definition_map_t definition_map_;
};

template<typename T>
std::ostream& operator<<(std::ostream& s, const DefinitionContainer<T>& d)
{
  return d.dump(s);
}

} //namespace parser
} //namespace hydla


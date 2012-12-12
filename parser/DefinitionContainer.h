#ifndef _INCLUDED_HYDLA_PARSER_DEFINITION_CONTAINER_H_
#define _INCLUDED_HYDLA_PARSER_DEFINITION_CONTAINER_H_

#include <ostream>
#include <string>
#include <vector>
#include <map>

#include <cassert>

#include <boost/shared_ptr.hpp>

#include "Node.h"

namespace hydla { 
namespace parser {

/**
 * ��`��������[����N���X
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

  typedef boost::shared_ptr<definition_node_t> definition_map_value_t;
  typedef std::map<definition_map_key_t, 
                   definition_map_value_t>     definition_map_t;
  typedef typename definition_map_t::const_iterator definition_map_const_iterator;

  /**
   * ��`�m�[�h��ǉ�����ۂ̃L�[���쐬����
   */
  definition_map_key_t 
  create_definition_key(definition_name_t name, 
                        bound_variable_count_t count) const 
  {
    return std::make_pair(name, count);
  }

  definition_map_key_t 
  create_definition_key(
    const boost::shared_ptr<hydla::parse_tree::Definition>& d) const 
  {
    return std::make_pair(d->get_name(), 
                          d->bound_variable_size());
  }
  
  /**
   * ��`�m�[�h��ǉ�����
   */
  void add_definition(const boost::shared_ptr<definition_node_t>& d)
  {
    definition_map_key_t key(create_definition_key(d));
    
    assert(!is_registered(key));
    definition_map_.insert(std::make_pair(key, d));
  }

  /**
   * ���łɓo�^�ς݂̃m�[�h�ł��邩�ǂ���
   */
  bool is_registered(
    const boost::shared_ptr<hydla::parse_tree::Definition>& d) const 
  {
    return is_registered(create_definition_key(d));
  }

  bool is_registered(const definition_map_key_t& d) const 
  {
    return definition_map_.find(d) != definition_map_.end();
  }


  /**
   * ��`�m�[�h��Ԃ�
   *
   * @return �^����ꂽ��`�ɑ΂���m�[�h�D
   *          ���݂��Ȃ���`�̏ꍇ�͋�N���X��Ԃ�
   */
  const boost::shared_ptr<definition_node_t> 
  get_definition(const definition_map_key_t& d) const
  {
    definition_map_const_iterator it = definition_map_.find(d);
    if(it == definition_map_.end()) {
      return boost::shared_ptr<definition_node_t>();
    }
    return it->second;
  }

  /**
   * �o�^����Ă���������ׂď������C������Ԃɖ߂�
   */
  void clear()
  {
    definition_map_.clear();
  }

  /**
   * ��`���e�̏o�͂������Ȃ�
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

private:

  /**
   * ��`���i�[���邽�߂�map
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

#endif //_INCLUDED_HYDLA_PARSER_DEFINITION_CONTAINER_H_

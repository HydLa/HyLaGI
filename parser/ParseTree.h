#ifndef _INCLUDED_HYDLA_PARSE_TREE_H_
#define _INCLUDED_HYDLA_PARSE_TREE_H_

#include <ostream>
#include <string>
#include <sstream>
#include <vector>
#include <map>
#include <cassert>
#include <set>

#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
//#include <boost/bimap/bimap.hpp>
//#include <boost/bimap/unordered_set_of.hpp>
//#include <boost/bimap/support/lambda.hpp>

#include "ParseError.h"
#include "Node.h"
#include "NodeFactory.h"

namespace hydla { 
namespace parse_tree {

class ParseTree {
public: 
  typedef hydla::parser::NodeFactory        node_factory_t;
  typedef boost::shared_ptr<node_factory_t> node_factory_sptr;

  // �ϐ��\
  typedef std::map<std::string, int>     variable_map_t;
  typedef variable_map_t::const_iterator variable_map_const_iterator;

    
  // �m�[�h�\
  typedef std::map<node_id_t, node_sptr> node_map_t;
  typedef node_map_t::value_type         node_map_value_t;
  typedef node_map_t::const_iterator     node_map_const_iterator;

  // �m�[�hID�\
  typedef std::set<node_id_t>           node_id_list_t;
  typedef node_id_list_t::const_iterator node_id_list_const_iterator;

  /*
  struct tag_node_id {};
  struct tag_node_sptr {};

  typedef 
    boost::bimaps::bimap<
      boost::bimaps::unordered_set_of<
        boost::bimaps::tags::tagged<node_id_t, tag_node_id> >,
    boost::bimaps::unordered_set_of<
        boost::bimaps::tags::tagged<node_sptr, tag_node_sptr> > > node_map_t;

  typedef node_map_t::value_type                         node_map_value_t;
  */

  static const int INITIAL_MAX_NODE_ID = 0;

  ParseTree();
  ParseTree(const ParseTree& pt);

  virtual ~ParseTree();

  /**
   * ParseTree���\�z����
   */
  template<typename NodeFactoryT>
  void parse(std::istream& s)
  {
    parse(s, boost::make_shared<NodeFactoryT>());
  }

  template<typename NodeFactoryT>
  void parse_string(std::string str)
  {
    std::istringstream stream(str);
    parse<NodeFactoryT>(stream);
  }
    
  void parse(std::istream& s, node_factory_sptr node_factory);


  /**
   * �m�[�hID�̕\�̍č\�z�������Ȃ�
   * ���܂ł̃m�[�hID�͖����ƂȂ�
   */
  void rebuild_node_id_list();

  /**
   * ID�̊��蓖�Ă��Ă��Ȃ��m�[�h�ɑ΂���ID�����蓖�Ă�
   * �m�[�h���폜���ꂽ�ꍇ�́C�m�[�h�\����폜����
   */
  void update_node_id_list();

  /**
   * �ϐ���o�^����
   * ���łɓo�^�ς݂̓���ϐ��̔����񐔂���
   * �傫��������ϐ��̃��X�g�ɓo�^�����
   *
   * @param name �ϐ���
   * @param differential_count ������
   *
   * @return �o�^���ꂽ���ǂ���
   */
  bool register_variable(const std::string& name, int differential_count);

  /**
   * �w�肵���ϐ��̍ő�����񐔂����߂�
   */
  int get_differential_count(const std::string& name) const;

  /**
   * �ϐ��\�̐擪�̗v�f��Ԃ�
   */
  variable_map_const_iterator variable_map_begin() const
  {
    return variable_map_.begin();
  }

  /**
   * �ϐ��\�̍Ō�̎��̗v�f��Ԃ�
   */
  variable_map_const_iterator variable_map_end() const
  {
    return variable_map_.end();
  }
  
  
  variable_map_t get_variable_map() const
  {
    return variable_map_;
  }


  /**
   * �p�[�X���ꂽ�m�[�h�c���[�̐ݒ�
   * �ݒ肳�ꂽ��C�Ӗ���͓��̑O�����͎����ł����Ȃ���
   */
  //void set_tree(const node_sptr& tree);
 
  /**
   * �m�[�h�c���[����������
   * �V�����m�[�h�c���[�͈Ӗ���͓��̑O�����������Ȃ�����ł���K�v������
   */
  node_sptr swap_tree(const node_sptr& tree);

  bool is_same_struct(const ParseTree& pt, bool exactly_same) const {
    return node_tree_->is_same_struct(*pt.node_tree_, exactly_same);
  }

  /**
   * dot����`���ł̕\��
   */
  std::string to_graphviz() const
  {
    std::stringstream str;
    to_graphviz(str);
    return str.str();
  }

  std::ostream& to_graphviz(std::ostream& s) const;

  /**
   * �m�[�h�c���[�ɑ΂��ăr�W�^�[��K�p����
   */
  void dispatch(parse_tree::TreeVisitor* visitor)
  {
    if(node_tree_) node_tree_->accept(node_tree_, visitor);
  }

  /**
   * �m�[�h�c���[�ɑ΂��ăr�W�^�[��K�p����
   */
  void dispatch(parse_tree::BaseNodeVisitor* visitor)
  {
    if(node_tree_) node_tree_->accept(node_tree_, visitor);
  }

  /**
   * �V�����m�[�h��ǉ�����
   */
  node_id_t register_node(const node_sptr& n);

  /**
   * ID�ɑΉ��t�����Ă���m�[�h�̕ύX
   */
  void update_node(node_id_t id, const node_sptr& n);

  /**
   * �m�[�h�\����w�肳�ꂽ�m�[�hID�̏����폜����
   */
  void remove_node(node_id_t id);
    
  /**
   * �m�[�h�ɑΉ��t�����Ă���ID�̕ύX
   */
  //void update_node_id(node_id_t id, const node_sptr& n);

  /**
   * �w�肳�ꂽID�ɑΉ�����m�[�h�𓾂�
   */  
  node_sptr get_node(node_id_t id) const
  {
    node_map_t::const_iterator it = node_map_.find(id);
    if(it != node_map_.end()) {
      return it->second;
    }

    return node_sptr();
  }  
  
  /**
   * �g�b�v�m�[�h�𓾂�
   */  
  node_sptr get_node() const
  {
    return node_tree_;
  }  
  
  /**
   * assert�m�[�h�𓾂�
   */
  node_sptr get_assertion_node() const
  {
    return assertion_node_tree_;
  }
  
  /*
  node_sptr get_node(node_id_t id) const
  {
    node_map_t::map_by<tag_node_id>::const_iterator it = 
      node_map_.by<tag_node_id>().find(id);
    if(it != node_map_.by<tag_node_id>().end()) {
      return it->second;
    }
    return node_sptr();
  }
  */

  
  /**
   * �w�肳�ꂽ�m�[�h�ɑΉ�����ID�𓾂�
   */
  /*
  node_id_t get_node_id(const node_sptr& n) const
  {
    node_map_t::map_by<tag_node_sptr>::const_iterator it = 
      node_map_.by<tag_node_sptr>().find(n);
    if(it != node_map_.by<tag_node_sptr>().end()) {
      return it->second;
    }
    return node_id_t();
  }
  */

  /**
   * �m�[�h�\�̍ŏ��̗v�f
   */
  node_map_const_iterator node_map_begin() const 
  {
    return node_map_.begin();
  }

  /**
   * �m�[�h�\�̍Ō�̎��̗v�f
   */ 
  node_map_const_iterator node_map_end() const 
  {
    return node_map_.end();
  }

  /**
   * �m�[�h�\�̃T�C�Y
   */
  size_t node_map_size() const
  {
    return node_map_.size();
  }

  /**
   * �m�[�h�\���ɂ���m�[�hID�̃��X�g���쐬����
   */
  void make_node_id_list();

  /**
   * �m�[�hID���X�g�̍ŏ��̗v�f
   */
  node_id_list_const_iterator node_id_list_begin()
  {
    return node_id_list_.begin();
  }

  /**
   * �m�[�hID���X�g�̍Ō�̎��̗v�f
   */
  node_id_list_const_iterator node_id_list_end()
  {
    return node_id_list_.end();
  }

  /**
   * �o�^����Ă���NodeFactory�����Ɏw�肳�ꂽ�^�̃m�[�h�𐶐�����
   */
  template<typename NodeType>
  boost::shared_ptr<NodeType> create_node() const
  {
    return node_factory_->create<NodeType>();
  }
  

  /**
   * ���ׂẴf�[�^��j�����A������Ԃɖ߂�
   */
  void clear();

  /**
   * ParseTree�̏�Ԃ̏o��
   */
  std::ostream& dump(std::ostream& s) const;

private:  
  ParseTree& operator=(const ParseTree& pt);

  node_factory_sptr    node_factory_;

  node_sptr            node_tree_;
  node_sptr            assertion_node_tree_;
  
  variable_map_t       variable_map_;

  node_map_t           node_map_;
  node_id_t            max_node_id_;
  node_id_list_t       node_id_list_;
};

std::ostream& operator<<(std::ostream& s, const ParseTree& pt);


} //namespace parse_tree
} //namespace hydla

#endif //_INCLUDED_HYDLA_PARSE_TREE_H_

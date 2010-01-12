#ifndef _INCLUDED_HYDLA_PARSE_TREE_H_
#define _INCLUDED_HYDLA_PARSE_TREE_H_

#include <ostream>
#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <cassert>

#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <boost/bimap/bimap.hpp>
#include <boost/bimap/unordered_set_of.hpp>
#include <boost/bimap/support/lambda.hpp>

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
  typedef boost::bimaps::bimap<
            boost::bimaps::unordered_set_of<node_id_t>, 
            boost::bimaps::unordered_set_of<node_sptr> > node_map_t;
  typedef node_map_t::value_type                         node_map_value_t;

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
   * �m�[�h��ID�̍X�V�������Ȃ�
   */
  void uptate_node_id();

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
   * �m�[�h�ɑΉ��t�����Ă���ID�̕ύX
   */
  void update_node_id(node_id_t id, const node_sptr& n);

  /**
   * �w�肳�ꂽID�ɑΉ�����m�[�h�𓾂�
   */
  node_sptr get_node(node_id_t id)
  {
    node_map_t::left_iterator it = node_map_.left.find(id);
    if(it != node_map_.left.end()) {
      return it->second;
    }
    return node_sptr();
  }
  
  /**
   * �w�肳�ꂽ�m�[�h�ɑΉ�����ID�𓾂�
   */
  node_id_t get_node_id(const node_sptr& n)
  {
    node_map_t::right_iterator it = node_map_.right.find(n);
    if(it != node_map_.right.end()) {
      return it->second;
    }
    return node_id_t();
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
  node_factory_sptr    node_factory_;

  node_sptr            node_tree_;
  variable_map_t       variable_map_;

  node_map_t           node_map_;
  node_id_t            max_node_id_;
};

std::ostream& operator<<(std::ostream& s, const ParseTree& pt);


} //namespace parse_tree
} //namespace hydla

#endif //_INCLUDED_HYDLA_PARSE_TREE_H_

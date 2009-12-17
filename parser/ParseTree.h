#ifndef _INCLUDED_HYDLA_PARSE_TREE_H_
#define _INCLUDED_HYDLA_PARSE_TREE_H_

#include <ostream>
#include <string>
#include <vector>
#include <map>

#include <assert.h>

#include <boost/shared_ptr.hpp>
#include <boost/bimap/bimap.hpp>
#include <boost/bimap/unordered_set_of.hpp>
#include <boost/bimap/support/lambda.hpp>

#include "ParseError.h"
#include "Node.h"

namespace hydla { 
namespace parse_tree {

class ParseTree {
public:
  // ��`�̌^
  typedef std::string                             difinition_name_t;
  typedef int                                     bound_variable_count_t;
  typedef std::pair<difinition_name_t, 
                    bound_variable_count_t>       difinition_type_t;

  // �����`
  typedef boost::shared_ptr<hydla::parse_tree::ConstraintDefinition> 
    constraint_def_map_value_t;
  typedef std::map<difinition_type_t, constraint_def_map_value_t>    
    constraint_def_map_t;

  // �v���O������`
  typedef boost::shared_ptr<hydla::parse_tree::ProgramDefinition>    
    program_def_map_value_t;
  typedef std::map<difinition_type_t, program_def_map_value_t>
    program_def_map_t;
  
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
   * �c���[�̈Ӗ���͂������Ȃ�
   */
  void semantic_analyze();

  /**
   * �m�[�h��ID�̍X�V�������Ȃ�
   */
  void uptate_node_id();
  
  /**
   * �����`��ǉ�����
   */
  void addConstraintDefinition(const boost::shared_ptr<ConstraintDefinition>& d)
  {
    cons_def_map_.insert(make_pair(create_definition_key(d), d));
  }
  
  /**
   * �v���O������`��ǉ�����
   */
  void addProgramDefinition(boost::shared_ptr<ProgramDefinition> d)
  {
    prog_def_map_.insert(make_pair(create_definition_key(d), d));
  }

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
  bool register_variable(const std::string& name, int differential_count)
  {
    variable_map_t::iterator it = variable_map_.find(name);
    if(it == variable_map_.end() ||
      it->second < differential_count) 
    {
      variable_map_[name] = differential_count;
      return true;
    }
    return false;
  }

  /**
   * �w�肵���ϐ��̍ő�����񐔂����߂�
   */
  int get_differential_count(const std::string& name) const
  {
    int ret = -1;

    variable_map_t::const_iterator it = variable_map_.find(name);
    if(it != variable_map_.end()) {
      ret =  it->second;
    }

    return ret;
  }

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
   */
  void set_tree(const node_sptr& tree) 
  {
    node_tree_ = tree;
  }

  node_sptr get_tree()
  {
    return node_tree_;
  }

  void dispatch(parse_tree::TreeVisitor* visitor)
  {
    if(node_tree_) node_tree_->accept(node_tree_, visitor);
  }
  
  void dispatch(parse_tree::BaseNodeVisitor* visitor)
  {
    if(node_tree_) node_tree_->accept(node_tree_, visitor);
  }

  /**
   * �����`�m�[�h��Ԃ�
   *
   * @return �^����ꂽ��`�ɑ΂���m�[�h�D
   *          ���݂��Ȃ���`�̏ꍇ�͋�N���X��Ԃ�
   */
  const boost::shared_ptr<ConstraintDefinition> 
    get_constraint_difinition(const difinition_type_t& def) const
  {
    constraint_def_map_t::const_iterator it = cons_def_map_.find(def);
    if(it == cons_def_map_.end()) {
      return boost::shared_ptr<ConstraintDefinition>();
    }
    return it->second;
  }


  /**
   * �v���O������`�m�[�h��Ԃ�
   *
   * @return �^����ꂽ��`�ɑ΂���m�[�h�D
   *          ���݂��Ȃ���`�̏ꍇ�͋�N���X��Ԃ�
   */
  const boost::shared_ptr<ProgramDefinition> 
    get_program_difinition(const difinition_type_t& def) const
  {
    program_def_map_t::const_iterator it = prog_def_map_.find(def);
    if(it == prog_def_map_.end()) {
      return boost::shared_ptr<ProgramDefinition>();
    }
    return it->second;
  }

  /**
   * �V�����m�[�h��ǉ�����
   */
  node_id_t register_node(const node_sptr& n)
  {
    assert(n->get_id() == 0);
    assert(node_map_.right.find(n) == node_map_.right.end());
    
    ++max_node_id_;
    n->set_id(max_node_id_);
    node_map_.insert(node_map_value_t(max_node_id_, n));
    return max_node_id_;
  }

  /**
   * ID�ɑΉ��t�����Ă���m�[�h�̕ύX
   */
  void update_node(node_id_t id, const node_sptr& n)
  {
    node_map_t::left_iterator it = node_map_.left.find(id);
    assert(it != node_map_.left.end());

    node_map_.left.modify_data(it, boost::bimaps::_data = n);
  }
    
  /**
   * �m�[�h�ɑΉ��t�����Ă���ID�̕ύX
   */
  void update_node_id(node_id_t id, const node_sptr& n)
  {
    node_map_t::right_iterator it = node_map_.right.find(n);
    assert(it != node_map_.right.end());
      
    node_map_.right.modify_data(it, boost::bimaps::_data = id);
  }

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
   * ���ׂẴf�[�^��j�����A������Ԃɖ߂�
   */
  void clear()
  {
    max_node_id_ = 0;
    node_tree_.reset();
    cons_def_map_.clear();
    prog_def_map_.clear();
    variable_map_.clear();
  }

  std::ostream& dump(std::ostream& s) const;

private:
  difinition_type_t create_definition_key(boost::shared_ptr<Definition> d);

  node_sptr            node_tree_;
  constraint_def_map_t cons_def_map_;
  program_def_map_t    prog_def_map_;
  variable_map_t       variable_map_;

  node_map_t           node_map_;
  node_id_t            max_node_id_;
};

std::ostream& operator<<(std::ostream& s, const ParseTree& pt);


} //namespace parse_tree
} //namespace hydla

#endif //_INCLUDED_HYDLA_PARSE_TREE_H_
